/*
    CaPriCe for Palm OS - Amstrad CPC 464/664/6128 emulator for Palm devices
    Copyright (C) 2009  Fr�d�ric Coste

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifdef __PALMOS__
#include <PalmOS.h>
#include <VFSMgr.h>
#include <PceNativeCall.h>
#include <ByteOrderUtils.h>
#endif

#include <string.h>

#include "CPC.h"
#include "Files.h"
#include "types.h"
#include "Routines.h"
#include "Prefs.h"
#include "trace.h"
#include "vfsfile.h"
#include "minIni.h"

//===================
// PATCH begin
#ifdef _PATCH_ENABLE

#endif /* _PATCH_ENABLE */
// PATCH end
//===================

#define StrCopy(x,y)                strcpy(((char*)x), ((char*)y))
#define StrNCopy(x,y,z)             strncpy((x), (y), (z))
#define StrCompare(x,y)             strcmp(((char*)x), ((char*)y))
#define StrCat(x,y)                 strcat((x), (y))
#define	StrLen(x)                   strlen(((char *)x))
#define	StrChr(x,y)                 strchr((x), (y))
#define	StrStr(x,y)                strstr((x), (y))

#define MemPtrNew(p) malloc(p)
#define MemPtrFree(p) free(p)

// Form Alert needs a function to display error //??
#define FrmAlert(p)

#define MISC_SECTION_SIZE          ( CONTEXT_SIZE_Z80_SZ + CONTEXT_SIZE_Z80_SZ_BIT + CONTEXT_SIZE_Z80_SZP + \
                                     CONTEXT_SIZE_Z80_SZHV_inc + CONTEXT_SIZE_Z80_SZHV_dec + \
                                     CONTEXT_SIZE_MODE0_TABLE + CONTEXT_SIZE_MODE1_TABLE + \
                                     CONTEXT_SIZE_GATEARRAY + \
                                     CONTEXT_SIZE_PPI + \
                                     CONTEXT_SIZE_PSG_LEVEL_PP + \
                                     CONTEXT_SIZE_PSG_LEVEL_AR + CONTEXT_SIZE_PSG_LEVEL_AL + \
                                     CONTEXT_SIZE_PSG_LEVEL_BR + CONTEXT_SIZE_PSG_LEVEL_BL + \
                                     CONTEXT_SIZE_PSG_LEVEL_CR + CONTEXT_SIZE_PSG_LEVEL_CL + \
                                     CONTEXT_SIZE_GPBUFFER + \
                                     CONTEXT_SIZE_RAM + CONTEXT_SIZE_SND_BUFFER + \
                                     CONTEXT_SIZE_SESSIONFILENAME )

typedef struct
{
  UInt32 Offset;
  UInt32 Size;
  UInt32 StaticAreaSize;
  UInt32 DynamicAreaSize;
} tRestoreCPCSection;

static const tRestoreCPCSection RestoreCPCSections[] =
{
	// Z80 section
	{ CONTEXT_OFFSET_Z80, CONTEXT_SIZE_Z80, STATIC_SIZE_Z80, DYNAMIC_SIZE_Z80 },
	// PSG section
	{ CONTEXT_OFFSET_PSG, CONTEXT_SIZE_PSG, STATIC_SIZE_PSG, DYNAMIC_SIZE_PSG },
	// FDC section
	{ CONTEXT_OFFSET_FDC, CONTEXT_SIZE_FDC, STATIC_SIZE_FDC, DYNAMIC_SIZE_FDC },
	// VDU section
	{ CONTEXT_OFFSET_VDU, CONTEXT_SIZE_VDU, STATIC_SIZE_VDU, DYNAMIC_SIZE_VDU },
	// CRTC section
	{ CONTEXT_OFFSET_CRTC, CONTEXT_SIZE_CRTC, STATIC_SIZE_CRTC, DYNAMIC_SIZE_CRTC },
	// Misc section
	{ CONTEXT_OFFSET_Z80_SZ, MISC_SECTION_SIZE, 0, 0 },
	// Sound Callback Param section
	{ CONTEXT_OFFSET_SOUND_CB_PARAM, CONTEXT_SIZE_SOUND_CB_PARAM, STATIC_SIZE_SOUND_CB_PARAM, DYNAMIC_SIZE_SOUND_CB_PARAM },
};
#define NB_RESTORECPC_SECTIONS    NUMBER_OF_ITEMS(RestoreCPCSections)


//
// BMP FORMAT
//
typedef struct
{
	tUShort Signature;
	tULong FileSize;
	tULong Reserved;
	tULong Offset;
}tBitmapFileHeader;

typedef struct
{
	tULong Size;
	tULong ImageWidth;
	tULong ImageHeight;
	tUShort NbPlanes;
	tUShort BitsPerPixel;
	tULong CompressionType;
	tULong ImageSize;
	tULong HorizontalResolution;
	tULong VerticalResolution;
	tULong NbColors;
	tULong NbImportantColors;
}tBitmapHeader;


#define SCREENSHOT_FILE_HEADER_SIZE     sizeof(tBitmapFileHeader)
#define SCREENSHOT_BITMAP_HEADER_SIZE   sizeof(tBitmapHeader)
#define SCREENSHOT_NB_COLORS            32
#define SCREENSHOT_COLOR_SIZE           4
#define SCREENSHOT_IMAGE_WIDTH          384L
#define SCREENSHOT_IMAGE_HEIGHT         272L
#define SCREENSHOT_IMAGE_HEIGHT_OFFSET  0  // To keep top and bottom borders with same height
#define SCREENSHOT_HEADER               ( SCREENSHOT_FILE_HEADER_SIZE + SCREENSHOT_BITMAP_HEADER_SIZE + \
                                          (SCREENSHOT_NB_COLORS * SCREENSHOT_COLOR_SIZE) )
#define SCREENSHOT_SIZE_IMAGE           (SCREENSHOT_IMAGE_WIDTH * SCREENSHOT_IMAGE_HEIGHT)
#define SCREENSHOT_SIZE                 (SCREENSHOT_HEADER + SCREENSHOT_SIZE_IMAGE)


static const tBitmapFileHeader BitmapFileHeader =
{
  0x4d42,
  EndianSwap32(SCREENSHOT_SIZE),
  0,
  EndianSwap32(SCREENSHOT_HEADER)
};

static const tBitmapHeader BitmapHeader =
{
  EndianSwap32(SCREENSHOT_BITMAP_HEADER_SIZE),
  EndianSwap32(SCREENSHOT_IMAGE_WIDTH),
  EndianSwap32(SCREENSHOT_IMAGE_HEIGHT),
  EndianSwap16(1),                               // Number of planes
  EndianSwap16(8),                               // Bits per pixel
  0,                                             // Type of compression
  EndianSwap32(SCREENSHOT_SIZE_IMAGE),
  0,                                             // Horizontal resolution
  0,                                             // Vertical resolution
  EndianSwap32(SCREENSHOT_NB_COLORS),            // Number of colors
  0                                              // Number of important colors
};


static Err OpenDirectory(const char* pathP,
                         FileRef* dirRefP,
                         UInt16* volRefNumP) SECTION_FILES;

static Err OpenFile(const char* pathP,
                    const char* filenameP,
                    FileRef* fileRefP,
                    UInt16 openMode,
                    Boolean create) SECTION_FILES;

#ifndef _USE_MACROS
static Err CloseFile(FileRef fileRef) SECTION_FILES;
#else /* _USE_MACROS */
#define CloseFile(fileRef) \
  VFSFileClose((FileRef)fileRef);
#endif /* _USE_MACROS */



const tDiskFormat disk_format[MAX_DISK_FORMAT] =
{
  { "178K Data Format", 40, 1, 9, 2, 0x52, 0xe5, {{ 0xc1, 0xc6, 0xc2, 0xc7, 0xc3, 0xc8, 0xc4, 0xc9, 0xc5 }} },
  { "169K Vendor Format", 40, 1, 9, 2, 0x52, 0xe5, {{ 0x41, 0x46, 0x42, 0x47, 0x43, 0x48, 0x44, 0x49, 0x45 }} }
};

static tCPCRestoreReturnCode RestoreCPC_Section(FileRef ContextFileRef,
                                                tUChar* ContextP,
                                                tUChar* FileContextP,
                                                const tRestoreCPCSection* sectionP) SECTION_FILES;
static tCPCRestoreReturnCode RestoreCPC_Drive(FileRef ContextFileRef,
                                              tDrive* NativeDriveP,
                                              tNativeCPC* NativeCPC,
                                              tBool* LoadDiskImageP);
static tCPCRestoreReturnCode RestoreCPC_DiskImage(FileRef ContextFileRef,
                                                  tDrive* NativeDriveP,
                                                  tNativeCPC* NativeCPC) SECTION_FILES;
static tCPCRestoreReturnCode RestoreCPC_MemBank(FileRef ContextFileRef,
                                                tUChar* ContextP,
                                                tUChar* FileContextP,
                                                tUChar* readNativeCPCP) SECTION_FILES;

/*----------------------------------------------------------------------------*/
void ReindexPointersArea(tReindexOperationParameters* paramP)
/***********************************************************************
 *
 *  ReindexPointersArea
 *
 ***********************************************************************/
{
#ifndef __RELEASE__
  if (paramP == NULL)
    printf("paramP == NULL\n");
#endif /* __RELEASE__ */

  while (paramP->areaSize)
  {
    // Get pointer address
#ifdef SIM
    uint64_t ptr = (UInt32)*paramP->srcAreaP;
#else
    UInt32 ptr = (UInt32)*paramP->srcAreaP;
#endif

    // If pointer is inside source context range
    if ( (ptr >= paramP->srcContextBase) &&
         (ptr < (paramP->srcContextBase+paramP->contextSize)) )
    {
      // Adapt pointer to destination context
      ptr -= paramP->srcContextBase;
      ptr += paramP->dstContextBase;

      // Update destination area
      *paramP->dstAreaP = (void*)ptr;
    }

    // Next pointer
    paramP->srcAreaP++;
    paramP->dstAreaP++;
    paramP->areaSize -= sizeof(tUChar*);
  }
}
/*----------------------------------------------------------------------------*/

char*	StrToLower(Char *dst, const Char *src)
/***********************************************************************
 *
 *  StrToLower
 *
 ***********************************************************************/
{
  for(int i = 0; src[i]; i++)
    dst[i] =  ((src[i] >= 'A') && (src[i] <= 'Z')) ? src[i] | 0x60 : src[i];
  return dst;
}
/*----------------------------------------------------------------------------*/
static tVoid MemMove(tUChar* destP,
                     tUChar* sourceP,
                     tULong numBytes)
/***********************************************************************
 *
 *  MemMove
 *
 ***********************************************************************/
{
  while (numBytes--)
  {
    *(destP++) = *(sourceP++);
  }
}
/*----------------------------------------------------------------------------*/

#ifndef _USE_MACROS

tBool IsDriveFilenameExist(tDrive* nativeDriveP)
/***********************************************************************
 *
 *  IsDriveFilenameExist
 *
 ***********************************************************************/
{
  return (((tDrive*)EndianSwap32(nativeDriveP))->filename[0] == 0 ? cFalse : cTrue);
}
/*----------------------------------------------------------------------------*/

void SetDriveFilename(tDrive* nativeDriveP,
                      const char* filenameP)
/***********************************************************************
 *
 *  SetDriveFilename
 *
 ***********************************************************************/
{
  StrCopy(((tDrive*)EndianSwap32(nativeDriveP))->filename,
          filenameP);
}
/*----------------------------------------------------------------------------*/

void GetDriveFilename(tDrive* nativeDriveP,
                      char* filenameP)
/***********************************************************************
 *
 *  GetDriveFilename
 *
 ***********************************************************************/
{
  StrCopy(filenameP,
          ((tDrive*)EndianSwap32(nativeDriveP))->filename);
}
/*----------------------------------------------------------------------------*/

#endif /* _USE_MACROS */


static Err OpenDirectory(const char* pathP,
                         FileRef* dirRefP,
                         UInt16* volRefNumP)
/***********************************************************************
 *
 *  OpenDirectory
 *
 ***********************************************************************/
{
Err Result = errNone;
UInt32 volIterator = vfsIteratorStart;

#ifndef __RELEASE__
  if (pathP == NULL)
    printf("pathP == NULL\n");
  if (dirRefP == NULL)
    printf("dirRefP == NULL\n");
  if (volRefNumP == NULL)
    printf("volRefNumP == NULL\n");
#endif /* __RELEASE__ */

  //
  // Search directory among volumes
  //
  while (volIterator != vfsIteratorStop)
  {
    Result = VFSVolumeEnumerate(volRefNumP,
                                &volIterator);
    if (Result != errNone)
      return Result;

    if (VFSFileOpen(*volRefNumP,
                    pathP,
                    vfsModeDirExist,
                    dirRefP) == errNone)
    {
    	return errNone;
    }
  }

	return vfsErrFileNotFound;
}
/*----------------------------------------------------------------------------*/


static Err OpenFile(const char* pathP,
                    const char* filenameP,
                    FileRef* fileRefP,
                    UInt16 openMode,
                    Boolean create)
/***********************************************************************
 *
 *  OpenFile
 *
 ***********************************************************************/
#undef OPENFILE_DEBUG_ENABLED
//#define OPENFILE_DEBUG_ENABLED
#undef OPENFILE_TRACE_ENABLED
//#define OPENFILE_TRACE_ENABLED

#ifdef OPENFILE_TRACE_ENABLED
#  define OPENFILE_TRACE_SHOW_INT(value) TRACE_SHOW_INT("OpenFile", value)
#else /* OPENFILE_TRACE_ENABLED */
#  define OPENFILE_TRACE_SHOW_INT(value)
#endif /* OPENFILE_TRACE_ENABLED */
{
Err Result = errNone;
UInt16 volRefNum;
UInt32 volIterator = vfsIteratorStart;
FileRef dirRef;
Boolean dirFound = false;
Boolean dirCreated = false;
char* fullpathnameP;

  OPENFILE_TRACE_SHOW_INT(1);

#ifndef __RELEASE__
  if (pathP == NULL)
    printf("pathP == NULL\n");
  if (filenameP == NULL)
    printf("filenameP == NULL\n");
  if (fileRefP == NULL)
    printf("fileRefP == NULL\n");
#endif /* __RELEASE__ */

#ifdef OPENFILE_DEBUG_ENABLED
  TRACE_SHOW_TEXT("pathP", pathP);
  TRACE_SHOW_TEXT("filenameP", filenameP);
#endif /* OPENFILE_DEBUG_ENABLED */

  //
  // Search directory among volumes
  //
  while (volIterator != vfsIteratorStop)
  {
    Result = VFSVolumeEnumerate(&volRefNum,
                                &volIterator);
    if (Result != errNone)
    {
#ifdef OPENFILE_DEBUG_ENABLED
      TRACE_SHOW_HEX("VFSVolumeEnumerate FAILURE Result=", Result);
#endif /* OPENFILE_DEBUG_ENABLED */
      return Result;
    }

    Result = VFSFileOpen(volRefNum,
                         pathP,
                         vfsModeDirExist, //fopen
                         &dirRef);
    if (Result == errNone)
    {
      dirFound = true;
      break;
    }
  }

  OPENFILE_TRACE_SHOW_INT(2);

  Result = VFSFileClose(dirRef);
  if (Result != errNone)
  {
#ifdef OPENFILE_DEBUG_ENABLED
    TRACE_SHOW_HEX("VFSFileClose FAILURE Result=", Result);
#endif /* OPENFILE_DEBUG_ENABLED */
    return Result;
  }

  OPENFILE_TRACE_SHOW_INT(3);

  if (dirFound == false)
  {
    if (create == false)
      return vfsErrFileNotFound;

    OPENFILE_TRACE_SHOW_INT(4);

    //
    // Create directory on first volume
    //
    while (volIterator != vfsIteratorStop)
    {
      Result = VFSVolumeEnumerate(&volRefNum,
                                  &volIterator);
      if (Result != errNone)
        return Result;

      if (VFSDirCreate(volRefNum,
                       pathP) == errNone)
      {
        dirCreated = true;
        break;
      }
    }

    OPENFILE_TRACE_SHOW_INT(5);

    if (dirCreated == false)
      return vfsErrFileNotFound;
  }

  OPENFILE_TRACE_SHOW_INT(6);

  //
  // Create filename
  //
  fullpathnameP = (char*)MemPtrNew(PATHNAME_MAXSIZE + SIZETAB_FILENAME + 1);
  if (fullpathnameP == NULL)
    return memErrNotEnoughSpace;

  StrCopy(fullpathnameP,
          pathP);
  StrCat(fullpathnameP,
         filenameP);

  OPENFILE_TRACE_SHOW_INT(7);

  //
  // Open file
  //
  Result = VFSFileOpen(volRefNum,
                       fullpathnameP,
                       openMode,
                       fileRefP);
  if (Result == vfsErrFileNotFound)
  {
    if (create == true)
    {
      //
      // Create file
      //
      Result = VFSFileCreate(volRefNum,
                             fullpathnameP);
      if (Result == errNone)
      {
        Result = VFSFileOpen(volRefNum,
                             fullpathnameP,
                             openMode,
                             fileRefP);
      }
    }
  }

  OPENFILE_TRACE_SHOW_INT(8);

  MemPtrFree(fullpathnameP);

  return Result;
}
/*----------------------------------------------------------------------------*/


#ifndef _USE_MACROS

static Err CloseFile(FileRef fileRef)
/***********************************************************************
 *
 *  CloseFile
 *
 ***********************************************************************/
{
  return VFSFileClose(fileRef);
}
/*----------------------------------------------------------------------------*/

#endif /* _USE_MACROS */


tBool IsFileExist(const char* pathP,
                  const char* filenameP)
/***********************************************************************
 *
 *  IsFileExist
 *
 ***********************************************************************/
{
FileRef ref;

  if ( (pathP == NULL) || (filenameP == NULL) )
    return cFalse;

  if (OpenFile(pathP,
               filenameP,
               &ref,
               vfsModeRead,
               false) == errNone)
  {
    CloseFile(ref);
    return cTrue;
  }

  return cFalse;
}
/*----------------------------------------------------------------------------*/


tBool IsDirectoryExist(const char* pathP)
/***********************************************************************
 *
 *  IsDirectoryExist
 *
 ***********************************************************************/
{
FileRef dirRef;
UInt16 volRefNum;

#ifndef __RELEASE__
  if (pathP == NULL)
    printf("pathP == NULL\n");
#endif /* __RELEASE__ */

  if (OpenDirectory(pathP,
                    &dirRef,
                    &volRefNum) == errNone)
  {
  	VFSFileClose(dirRef);
  	return cTrue;
  }

  return cFalse;
}
/*----------------------------------------------------------------------------*/


Err DeleteFile(const char* pathP,
               const char* filenameP)
/***********************************************************************
 *
 *  DeleteFile
 *
 ***********************************************************************/
#undef DELETEFILE_TRACE_ENABLED
//#define DELETEFILE_TRACE_ENABLED

#ifdef DELETEFILE_TRACE_ENABLED
#  define DELETEFILE_TRACE_SHOW_INT(value) TRACE_SHOW_INT("DeleteFile", value)
#else
#  define DELETEFILE_TRACE_SHOW_INT(value)
#endif /* DELETEFILE_TRACE_ENABLED */

{
Err Result = errNone;
UInt16 volRefNum;
FileRef dirRef = 0;
char* fullfilenameP;

  DELETEFILE_TRACE_SHOW_INT(1);

  fullfilenameP = (char*)MemPtrNew(PATHNAME_MAXSIZE + SIZETAB_FILENAME + 1);
  if (fullfilenameP == NULL)
    return memErrNotEnoughSpace;

  DELETEFILE_TRACE_SHOW_INT(8);

  //
  // Delete File
  //
  do
  {
  	// Open directory
    Result = OpenDirectory(pathP,
                           &dirRef,
                           &volRefNum);
    if (Result != errNone) continue;

    DELETEFILE_TRACE_SHOW_INT(9);

    // Create full filename
    StrCopy(fullfilenameP,
            pathP);
    StrCat(fullfilenameP,
           filenameP);

  	Result = VFSFileDelete(volRefNum,
  	                       fullfilenameP);
    if (Result != errNone) continue;

    DELETEFILE_TRACE_SHOW_INT(10);
  }
  while (0);

  DELETEFILE_TRACE_SHOW_INT(11);

  if (dirRef)
  {
  	VFSFileClose(dirRef);
  }

  DELETEFILE_TRACE_SHOW_INT(12);

  MemPtrFree(fullfilenameP);

  return Result;
}
/*----------------------------------------------------------------------------*/


Err CreateDirectories(const char* palmPathP,
                      const char* capricePathP,
                      const char* diskPathP,
                      const char* screenshotPathP)
/***********************************************************************
 *
 *  CreateDirectories
 *
 ***********************************************************************/
{
Err Result = errNone;
UInt16 volRefNum;
FileRef dirRef;

  // Search palm directory among volumes
  Result = OpenDirectory(palmPathP,
                         &dirRef,
                         &volRefNum);
  if (Result != errNone)
    return Result;

  VFSFileClose(dirRef);

  // Open CaPriCe directory
  if (VFSFileOpen(volRefNum,
                  capricePathP,
                  vfsModeDirExist,
                  &dirRef) == errNone)
  {
    VFSFileClose(dirRef);
  }
  else
  {
    if (VFSDirCreate(volRefNum,
                     capricePathP) != errNone)
    {
      return vfsErrFileNotFound;
    }
  }

  // Open Disc directory
  if (VFSFileOpen(volRefNum,
                  diskPathP,
                  vfsModeDirExist,
                  &dirRef) == errNone)
  {
    VFSFileClose(dirRef);
  }
  else
  {
    if (VFSDirCreate(volRefNum,
                     diskPathP) != errNone)
    {
      return vfsErrFileNotFound;
    }
  }

  // Open Screenshot directory
  if (VFSFileOpen(volRefNum,
                  screenshotPathP,
                  vfsModeDirExist,
                  &dirRef) == errNone)
  {
    VFSFileClose(dirRef);
  }
  else
  {
    if (VFSDirCreate(volRefNum,
                     screenshotPathP) != errNone)
    {
      return vfsErrFileNotFound;
    }
  }

  return errNone;
}
/*----------------------------------------------------------------------------*/



Err EjectDisk(tDrive* nativedriveP,
              tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  EjectDisk
 *
 ***********************************************************************/
{
tDiskOperation* DiskOperationP;
Err Result;

  DiskOperationP = (tDiskOperation*)(NativeCPC->contextP + CONTEXT_OFFSET_DISKOPERATION);

  DiskOperationP->NativeCPC = NativeCPC;
  DiskOperationP->Drive = (tDrive*)nativedriveP;

  Result = Engine_DiskEject(DiskOperationP);

#if defined(_TESTU) && defined(_TRACE)
  if (Result >= appErrorClass)
  {
    TRACE_SHOW_HEX("TestU Native_DiskEject",
                   Result-testUErrorClass);
  }
#endif

  return Result;
}
/*----------------------------------------------------------------------------*/


Err LoadDiskImage(const char* pathP,
                  tDrive* nativedriveP,
                  tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  LoadDiskImage
 *
 ***********************************************************************/
{
tDrive* Drive = (tDrive*)nativedriveP;
tDiskOperation* DiskOperationP = NULL;
FileRef DiskImageRef;
UInt32 numBytesRead;
Err Result;
char* memP = NULL;

  Result = OpenFile(pathP,
                    (char*)Drive->filename,
                    &DiskImageRef,
                    vfsModeRead,
                    false);
  if (Result != errNone)
    return Result;

  do
  {
    memP = (char*)MemPtrNew(sizeof(Drive->filename));
    if (memP == NULL)
    {
    	Result = memErrNotEnoughSpace;
    	continue;
    }
    MemMove((tUChar*)memP,
            (tUChar*)Drive->filename,
            sizeof(Drive->filename));

    printf("SaveandExitDiskImage :\n");
    Result = SaveAndEjectDiskImage(pathP,
                                   nativedriveP,
                                   NativeCPC);

      printf("SaveandExitDiskImage %d:\n",Result);
    if (Result != errNone) continue;

    MemMove((tUChar*)Drive->filename,
            (tUChar*)memP,
            sizeof(Drive->filename));

    //
    // Prepare disk load operation
    //
    DiskOperationP = (tDiskOperation*)(EndianSwap32(NativeCPC->contextP) + CONTEXT_OFFSET_DISKOPERATION);

    DiskOperationP->Drive = Drive;
    DiskOperationP->NativeCPC = NativeCPC;

    //
    // Load entire disk image
    //
    // Get File size
    Result = VFSFileSize(DiskImageRef,
                         &DiskOperationP->disk_size);
    if (Result != errNone) continue;

    // Allocate memory for whole disk data
    MemPtrNewLarge(DiskOperationP->disk_size,
                   (void**)&DiskOperationP->DiskContentP);
    if (DiskOperationP->DiskContentP == NULL)
    {
      FrmAlert(NotEnoughMemoryAlert);
      Result = memErrNotEnoughSpace;
      continue;
    }

    // Read entire disk image file
    VFSFileRead(DiskImageRef,
                DiskOperationP->disk_size,
                (void*)DiskOperationP->DiskContentP,
                &numBytesRead);
    // Check read
    if (numBytesRead != DiskOperationP->disk_size)
    {
      FrmAlert(InvalidImageFormatAlert);
      Result = vfsErrBadData;
      continue;
    }

    //
    // Disk load operation
    //
    Result = Engine_DiskLoad(DiskOperationP);

#if defined(_TESTU) && defined(_TRACE)
    if (Result >= appErrorClass)
    {
      TRACE_SHOW_HEX("TestU Native_DiskLoad",
                     Result-testUErrorClass);
    }
#endif

    if (Result != errNone)
    {
      FrmAlert(InvalidImageFormatAlert);
      continue;
    }

  }
  while (0);

  if (memP != NULL)
  {
    MemPtrFree(memP);
  }

  // Fermer le fichier image
  CloseFile(DiskImageRef);

  return Result;
}
/*----------------------------------------------------------------------------*/


Err LoadDiskImageFromMemory(tDrive* nativedriveP,
                            tNativeCPC* NativeCPC,
                            tULong DiskSize,
                            tUChar* DiskContentP)
/***********************************************************************
 *
 *  LoadDiskImageFromMemory
 *
 ***********************************************************************/
{
tDrive* Drive = (tDrive*)EndianSwap32(nativedriveP);
tDiskOperation* DiskOperationP;
Err Result;

  //
  // Prepare disk load operation
  //
  DiskOperationP = (tDiskOperation*)(EndianSwap32(NativeCPC->contextP) + CONTEXT_OFFSET_DISKOPERATION);

  DiskOperationP->Drive = Drive;
  DiskOperationP->NativeCPC = NativeCPC;
  DiskOperationP->disk_size = EndianSwap32(DiskSize);
  DiskOperationP->DiskContentP = (tUChar*)EndianSwap32(DiskContentP);

  //
  // Disk load operation
  //
  Result = Engine_DiskLoad(DiskOperationP);

#ifdef _TESTU
  if (Result >= appErrorClass)
  {
    TRACE_SHOW_HEX("TestU Native_DiskLoad", Result-testUErrorClass);
  }
#endif /* _TESTU */

  if (Result != errNone)
  {
    FrmAlert(InvalidImageFormatAlert);
  }

  return Result;
}
/*----------------------------------------------------------------------------*/

UInt16  ReadDiskCatalogue(tDriveEnum drive, tUChar User, char** CatP, tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  ReadDiskCatalogue
 *
 ***********************************************************************/
{
  tDiskOperation* DiskOperationP = (tDiskOperation*)(NativeCPC->contextP + CONTEXT_OFFSET_DISKOPERATION);
  memset(DiskOperationP, 0, sizeof(tDiskOperation));
  DiskOperationP->NativeCPC = NativeCPC;
  DiskOperationP->Drive = (drive == DriveA) ? NativeCPC->DriveA : NativeCPC->DriveB;

  // Read Catalogue
  Engine_DiskReadCatalogue(DiskOperationP);


  UInt16 cat_size = DiskOperationP->NbCatalogueEntries * CPC_FILE_LENGTH;

  // Add catalog entries
  if (*CatP != NULL)
    MemPtrFree(*CatP);

  // Empty dir
  if (cat_size == 0)
    return (0);

  // get some memory
  *CatP = (char*)MemPtrNew(cat_size);
  if (*CatP == NULL)
    return 0;

  char* cat = *CatP;
  memset(cat,0, cat_size);
  for (tULong Loop=0; Loop < DiskOperationP->NbCatalogueEntries; Loop++)
  {
    // matching user ?
    if (DiskOperationP->CatalogueEntryUser[Loop] == User)
    {
      char *Entry = (char*)DiskOperationP->CatalogueEntries[Loop];
      printf("Entry %s\n", Entry);
      StrNCopy(cat, Entry, 8);
      cat[8]='.';
      StrNCopy(cat+9, Entry+8, 3);
      cat += CPC_FILE_LENGTH;
    }
  }
  return DiskOperationP->NbCatalogueEntries;
}
/***********************************************************************/

Err LoadKeymapFromConfigFile(tDrive* nativedriveP, const char* Key, char** Settings)
/***********************************************************************
 *
 *  LoadKeymapFromConfigFile
 *
 ***********************************************************************/
{
  char* filenameP;
  char* fullfilenameP;
  char* extP;
  Err Result;
  static char LocalSettings[16];

  if (IsDriveFilenameExist(nativedriveP) == cFalse)
    return (errBadName);

  fullfilenameP = (char*)MemPtrNew(PATHNAME_MAXSIZE + SIZETAB_FILENAME + 1);
  if (fullfilenameP == NULL)
    return (memErrNotEnoughSpace);

  filenameP = (char*)MemPtrNew(MAX_FILE_NAME * sizeof(Char));
  if (filenameP == NULL)
  {
    free(fullfilenameP);
    return (memErrNotEnoughSpace);
  }

  GetDriveFilename(nativedriveP, filenameP);

  extP = strrchr(filenameP,'.');
  strcpy(extP,KEYMAPPING_EXTENSION);

  strcpy(fullfilenameP, DEFAULT_KEYMAPPING_PATH);
  strcat(fullfilenameP, filenameP);
  //printf("Looking for Keymap file: %s\n",fullfilenameP);

  //int   ini_gets(const mTCHAR *Section, const mTCHAR *Key, const mTCHAR *DefValue, mTCHAR *Buffer, int BufferSize, const mTCHAR *Filename);
  Result = ini_gets("KEYMAPPING", Key, "", LocalSettings, 16, fullfilenameP);

  free (fullfilenameP);
  free (filenameP);

  if (Result)
  {
    //printf("Success: %s\n",LocalSettings);
    *Settings = LocalSettings;
    return (errNone);
  }
  else
  {
    //printf("Fail\n");
    *Settings = NULL;
    return (errBadName);
  }
}



Err SaveDiskImage(const char* pathP,
                  tDrive* nativedriveP,
                  tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  SaveDiskImage
 *
 ***********************************************************************/
{
tDrive* DriveP = (tDrive*)EndianSwap32(nativedriveP);
tUChar* diskdataP;
Err Result;
UInt32 disksize;
UInt32 numBytesWritten;
FileRef DiskImageRef;
tDiskOperation* DiskOperationP = NULL;


  if ( (DriveP == NULL) || (pathP == NULL) || (!DriveP->filename[0]))
    return vfsErrFileNotFound;

  // Si le disque n'a pas �t� modifi�, ne rien faire
  if (DriveP->altered == 0)
    return errNone;

  // Ouvrir le fichier image
  Result = OpenFile(pathP,
                    (char*)DriveP->filename,
                    &DiskImageRef,
                    vfsModeReadWrite,
                    true);
  if (Result != errNone)
    return Result;

  do
  {
    // Sauvegarder le fichier image
    DiskOperationP = (tDiskOperation*)(EndianSwap32(NativeCPC->contextP) + CONTEXT_OFFSET_DISKOPERATION);

    DiskOperationP->Drive = DriveP;
#ifdef _TRACE
    DiskOperationP->NativeCPC = NativeCPC;
#endif /* _TRACE */

Result = Engine_DiskSave(DiskOperationP);

#ifdef _TESTU
    if (Result >= appErrorClass)
    {
      TRACE_SHOW_HEX("TestU Native_DiskSave", Result-testUErrorClass);
    }
#endif

    if (Result != errNone)
    {
      FrmAlert(BadWriteImageAlert);
      continue;
    }

    //
    // Save entire disk file
    //
    diskdataP = (tUChar*)EndianSwap32(DriveP->dataP);
    disksize = (UInt32)EndianSwap32(DriveP->data_size);
    Result = VFSFileWrite(DiskImageRef,
                          disksize,
                          diskdataP,
                          &numBytesWritten);
#ifndef __RELEASE__
    if (Result != errNone)
      printf("VFSFileWrite != errNone\n");
#endif /* __RELEASE__ */
    if (numBytesWritten != disksize)
    {
      Result = BadWriteImageAlert;
    }
    else
    {
      DriveP->altered = 0;
    }
  }
  while (0);

  // Fermer le fichier image
  CloseFile(DiskImageRef);

  return Result;
}
/*----------------------------------------------------------------------------*/


Err SaveAndEjectDiskImage(const char* pathP,
                          tDrive* nativedriveP,
                          tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  SaveAndEjectDiskImage
 *
 ***********************************************************************/
{
	// Save disk if possible before eject
  SaveDiskImage(pathP,
                nativedriveP,
                NativeCPC);

  return (EjectDisk(nativedriveP,
                    NativeCPC));
}
/*----------------------------------------------------------------------------*/


void SwapDrive(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  SwapDrive
 *
 ***********************************************************************/
{
tDrive* driveTempP;

  // Swap Drives not regarding endianess
  driveTempP = NativeCPC->DriveA;
  NativeCPC->DriveA = NativeCPC->DriveB;
  NativeCPC->DriveB = driveTempP;
#ifdef __PALMOS__
  FrmUpdateForm(MainForm,
                SubPanelRedrawUpdateCode);
#endif
}
/*----------------------------------------------------------------------------*/


Boolean SaveAndEjectBoth(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  SaveAndEjectBoth
 *
 ***********************************************************************/
{
Boolean Change = false;

  // Drive A
  if (IsDriveFilenameExist(NativeCPC->DriveA) == cTrue)
  {
    SaveAndEjectDiskImage((char*)prefP->FilesPathname,
                          NativeCPC->DriveA,
                          NativeCPC);
    Change = true;
  }

  // Drive B
  if (IsDriveFilenameExist(NativeCPC->DriveB) == cTrue)
  {
    SaveAndEjectDiskImage((char*)prefP->FilesPathname,
                          NativeCPC->DriveB,
                          NativeCPC);
    Change = true;
  }

  return (Change);
}
/*----------------------------------------------------------------------------*/


Err CreateDisk(tDrive* nativedriveP,
               tUChar FormatType,
               tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  CreateDisk
 *
 ***********************************************************************/
{
tDiskOperation* DiskOperationP;
Err Result;

  DiskOperationP = (tDiskOperation*)(EndianSwap32(NativeCPC->contextP) + CONTEXT_OFFSET_DISKOPERATION);
  DiskOperationP->Drive = (tDrive*)EndianSwap32(nativedriveP);
  DiskOperationP->NativeCPC = NativeCPC;
  DiskOperationP->FormatType = FormatType;
  DiskOperationP->DiskFormatTableP = (tDiskFormat*)&disk_format[0];

  Result = Engine_DiskFormat(DiskOperationP);

#ifdef _TESTU
  if (Result >= appErrorClass)
  {
    TRACE_SHOW_HEX("TestU Native_DiskFormat",
                   Result-testUErrorClass);
  }
#endif

  return Result;
}
/*----------------------------------------------------------------------------*/


tCPCSaveReturnCode SaveCPC(const char* pathP,
                           const char* filenameP,
                           tUChar* contextP)
/***********************************************************************
 *
 *  SaveCPC
 *
 ***********************************************************************/
{
tCPCSaveReturnCode ReturnCode = Save_OK;
Err Result;
FileRef DiskImageRef;
UInt32 numBytesWritten;
tUChar* memoryP;
UInt32 size;
tDrive* driveP;
tContextHeader* contextHeaderP;
tULong Flags = 0;

  // Create result file
  Result = OpenFile(pathP,
                    filenameP,
                    &DiskImageRef,
                    vfsModeReadWrite,
                    true);
  if (Result != errNone)
    return File_Not_Created;

  // Prepare flags
#ifdef _PROFILE
  Flags |= CONTEXT_FLAG_PROFILE;
#endif /* _PROFILE */
#ifdef _DEBUG
  Flags |= CONTEXT_FLAG_DEBUG;
#endif /* _DEBUG */
#ifdef _TESTU
  Flags |= CONTEXT_FLAG_TESTU;
#endif /* _TESTU */
#ifdef _TRACE
  Flags |= CONTEXT_FLAG_TRACE;
#endif /* _TRACE */

  // Prepare context header
  contextHeaderP = (tContextHeader*)(contextP + CONTEXT_OFFSET_HEADER);
  contextHeaderP->Flags = EndianSwap32(Flags);
  contextHeaderP->CPCModel = prefP->CPCModel;
  contextHeaderP->ScreenType = prefP->ScreenType;
  contextHeaderP->UseParados = prefP->UseParados;
  contextHeaderP->Use64kMemoryExtension = prefP->Use64kMemoryExtension;
  contextHeaderP->EmuKeyFxState = EmulatorKeys[KEYINDEX_FX].KeyStatus == KeyPressed ? 1 : 0;
  contextHeaderP->EmuKeyJoystickState = EmulatorKeys[KEYINDEX_JOYSTICK].KeyStatus == KeyPressed ? 1 : 0;
  contextHeaderP->EmuKeyAutoToggleState = EmulatorKeys[KEYINDEX_AUTOTOGGLE].KeyStatus == KeyPressed ? 1 : 0;
  contextHeaderP->Mode2AntiAliasing = prefP->Mode2AntiAliasing;

  do
  {
    // Save of the context
    Result = VFSFileWrite(DiskImageRef,
                          SIZETAB_CONTEXT,
                          contextP,
                          &numBytesWritten);
#ifndef __RELEASE__
    if (Result != errNone)
      printf("VFSFileWrite != errNone\n");
#endif /* __RELEASE__ */
    if (numBytesWritten != SIZETAB_CONTEXT)
    {
      ReturnCode = Write_Error;
      continue;
    }

    // Save Drive A Data
    driveP = (tDrive*)(contextP + CONTEXT_OFFSET_DRIVE_A);
    memoryP = (tUChar*)EndianSwap32(driveP->dataP);
    size = EndianSwap32(driveP->data_size);
    if (size)
    {
      Result = VFSFileWrite(DiskImageRef,
                            size,
                            memoryP,
                            &numBytesWritten);
#ifndef __RELEASE__
      if (Result != errNone)
        printf("VFSFileWrite != errNone\n");
#endif /* __RELEASE__ */
      if (numBytesWritten != size)
      {
        ReturnCode = Write_Error;
        continue;
      }
    }

    // Save Drive B Data
    driveP = (tDrive*)(contextP + CONTEXT_OFFSET_DRIVE_B);
    memoryP = (tUChar*)EndianSwap32(driveP->dataP);
    size = EndianSwap32(driveP->data_size);
    if (size)
    {
      Result = VFSFileWrite(DiskImageRef,
                            size,
                            memoryP,
                            &numBytesWritten);
#ifndef __RELEASE__
      if (Result != errNone)
        printf("VFSFileWrite != errNone\n");
#endif /* __RELEASE__ */
      if (numBytesWritten != size)
      {
        ReturnCode = Write_Error;
        continue;
      }
    }
  }
  while (0);

  VFSFileClose(DiskImageRef);

  // Something wrong...Delete potential invalid file
  if (Result != errNone)
  {
    DeleteFile(pathP, filenameP);
  }

#ifdef _TRACE
  if (Result != errNone) TRACE_SHOW_HEX("SaveCPC End",
                                        Result);
#endif /* _TRACE */

  return ReturnCode;
}
/*----------------------------------------------------------------------------*/


tCPCRestoreReturnCode RestoreCPC(const char* pathP,
                                 const char* filenameP,
                                 tEmulatorKeysStatus* KeyStatusP,
                                 tPreferences* requestedPrefP,
                                 tUChar* contextP)
/***********************************************************************
 *
 *  RestoreCPC
 *
 ***********************************************************************/
#undef RESTORECPC_TRACE_ENABLED
//#define RESTORECPC_TRACE_ENABLED

#ifdef RESTORECPC_TRACE_ENABLED
#  define RESTORECPC_TRACE_SHOW_INT(value) TRACE_SHOW_INT("RestoreCPC", value)
#else
#  define RESTORECPC_TRACE_SHOW_INT(value)
#endif /* RESTORECPC_TRACE_ENABLED */
{
//tNativeCPC* NativeCPC = (tNativeCPC*)(contextP + CONTEXT_OFFSET_NATIVECPC);
const tRestoreCPCSection* sectionP;
tCPCRestoreReturnCode ReturnCode = Restore_OK;
Err Result;
FileRef ContextFileRef;
tUChar* memP = NULL;
tUChar* readNativeCPCP = NULL;
tUChar* readContextP;
tContextHeader* contextHeaderP;
UInt32 numBytesRead;
tReindexOperationParameters ReindexParam;
UInt8 Loop;
tBool loadDiskImageA;
tBool loadDiskImageB;

  RESTORECPC_TRACE_SHOW_INT(1);

  Result = OpenFile(pathP,
                    filenameP,
                    &ContextFileRef,
                    vfsModeRead,
                    false);
  if (Result != errNone)
    return File_Not_Found;

  RESTORECPC_TRACE_SHOW_INT(2);

  contextHeaderP = (tContextHeader*)(contextP + CONTEXT_OFFSET_HEADER);

  do
  {
    //
    // Header
    //
    memP = (tUChar*)MemPtrNew(CONTEXT_SIZE_HEADER);
    if (memP == NULL)
    {
    	Result = memErrNotEnoughSpace;
    	continue;
    }
    Result = VFSFileRead(ContextFileRef,
                         CONTEXT_SIZE_HEADER,
                         (void*)memP,
                         &numBytesRead);
#ifndef __RELEASE__
    if (Result != errNone)
      printf("VFSFileRead != errNone\n");
#endif /* __RELEASE__ */
    if (numBytesRead != CONTEXT_SIZE_HEADER)
    {
      Result = vfsErrBadData;
      continue;
    }

    // Retreive requested parameters
    requestedPrefP->CPCModel  = ((tContextHeader*)memP)->CPCModel;
    requestedPrefP->ScreenType  = ((tContextHeader*)memP)->ScreenType;
    requestedPrefP->UseParados  = ((tContextHeader*)memP)->UseParados;
    requestedPrefP->Use64kMemoryExtension  = ((tContextHeader*)memP)->Use64kMemoryExtension;
    requestedPrefP->Mode2AntiAliasing  = ((tContextHeader*)memP)->Mode2AntiAliasing;

    // Check CaPriCe version
    if (StrCompare(((tContextHeader*)memP)->version,
                   contextHeaderP->version))
    {
      ReturnCode = Wrong_Version;
      continue;
    }
    // Check CPC Model
    if (requestedPrefP->CPCModel != prefP->CPCModel)
    {
      ReturnCode = Wrong_CPC_Model;
      continue;
    }
    // Restore keyboard status
    KeyStatusP->FxKeyStatus = ((tContextHeader*)memP)->EmuKeyFxState ? KeyPressed : KeyReleased;
    KeyStatusP->JoystickKeyStatus = ((tContextHeader*)memP)->EmuKeyJoystickState ? KeyPressed : KeyReleased;
    KeyStatusP->AutoToggleKeyStatus = ((tContextHeader*)memP)->EmuKeyAutoToggleState ? KeyPressed : KeyReleased;

    RESTORECPC_TRACE_SHOW_INT(3);

    //
    // Pass through Resources + DiskOperation
    //
    Result = VFSFileSeek(ContextFileRef,
                         vfsOriginCurrent,
                         CONTEXT_SIZE_RESOURCES + CONTEXT_SIZE_DISKOPERATION);
#ifndef __RELEASE__
    if (Result != errNone)
      printf("VFSFileSeek != errNone\n");
#endif /* __RELEASE__ */

    RESTORECPC_TRACE_SHOW_INT(4);

    //
    // NativeCPC
    //
#define RESTORECPC_NATIVE_CPC_SIZE    (STATIC_SIZE_NATIVECPC + DYNAMIC_SIZE_NATIVECPC)
#define RESTORECPC_NATIVE_CPC_OFFSET  (CONTEXT_OFFSET_NATIVECPC + STATIC_SIZE_NATIVECPC)

    readNativeCPCP = (tUChar*)MemPtrNew(RESTORECPC_NATIVE_CPC_SIZE);
    if (readNativeCPCP == NULL)
    {
  	  Result = memErrNotEnoughSpace;
  	  continue;
  	}

    Result = VFSFileRead(ContextFileRef,
                         RESTORECPC_NATIVE_CPC_SIZE,
                         (void*)readNativeCPCP,
                         &numBytesRead);
#ifndef __RELEASE__
    if (Result != errNone)
      printf("VFSFileRead != errNone\n");
#endif /* __RELEASE__ */

    if ( (numBytesRead != RESTORECPC_NATIVE_CPC_SIZE) ||
         (Result != errNone) )
    {
      ReturnCode = Bad_Data;
      continue;
    }

    // Save read context pointer
    readContextP = (tUChar*)EndianSwap32(((tNativeCPC*)readNativeCPCP)->contextP);

    // Prepare reindex parameters
#ifdef SIM
    ReindexParam.srcContextBase = (uint64_t)readContextP;
    ReindexParam.dstContextBase = (uint64_t)contextP;
#else
    ReindexParam.srcContextBase = (UInt32)readContextP;
    ReindexParam.dstContextBase = (UInt32)contextP;
#endif
    ReindexParam.contextSize = SIZETAB_CONTEXT;
    ReindexParam.native = cTrue;

    // Re-index dynamic area
    ReindexParam.srcAreaP = (void**)(readNativeCPCP + STATIC_SIZE_NATIVECPC);
    ReindexParam.dstAreaP = (void**)(contextP + RESTORECPC_NATIVE_CPC_OFFSET);
    ReindexParam.areaSize = DYNAMIC_SIZE_NATIVECPC;
    ReindexPointersArea(&ReindexParam);

    // Restore NativeCPC Variables
#define RESTORECPC_NATIVE_CPC_VAR_SIZE   (CONTEXT_SIZE_NATIVECPC - STATIC_SIZE_NATIVECPC - DYNAMIC_SIZE_NATIVECPC)
#define RESTORECPC_NATIVE_CPC_VAR_OFFSET (CONTEXT_OFFSET_NATIVECPC + STATIC_SIZE_NATIVECPC + DYNAMIC_SIZE_NATIVECPC)
    Result = VFSFileRead(ContextFileRef,
                         RESTORECPC_NATIVE_CPC_VAR_SIZE,
                         (void*)(contextP + RESTORECPC_NATIVE_CPC_VAR_OFFSET),
                         &numBytesRead);
#ifndef __RELEASE__
    if (Result != errNone)
      printf("VFSFileRead != errNone\n");
#endif /* __RELEASE__ */
    if (Result != errNone) continue;

    RESTORECPC_TRACE_SHOW_INT(5);

    //
    // Read sections
    //
    for (Loop=NB_RESTORECPC_SECTIONS, sectionP=RestoreCPCSections;
         Loop;
         Loop--, sectionP++)
    {
      RESTORECPC_TRACE_SHOW_INT(500 + NB_RESTORECPC_SECTIONS - Loop);

      ReturnCode = RestoreCPC_Section(ContextFileRef,
                                      contextP,
                                      readContextP,
                                      sectionP);
      if (ReturnCode != Restore_OK) break;
    }
    if (ReturnCode != Restore_OK) continue;

    RESTORECPC_TRACE_SHOW_INT(6);

    //
    // Drive A
    //
    ReturnCode = RestoreCPC_Drive(ContextFileRef,
                                  (tDrive*)EndianSwap32(contextP + CONTEXT_OFFSET_DRIVE_A),
                                  NativeCPC,
                                  &loadDiskImageA);
    if (ReturnCode != Restore_OK) continue;

    RESTORECPC_TRACE_SHOW_INT(7);

    //
    // Drive B
    //
    ReturnCode = RestoreCPC_Drive(ContextFileRef,
                                  (tDrive*)EndianSwap32(contextP + CONTEXT_OFFSET_DRIVE_B),
                                  NativeCPC,
                                  &loadDiskImageB);
    if (ReturnCode != Restore_OK) continue;

    RESTORECPC_TRACE_SHOW_INT(8);

    //
    // Mem Banks
    //
    ReturnCode = RestoreCPC_MemBank(ContextFileRef,
                                    contextP,
                                    readContextP,
                                    readNativeCPCP);
    if (ReturnCode != Restore_OK) continue;

    RESTORECPC_TRACE_SHOW_INT(9);

    //
    // Drive A Disk Image
    //
    if (loadDiskImageA == cTrue)
    {
      RESTORECPC_TRACE_SHOW_INT(10);

      ReturnCode = RestoreCPC_DiskImage(ContextFileRef,
                                        (tDrive*)EndianSwap32(contextP + CONTEXT_OFFSET_DRIVE_A),
                                        NativeCPC);
      if (ReturnCode != Restore_OK) continue;
    }

    RESTORECPC_TRACE_SHOW_INT(11);

    //
    // Drive B Disk Image
    //
    if (loadDiskImageB == cTrue)
    {
      RESTORECPC_TRACE_SHOW_INT(12);

      ReturnCode = RestoreCPC_DiskImage(ContextFileRef,
                                        (tDrive*)EndianSwap32(contextP + CONTEXT_OFFSET_DRIVE_B),
                                        NativeCPC);
      if (ReturnCode != Restore_OK) continue;
    }

    RESTORECPC_TRACE_SHOW_INT(13);

    NativeCPC->RestorationPerformed = 1;
  }
  while (0);

  RESTORECPC_TRACE_SHOW_INT(14);

  if (memP != NULL)
  {
    MemPtrFree(memP);
  }

  if (readNativeCPCP != NULL)
  {
    MemPtrFree(readNativeCPCP);
  }

  VFSFileClose(ContextFileRef);

  RESTORECPC_TRACE_SHOW_INT(15);

  return (ReturnCode);
}
/*----------------------------------------------------------------------------*/


static tCPCRestoreReturnCode RestoreCPC_Section(FileRef ContextFileRef,
                                                tUChar* ContextP,
                                                tUChar* FileContextP,
                                                const tRestoreCPCSection* sectionP)
/***********************************************************************
 *
 *  RestoreCPC_Section
 *
 ***********************************************************************/
#undef RESTORECPC_SECTION_TRACE_ENABLED
//#define RESTORECPC_SECTION_TRACE_ENABLED

#ifdef RESTORECPC_SECTION_TRACE_ENABLED
#  define RESTORECPC_SECTION_TRACE_SHOW_INT(value) TRACE_SHOW_INT("RestoreCPC_Section", value)
#else
#  define RESTORECPC_SECTION_TRACE_SHOW_INT(value)
#endif /* RESTORECPC_SECTION_TRACE_ENABLED */
{
tReindexOperationParameters ReindexParam;
UInt32 numBytesRead;
UInt32 Size;
Err Result;

  RESTORECPC_SECTION_TRACE_SHOW_INT(1);

  // Pass through STATIC areaSize
  if (sectionP->StaticAreaSize)
  {
    Result = VFSFileSeek(ContextFileRef,
                         vfsOriginCurrent,
                         sectionP->StaticAreaSize);
#ifndef __RELEASE__
    if (Result != errNone)
      printf("VFSFileSeek != errNone\n");
#endif /* __RELEASE__ */
  }

  RESTORECPC_SECTION_TRACE_SHOW_INT(2);

  // Read dynamic and variables areas
  Size = sectionP->Size - sectionP->StaticAreaSize;
  if (Size)
  {
    RESTORECPC_SECTION_TRACE_SHOW_INT(3);

    Result = VFSFileRead(ContextFileRef,
                         Size,
                         (void*)(ContextP + sectionP->Offset + sectionP->StaticAreaSize),
                         &numBytesRead);
#ifndef __RELEASE__
    if (Result != errNone)
      printf("VFSFileRead != errNone\n");
#endif /* __RELEASE__ */
    if (numBytesRead != Size)
    {
      return (Bad_Data);
    }

    RESTORECPC_SECTION_TRACE_SHOW_INT(4);

    // Reindex dynamic area
    if (sectionP->DynamicAreaSize)
    {
      RESTORECPC_SECTION_TRACE_SHOW_INT(5);

      // Prepare reindex parameters
#ifdef SIM
      ReindexParam.srcContextBase = (uint64_t)FileContextP;
      ReindexParam.dstContextBase = (uint64_t)ContextP;
#else
      ReindexParam.srcContextBase = (UInt32)FileContextP;
      ReindexParam.dstContextBase = (UInt32)ContextP;
#endif

      ReindexParam.contextSize = SIZETAB_CONTEXT;
      ReindexParam.native = cTrue;

      // Re-index dynamic area
      ReindexParam.srcAreaP = (void**)(ContextP + sectionP->Offset + sectionP->StaticAreaSize);
      ReindexParam.dstAreaP = ReindexParam.srcAreaP;
      ReindexParam.areaSize = sectionP->DynamicAreaSize;
      ReindexPointersArea(&ReindexParam);
    }
  }

  RESTORECPC_SECTION_TRACE_SHOW_INT(6);

  return (Restore_OK);
}
/*----------------------------------------------------------------------------*/


static tCPCRestoreReturnCode RestoreCPC_Drive(FileRef ContextFileRef,
                                              tDrive* NativeDriveP,
                                              tNativeCPC* NativeCPC,
                                              tBool* LoadDiskImageP)
/***********************************************************************
 *
 *  RestoreCPC_Drive
 *
 ***********************************************************************/
#undef RESTORECPC_DRIVE_TRACE_ENABLED
//#define RESTORECPC_DRIVE_TRACE_ENABLED

#ifdef RESTORECPC_DRIVE_TRACE_ENABLED
#  define RESTORECPC_DRIVE_TRACE_SHOW_INT(value) TRACE_SHOW_INT("RestoreCPC_Drive", value)
#else
#  define RESTORECPC_DRIVE_TRACE_SHOW_INT(value)
#endif /* RESTORECPC_DRIVE_TRACE_ENABLED */
{
tDrive* driveP = (tDrive*)EndianSwap32(NativeDriveP);
tDrive* readDriveP;
UInt32 numBytesRead;
Err Result;
tCPCRestoreReturnCode ReturnCode = Restore_OK;

#ifndef __RELEASE__
  if (NativeCPC == cNull)
    printf("NativeCPC == cNull\n");
  if (driveP == cNull)
    printf("driveP == cNull\n");
  if (LoadDiskImageP == cNull)
    printf("LoadDiskImageP == cNull\n");
#endif /* __RELEASE__ */

  *LoadDiskImageP = cFalse;

  RESTORECPC_DRIVE_TRACE_SHOW_INT(1);

  // Allocate memory to read Drive data
  MemPtrNewLarge(CONTEXT_SIZE_DRIVE, (void**)&readDriveP);
  if (readDriveP == NULL)
  {
    return (Not_Enough_Memory);
  }

  RESTORECPC_DRIVE_TRACE_SHOW_INT(4);

  do
  {
    // Read drive data
    Result = VFSFileRead(ContextFileRef,
                         CONTEXT_SIZE_DRIVE,
                         (void*)readDriveP,
                         &numBytesRead);
#ifndef __RELEASE__
    if (Result != errNone)
      printf("VFSFileRead != errNone\n");
#endif /* __RELEASE__ */
    if (numBytesRead != CONTEXT_SIZE_DRIVE)
    {
    	ReturnCode = Bad_Data;
    	continue;
    }

    RESTORECPC_DRIVE_TRACE_SHOW_INT(5);

    if (readDriveP->dataP)
    {
      RESTORECPC_DRIVE_TRACE_SHOW_INT(6);

  	  MemMove((tUChar*)driveP,
  	          (tUChar*)readDriveP,
  	          CONTEXT_SIZE_DRIVE);

  	  *LoadDiskImageP = cTrue;

      RESTORECPC_DRIVE_TRACE_SHOW_INT(7);
    }
  }
  while(0);

  MemPtrFree(readDriveP);

  RESTORECPC_DRIVE_TRACE_SHOW_INT(8);

  return ReturnCode;
}
/*----------------------------------------------------------------------------*/


static tCPCRestoreReturnCode RestoreCPC_DiskImage(FileRef ContextFileRef,
                                                  tDrive* NativeDriveP,
                                                  tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  RestoreCPC_DiskImage
 *
 ***********************************************************************/
#undef RESTORECPC_DISKIMAGE_TRACE_ENABLED
//#define RESTORECPC_DISKIMAGE_TRACE_ENABLED

#ifdef RESTORECPC_DISKIMAGE_TRACE_ENABLED
#  define RESTORECPC_DISKIMAGE_TRACE_SHOW_INT(value) TRACE_SHOW_INT("RestoreCPC_Drive", value)
#else
#  define RESTORECPC_DISKIMAGE_TRACE_SHOW_INT(value)
#endif /* RESTORECPC_DISKIMAGE_TRACE_ENABLED */
{
tDrive* driveP = (tDrive*)EndianSwap32(NativeDriveP);
tUChar* memP;
UInt32 size;
UInt32 numBytesRead;
Err Result;

  RESTORECPC_DISKIMAGE_TRACE_SHOW_INT(1);

  // Get size of disk image
  size = EndianSwap32(driveP->data_size);

  // Something to read...
  if (size)
  {
    RESTORECPC_DISKIMAGE_TRACE_SHOW_INT(2);

    MemPtrNewLarge(size, (void**)&memP);
    if (memP == NULL)
    {
      return (Not_Enough_Memory);
    }

    RESTORECPC_DISKIMAGE_TRACE_SHOW_INT(3);

    // Read disk image
    driveP->dataP = (tUChar*)EndianSwap32(memP);
    Result = VFSFileRead(ContextFileRef,
                         size,
                         (void*)memP,
                         &numBytesRead);
#ifndef __RELEASE__
    if (Result != errNone)
      printf("VFSFileRead != errNone\n");
#endif /* __RELEASE__ */
    if (numBytesRead != size)
    {
      return (Bad_Data);
    }

    RESTORECPC_DISKIMAGE_TRACE_SHOW_INT(4);

    // Initialize native drive structure
    if (LoadDiskImageFromMemory(NativeDriveP,
                                NativeCPC,
                                driveP->data_size,
                                driveP->dataP) != errNone)
    {
      RESTORECPC_DISKIMAGE_TRACE_SHOW_INT(5);

      EjectDisk(NativeDriveP,
                NativeCPC);

      SetDriveFilename(NativeDriveP,
                       NODISK_FILENAME);
    }

    RESTORECPC_DISKIMAGE_TRACE_SHOW_INT(6);
  }

  RESTORECPC_DISKIMAGE_TRACE_SHOW_INT(7);

  return (Restore_OK);
}
/*----------------------------------------------------------------------------*/


static tCPCRestoreReturnCode RestoreCPC_MemBank(FileRef ContextFileRef,
                                                tUChar* ContextP,
                                                tUChar* FileContextP,
                                                tUChar* readNativeCPCP)
/***********************************************************************
 *
 *  RestoreCPC_MemBank
 *
 ***********************************************************************/
#undef RESTORECPC_MEMBANK_TRACE_ENABLED
//#define RESTORECPC_MEMBANK_TRACE_ENABLED

#ifdef RESTORECPC_MEMBANK_TRACE_ENABLED
#  define RESTORECPC_MEMBANK_TRACE_SHOW_INT(value) TRACE_SHOW_INT("RestoreCPC_MemBank", value)
#else
#  define RESTORECPC_MEMBANK_TRACE_SHOW_INT(value)
#endif /* RESTORECPC_MEMBANK_TRACE_ENABLED */
{
tCPCRestoreReturnCode ReturnCode = Restore_OK;
Err Result;
UInt32 Loop;
UInt32 numBytesRead;
//tNativeCPC* NativeCPC = (tNativeCPC*)(ContextP + CONTEXT_OFFSET_NATIVECPC);
tUChar** FileMemmapROM;
tUChar* FileExpansionROM;

  RESTORECPC_MEMBANK_TRACE_SHOW_INT(1);

  FileExpansionROM = ((tNativeCPC*)readNativeCPCP)->pbExpansionROM;

  do
  {
    //
    // Read MEMMAP_ROM
    //
    FileMemmapROM = (tUChar**)MemPtrNew(CONTEXT_SIZE_MEMMAP_ROM);
    if (FileMemmapROM == NULL)
    {
  	  ReturnCode = Not_Enough_Memory;
  	  continue;
    }

    Result = VFSFileRead(ContextFileRef,
                         CONTEXT_SIZE_MEMMAP_ROM,
                         (void*)FileMemmapROM,
                         &numBytesRead);
#ifndef __RELEASE__
    if (Result != errNone)
      printf("VFSFileRead != errNone\n");
#endif /* __RELEASE__ */

    if ( (numBytesRead != CONTEXT_SIZE_MEMMAP_ROM) ||
         (Result != errNone) )
    {
      ReturnCode = Bad_Data;
      continue;
    }

    //
    // EXPANSION ROM
    //
    for (Loop=0; Loop < MEMMAP_ROM_COUNT; Loop++)
    {
  	  if (FileExpansionROM == FileMemmapROM[Loop])
  	  {
        RESTORECPC_MEMBANK_TRACE_SHOW_INT(200 + Loop);
  		  NativeCPC->pbExpansionROM = ((tUChar**)EndianSwap32(NativeCPC->memmap_ROM))[Loop];
  		  break;
  	  }
    }
  }
  while (0);

  if (FileMemmapROM != NULL)
  {
    MemPtrFree(FileMemmapROM);
  }

  return (ReturnCode);
}
/*----------------------------------------------------------------------------*/


tCPCSaveReturnCode WriteFile(const char* pathP,
                             const char* filenameP,
                             const tUChar* fileContentP,
                             UInt32 fileSize)
/***********************************************************************
 *
 *  WriteFile
 *
 ***********************************************************************/

#undef WRITEFILE_TRACE_ENABLED
//#define WRITEFILE_TRACE_ENABLED

#ifdef WRITEFILE_TRACE_ENABLED
#  define WRITEFILE_TRACE_SHOW_INT(value) TRACE_SHOW_INT("WriteFile", value)
#else
#  define WRITEFILE_TRACE_SHOW_INT(value)
#endif /* WRITEFILE_TRACE_ENABLED */
{
tCPCSaveReturnCode ReturnCode = Save_OK;
FileRef fileRef;
UInt32 numBytesWritten;
Err Result;

  WRITEFILE_TRACE_SHOW_INT(1);

  // Create result file
  Result = OpenFile(pathP,
                    filenameP,
                    &fileRef,
                    vfsModeReadWrite,
                    true);
  if (Result != errNone)
    return File_Not_Created;

  WRITEFILE_TRACE_SHOW_INT(2);

  // Write file content
  Result = VFSFileWrite(fileRef,
                        fileSize,
                        fileContentP,
                        &numBytesWritten);
#ifndef __RELEASE__
  if (Result != errNone)
    printf("VFSFileWrite != errNone\n");
#endif /* __RELEASE__ */
  if (numBytesWritten != fileSize)
  {
    ReturnCode = Write_Error;

    WRITEFILE_TRACE_SHOW_INT(3);
  }

  WRITEFILE_TRACE_SHOW_INT(4);

  VFSFileClose(fileRef);

  WRITEFILE_TRACE_SHOW_INT(5);

  return ReturnCode;
}
/*----------------------------------------------------------------------------*/


Err PrepareScreenshot(const tUChar* pScreen,
                      const tUChar* pCPCColors,
                      const tUChar* pPalmPalette,
                      tUChar** ppScreenshot,
                      UInt32* pSize)
/***********************************************************************
 *
 *  PrepareScreenshot
 *
 ***********************************************************************/
#undef PREPARESCREENSHOT_TRACE_ENABLED
//#define PREPARESCREENSHOT_TRACE_ENABLED

#ifdef PREPARESCREENSHOT_TRACE_ENABLED
#  define PREPARESCREENSHOT_TRACE_SHOW_INT(value) TRACE_SHOW_INT("PrepareScreenshot", value)
#else
#  define PREPARESCREENSHOT_TRACE_SHOW_INT(value)
#endif /* PREPARESCREENSHOT_TRACE_ENABLED */
{
Err Result = errNone;
tUChar* pScreenshot;
tUChar* pBuffer;
tUChar* pPalmtoCPCColours;
UInt32 Loop;
UInt32 lineIndex;
UInt32 rowIndex;

  *ppScreenshot = NULL;
  *pSize = 0;

  PREPARESCREENSHOT_TRACE_SHOW_INT(1);

  do
  {
    MemPtrNewLarge(SCREENSHOT_SIZE, (tVoid**)&pScreenshot);
    pPalmtoCPCColours = (tUChar*)MemPtrNew(256);
    if ( (pScreenshot == NULL) ||
         (pPalmtoCPCColours == NULL) )
    {
      FrmAlert(NotEnoughMemoryAlert);
      Result = memErrNotEnoughSpace;
      continue;
    }

    *ppScreenshot = pBuffer = pScreenshot;
    *pSize = SCREENSHOT_SIZE;

    PREPARESCREENSHOT_TRACE_SHOW_INT(2);

    //
    // Prepare file header
    //
    MemMove(pBuffer,
            (tUChar*)&BitmapFileHeader,
            sizeof(tBitmapFileHeader));
    pBuffer+=sizeof(tBitmapFileHeader);

    PREPARESCREENSHOT_TRACE_SHOW_INT(3);

    //
    // Prepare image header
    //
    MemMove(pBuffer,
            (tUChar*)&BitmapHeader,
            sizeof(tBitmapHeader));
    pBuffer+=sizeof(tBitmapHeader);

    PREPARESCREENSHOT_TRACE_SHOW_INT(4);

    //
    // Prepare palette
    //
    for (Loop=0; Loop < SCREENSHOT_NB_COLORS; Loop++, pCPCColors+=sizeof(colours_rgb_entry))
    {
    	*(pBuffer++) = *(pCPCColors+2); // Blue
    	*(pBuffer++) = *(pCPCColors+1); // Green
    	*(pBuffer++) = *pCPCColors;     // Red
    	*(pBuffer++) = 0;               // Reserved
    }

    PREPARESCREENSHOT_TRACE_SHOW_INT(5);

    //
    // Prepare image content
    //
    // Conversion from Palm Palette to CPC Palette
    for (Loop=0; Loop < SCREENSHOT_NB_COLORS; Loop++)
    {
      pPalmtoCPCColours[pPalmPalette[Loop]] = Loop;
    }

    PREPARESCREENSHOT_TRACE_SHOW_INT(6);

    // Transfer bitmap content from bottom left to top right using CPC Palette conversion
    for (lineIndex=SCREENSHOT_IMAGE_HEIGHT; lineIndex; lineIndex--)
    {
      const tUChar* lineP = pScreen + ((lineIndex + SCREENSHOT_IMAGE_HEIGHT_OFFSET) * SCREENSHOT_IMAGE_WIDTH);
      for (rowIndex=0; rowIndex < SCREENSHOT_IMAGE_WIDTH; rowIndex++)
      {
    	  *(pBuffer++) = pPalmtoCPCColours[*(lineP++)];
      }
    }
  }
  while (0);

  PREPARESCREENSHOT_TRACE_SHOW_INT(7);

  if (Result != errNone)
  {
  	if (pScreenshot)
  	{
  	  MemPtrFree(pScreenshot);
  	}
  }

  if (pPalmtoCPCColours)
  {
    MemPtrFree(pPalmtoCPCColours);
  }

  return Result;
}
/*----------------------------------------------------------------------------*/


Err ExtractExtensionFromFilename(char* filenameP,
                                 const char* extensionP)
/***********************************************************************
 *
 *  ExtractExtensionFromFilename
 *
 ***********************************************************************/
#undef EXTRACTEXTENSIONFROMFILENAME_TRACE_ENABLED
//#define EXTRACTEXTENSIONFROMFILENAME_TRACE_ENABLED

#ifdef EXTRACTEXTENSIONFROMFILENAME_TRACE_ENABLED
#  define EXTRACTEXTENSIONFROMFILENAME_TRACE_SHOW_INT(value) TRACE_SHOW_INT("ExtractExtensionFromFilename", value)
#else /* EXTRACTEXTENSIONFROMFILENAME_TRACE_ENABLED */
#  define EXTRACTEXTENSIONFROMFILENAME_TRACE_SHOW_INT(value)
#endif /* EXTRACTEXTENSIONFROMFILENAME_TRACE_ENABLED */
{
char* tempP;
char* extP;

#ifndef __RELEASE__
  if (filenameP == NULL)
    printf("filenameP == NULL\n");
  if (extensionP == NULL)
    printf("extensionP == NULL\n");
#endif /* __RELEASE__ */

  EXTRACTEXTENSIONFROMFILENAME_TRACE_SHOW_INT(1);

  tempP = (char*)MemPtrNew(StrLen(filenameP)+1);
  if (tempP == NULL)
  {
    FrmAlert(NotEnoughMemoryAlert);
    return memErrNotEnoughSpace;
  }

  EXTRACTEXTENSIONFROMFILENAME_TRACE_SHOW_INT(2);

  // Suppress extension to filename
  extP = StrStr(StrToLower(tempP,
                           filenameP),
                extensionP);
  if (extP != NULL)
  {
    *(filenameP + (extP - tempP)) = 0;
  }

  EXTRACTEXTENSIONFROMFILENAME_TRACE_SHOW_INT(3);

  MemPtrFree(tempP);

  EXTRACTEXTENSIONFROMFILENAME_TRACE_SHOW_INT(4);

  return errNone;
}
/*----------------------------------------------------------------------------*/


UInt16 GetSaveFilename(char* filenameP,
                       char* extensionP,
                       char* formTitle)
/***********************************************************************
 *
 *  GetSaveFilename
 *
 ***********************************************************************/
{

UInt16 filenameLength = 0;

#ifndef __RELEASE__
  if (filenameP == NULL)
    printf("filenameP == NULL\n");
  if (extensionP == NULL)
    printf("extensionP == NULL\n");
#endif /* __RELEASE__ */

  StrCopy (filenameP,"NoNameFile\n");

  if (StrLen(extensionP)) {
    StrCat(filenameP,
         extensionP);
  }
  else
  {
    StrCat(filenameP,
         ".xyz");
  }

  filenameLength = StrLen(filenameP);
  return filenameLength;
}
/*----------------------------------------------------------------------------*/

void SaveScreenshot(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  SaveScreenShot
 *
 ***********************************************************************/
#undef SAVESCREENSHOT_DEBUG_ENABLE
//#define SAVESCREENSHOT_DEBUG_ENABLE
#undef SAVESCREENSHOT_TRACE_ENABLED
//#define SAVESCREENSHOT_TRACE_ENABLED

#ifdef SAVESCREENSHOT_TRACE_ENABLED
#  define SAVESCREENSHOT_TRACE_SHOW_INT(value) TRACE_SHOW_INT("SaveScreenshot", value)
#else
#  define SAVESCREENSHOT_TRACE_SHOW_INT(value)
#endif /* SAVESCREENSHOT_TRACE_ENABLED */


#undef SAVESCREENSHOT_FILE_NOT_CREATED
//#define SAVESCREENSHOT_FILE_NOT_CREATED
#undef SAVESCREENSHOT_WRITE_ERROR
//#define SAVESCREENSHOT_WRITE_ERROR
#undef SAVESCREENSHOT_SAVE_SUCCESS
//#define SAVESCREENSHOT_SAVE_SUCCESS
{
char* filenameP = NULL;
char* stringP = NULL;
tUChar* fileContentP = NULL;
UInt32 fileSize;
tCPCSaveReturnCode ReturnCode;

  SAVESCREENSHOT_TRACE_SHOW_INT(1);

  do
  {
 	  filenameP = (char*)MemPtrNew(MAX_FILE_NAME * sizeof(Char));
 	  stringP = (char*)MemPtrNew(MAX_FILE_NAME * sizeof(Char));
    if ( (filenameP == NULL) || (stringP == NULL) )
    {
      FrmAlert(NotEnoughMemoryAlert);
      continue;
    }

    SAVESCREENSHOT_TRACE_SHOW_INT(2);

  	if (IsDriveFilenameExist(NativeCPC->DriveA) == cTrue)
  	{
  		//
  		// Build filename using portion of filename and current time
  		//
  		GetDriveFilename(NativeCPC->DriveA,
  		                 filenameP);

      ExtractExtensionFromFilename(filenameP,
                                   DISK_EXTENSION);

      filenameP[12]=chrNull; // Keep the 12 first characters of filename

      // Add Number of seconds since the beginning...?
  	}
  	else
  	{
      // Force filename generation
      filenameP[0] = 0;
    }

    SAVESCREENSHOT_TRACE_SHOW_INT(3);

#ifdef SAVESCREENSHOT_DEBUG_ENABLE
    TRACE_SHOW_TEXT("filenameP=", filenameP);
#endif

    // Get Screenshot filename
    if (GetSaveFilename(filenameP,
                        SCREENSHOT_EXTENSION,
                        SAVE_SCREENSHOT_TITLE) == 0)
    {
    	continue;
    }

    SAVESCREENSHOT_TRACE_SHOW_INT(4);

#ifdef SAVESCREENSHOT_DEBUG_ENABLE
    TRACE_SHOW_TEXT("filenameP=", filenameP);
#endif

    SAVESCREENSHOT_TRACE_SHOW_INT(5);

    // Prepare file content
    if (PrepareScreenshot((const tUChar*)EndianSwap32(NativeCPC->BmpOffScreenBits),
      	                  (const tUChar*)EndianSwap32(NativeCPC->active_colours),
      	                  (const tUChar*)NativeCPC->colours,
      	                  &fileContentP,
      	                  &fileSize) != errNone)
    {
    	continue;
    }

    SAVESCREENSHOT_TRACE_SHOW_INT(6);

    // Create file
    ReturnCode = WriteFile(DEFAULT_SCREENSHOT_PATH,
      		                 filenameP,
      		                 fileContentP,
      		                 fileSize);

#ifdef SAVESCREENSHOT_FILE_NOT_CREATED
    ReturnCode = File_Not_Created;
#endif /* SAVESCREENSHOT_FILE_NOT_CREATED */
#ifdef SAVESCREENSHOT_WRITE_ERROR
    ReturnCode = Write_Error;
#endif /* SAVESCREENSHOT_WRITE_ERROR */
#ifdef SAVESCREENSHOT_SAVE_SUCCESS
    ReturnCode = Save_OK;
#endif /* SAVESCREENSHOT_SAVE_SUCCESS */
    if (ReturnCode == File_Not_Created)
    {
      FrmAlert(ScreenshotCreationAlert);
    }
    else if (ReturnCode == Write_Error)
    {
      FrmAlert(BadCPCSaveAlert);
    }

    SAVESCREENSHOT_TRACE_SHOW_INT(7);

    MemPtrFree(fileContentP);
  }
  while (0);

  if (filenameP) MemPtrFree(filenameP);
  if (stringP) MemPtrFree(stringP);

  SAVESCREENSHOT_TRACE_SHOW_INT(8);
}
/*----------------------------------------------------------------------------*/


void SaveSession(tNativeCPC* NativeCPC,
                 tUChar* contextP,
                 char* sessionFilenameP)
/***********************************************************************
 *
 *  SaveSession
 *
 ***********************************************************************/
#undef SAVESESSION_FILE_NOT_CREATED
//#define SAVESESSION_FILE_NOT_CREATED
#undef SAVESESSION_WRITE_ERROR
//#define SAVESESSION_WRITE_ERROR
#undef SAVESESSION_SAVE_SUCCESS
//#define SAVESESSION_SAVE_SUCCESS
{
char* filenameP;
tCPCSaveReturnCode ReturnCode;

 	filenameP = (char*)MemPtrNew(MAX_FILE_NAME);
  if (filenameP == NULL)
  {
    FrmAlert(NotEnoughMemoryAlert);
    return;
  }

  // Get last session filename
  StrCopy(filenameP,
          sessionFilenameP);

  do
  {
  	if (IsDriveFilenameExist(NativeCPC->DriveA) == cTrue)
  	{
  		// Get Disk A filename as default session filename
  		GetDriveFilename(NativeCPC->DriveA,
  		                 filenameP);

      ExtractExtensionFromFilename(filenameP,
                                   DISK_EXTENSION);
  	}
  	else
  	{
      // Force filename generation
      filenameP[0] = 0;
    }

    // Get Screenshot filename
    if (GetSaveFilename(filenameP,
                        CONTEXT_EXTENSION,
                        SAVE_SESSION_TITLE) == 0)
    {
    	continue;
    }

    // Save session filename
    StrCopy(sessionFilenameP,
            filenameP);

    // Save session
    ReturnCode = SaveCPC(DEFAULT_CAPRICE_PATH,
                         sessionFilenameP,
                         contextP);

#ifdef SAVESESSION_FILE_NOT_CREATED
    ReturnCode = File_Not_Created;
#endif /* SAVESESSION_FILE_NOT_CREATED */
#ifdef SAVESESSION_WRITE_ERROR
    ReturnCode = Write_Error;
#endif /* SAVESESSION_WRITE_ERROR */
#ifdef SAVESESSION_SAVE_SUCCESS
    ReturnCode = Save_OK;
#endif /* SAVESESSION_SAVE_SUCCESS */

    if (ReturnCode == File_Not_Created)
    {
      FrmAlert(CPCSaveCreationAlert);
    }
    else if (ReturnCode == Write_Error)
    {
      FrmAlert(BadCPCSaveAlert);
    }
  }
  while (0);

  MemPtrFree(filenameP);
}
/*----------------------------------------------------------------------------*/


UInt32 DiskAutostart(tNativeCPC* NativeCPC,
                     tDrive* nativedriveP,
                     tUChar* commandP)
/***********************************************************************
 *
 *  DiskAutostart
 *
 ***********************************************************************/
{
tDiskOperation* DiskOperationP;
tAutoStartReturnCode returnCode;

  DiskOperationP = (tDiskOperation*)(EndianSwap32(NativeCPC->contextP) + CONTEXT_OFFSET_DISKOPERATION);

  DiskOperationP->NativeCPC = NativeCPC;
  DiskOperationP->Drive = (tDrive*)EndianSwap32(nativedriveP);
  DiskOperationP->Param = (tVoid*)commandP;

  returnCode = Engine_DiskAutoStart(DiskOperationP);

  return (UInt32)returnCode;
}
/*----------------------------------------------------------------------------*/

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

#ifndef FILES_H
#define FILES_H

#include <stdio.h>

#include "types.h"
#include "sections.h"
#include "Native_CPC.h"
#include "vfsfile.h"
#include "Keyboard.h"

#define CPC_FILE_LENGTH           13
#define MAX_FILE_NAME              256
#define CHEAT_FILE_MAGIC          (0x2E636D66)
#define PARENT_PATH               "/.."

#ifdef SIM
#define ROOT_PATH                 "/"
#define DEFAULT_DISK_PATH         "../games"
#define DEFAULT_CAPRICE_PATH      "."
#define DEFAULT_SCREENSHOT_PATH   "."
#define DEFAULT_CHEAT_PATH        "../cheat"
#else
#define ROOT_PATH                 "/sd"
#define DEFAULT_DISK_PATH         "/sd/cpc/dsk"
#define DEFAULT_CAPRICE_PATH      "/sd/cpc/cps"
#define DEFAULT_SCREENSHOT_PATH   "/sd/cpc/scr"
#define DEFAULT_CHEAT_PATH        "/sd/cpc/cmf"
#endif

#define NODISK_FILENAME           ""
#define CONTEXT_EXTENSION         ".cpc"
#define DISK_EXTENSION_SHORT      "dsk"
#define DISK_EXTENSION            "." DISK_EXTENSION_SHORT
#define SCREENSHOT_EXTENSION      ".bmp"
#define CHEAT_EXTENSION           ".cmf"
#define NEW_DISK_FILENAME         "New Disk" DISK_EXTENSION
#define LASTSESSION_FILENAME      "LastSession" CONTEXT_EXTENSION

#define DISK_EXTENSION_DESCRIPTION     "Amstrad CPC Disk Image"

#define SAVE_SESSION_TITLE        "Session saving"
#define SAVE_SCREENSHOT_TITLE     "Screenshot saving"

#define	chrNull							0x0000

#define errNone                       0x0000  // No error
#define memErrorClass                 0x0100  // Memory Manager
// copied from MemoryMgr.h
#define memErrNotEnoughSpace          (memErrorClass | 2)

// missing items from PalmOS
#define EndianSwap32(p) p
#define EndianSwap16(p) p

typedef enum
{
  Restore_OK,
  File_Not_Found,
  Wrong_Version,
  Bad_Data,
  Not_Enough_Memory,
  Wrong_CPC_Model,
} tCPCRestoreReturnCode;

typedef enum
{
  Save_OK,
  File_Not_Created,
  Write_Error
} tCPCSaveReturnCode;

typedef enum
{
  No_Option = 0x0,
  Show_Folder = 0x01
} tCPCFolderContentOptions;


typedef enum
{
  noDrive,
  DriveA,
  DriveB
} tDriveEnum;


typedef struct
{
  UInt32 srcContextBase;
  UInt32 dstContextBase;
  UInt32 contextSize;
  void** srcAreaP;
  void** dstAreaP;
  UInt32 areaSize;
  Boolean native;
} tReindexOperationParameters;

// Virtual File System Support


#ifndef _USE_MACROS

extern tBool IsDriveFilenameExist(tDrive* nativeDriveP); //FC!! Section
extern void SetDriveFilename(tDrive* nativeDriveP,
                             const char* filenameP); //FC!! Section
extern void GetDriveFilename(tDrive* nativeDriveP,
                             char* filenameP); //FC!! Section

#else /* _USE_MACROS */

#define IsDriveFilenameExist(nativeDriveP) \
  (((tDrive*)EndianSwap32((UInt32)nativeDriveP))->filename[0] == 0 ? cFalse : cTrue)

#define SetDriveFilename(nativeDriveP, filenameP) \
  StrCopy(((tDrive*)EndianSwap32((UInt32)nativeDriveP))->filename, \
          (const char*)filenameP);

#define GetDriveFilename(nativeDriveP, filenameP) \
  StrCopy((char*)filenameP, \
          ((tDrive*)EndianSwap32((UInt32)nativeDriveP))->filename);

#endif /* _USE_MACROS */


extern Err CreateDirectories(const char* palmPathP,
                             const char* capricePathP,
                             const char* diskPathP,
                             const char* screenshotPathP) SECTION_FILES;

extern Err EjectDisk(tDrive* nativedriveP,
                     tNativeCPC* NativeCPC) SECTION_FILES;
extern Boolean SaveAndEjectBoth(tNativeCPC* NativeCPC) SECTION_FILES;
extern void SwapDrive(tNativeCPC* NativeCPC) SECTION_FILES;

extern Err LoadDiskImage(const char* pathP,
                         tDrive* nativedriveP,
                         tNativeCPC* NativeCPC) SECTION_FILES;
extern Err LoadDiskImageFromMemory(tDrive* nativedriveP,
                                   tNativeCPC* NativeCPC,
                                   tULong DiskSize,
                                   tUChar* DiskContentP) SECTION_FILES;

extern UInt16 ReadDiskCatalogue(tDriveEnum drive, tUChar User, char** CatP, tNativeCPC* NativeCPC) SECTION_FILES;

extern Err SaveDiskImage(const char* pathP,
                         tDrive* nativedriveP,
                         tNativeCPC* NativeCPC) SECTION_FILES;
extern Err SaveAndEjectDiskImage(const char* pathP,
                                 tDrive* nativedriveP,
                                 tNativeCPC* NativeCPC) SECTION_FILES;

extern Err CreateDisk(tDrive* nativedriveP,
                      tUChar FormatType,
                      tNativeCPC* NativeCPC) SECTION_FILES;

extern tBool IsFileExist(const char* pathP,
                         const char* filenameP) SECTION_FILES;
extern tBool IsDirectoryExist(const char* pathP) SECTION_FILES;

extern Err DeleteFile(const char* pathP,
                      const char* filenameP) SECTION_FILES;

extern UInt16 RetreiveFolderContent(const char* pathP,
                                    char*** filelistPP,
                                    const char* extensionP,
                                    tCPCFolderContentOptions Options) SECTION_FILES;

extern tCPCSaveReturnCode SaveCPC(const char* pathP,
                                  const char* filenameP,
                                  tUChar* contextP) SECTION_FILES;
extern tCPCRestoreReturnCode RestoreCPC(const char* pathP,
                                        const char* filenameP,
                                        tEmulatorKeysStatus* KeyStatusP,
                                        tPreferences* requestedPrefP,
                                        tUChar* contextP) SECTION_FILES;

extern tCPCSaveReturnCode WriteFile(const char* pathP,
                                    const char* filenameP,
                                    const tUChar* fileContentP,
                                    UInt32 fileSize) SECTION_FILES;

extern Err PrepareScreenshot(const tUChar* pScreen,
                             const tUChar* pCPCColors,
                             const tUChar* pPalmPalette,
                             tUChar** ppScreenshot,
                             UInt32* pSize) SECTION_FILES;

extern Err ExtractExtensionFromFilename(char* filenameP,
                                        const char* extensionP) SECTION_FILES;


extern UInt16 GetSaveFilename(char* filenameP,
                              char* extensionP,
                              char* formTitle) SECTION_FILES;

extern void SaveScreenshot(tNativeCPC* NativeCPC) SECTION_FILES;

extern void SaveSession(tNativeCPC* NativeCPC,
                        tUChar* contextP,
                        char* sessionFilenameP) SECTION_FILES;

extern UInt32 DiskAutostart(tNativeCPC* NativeCPC,
                            tDrive* nativedriveP,
                            tUChar* commandP) SECTION_FILES;

#endif /* ! FILES_H */

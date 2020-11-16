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
#include <PceNativeCall.h>
#include <ByteOrderUtils.h>
#endif

#include <stdlib.h>

#include "types.h"

// Copied from Window.h
typedef tUChar IndexedColorType;      // 1, 2, 4, or 8 bit index

#include "Native_CPC.h"


/*
** Make sure we can call this stuff from C++.
*/
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __PALMOS__
tULong PNOMain(const tVoid*,
               tVoid*,
               Call68KFuncType*);
#endif

//
// Routines
//
static tVoid MemMove(tUChar* destP,
                     tUChar* sourceP,
                     tULong numBytes);
static tVoid MemSet(tUChar* destP,
                    tULong numBytes,
                    tUChar value);
//
// Palm OS routines
//
#ifdef __PALMOS__
static tVoid* MemPtrNewLarge(tULong size,
                             tULong routineP,
                             const tVoid* emulStateP,
                             Call68KFuncType* call68KFuncP);
#else
#define NewMemFct(p) malloc(p)
#endif
//
// TestU
//
#ifdef _TESTU
static tUShort PerformTestU(tNativeCPC* NativeCPC);
#endif /* _TESTU */


// copied from ErrorBase.h
#define errNone                       0x0000  // No error
#define memErrorClass                 0x0100  // Memory Manager

// copied from VFSMgr.h
#define vfsErrorClass                 0x2A00    // Post-3.5 this is defined in ErrorBase.h
#define vfsErrBadData                 (vfsErrorClass | 12)  // operation could not be completed because of invalid data (i.e., import DB from .PRC file)

// copied from MemoryMgr.h
#define memErrNotEnoughSpace          (memErrorClass | 2)


/* Table available on resource TABLE_ID_DISK_FORMAT
const tDiskFormat disk_format[MAX_DISK_FORMAT] =
{
  { "178K Data Format", 40, 1, 9, 2, 0x52, 0xe5, {{ 0xc1, 0xc6, 0xc2, 0xc7, 0xc3, 0xc8, 0xc4, 0xc9, 0xc5 }} },
  { "169K Vendor Format", 40, 1, 9, 2, 0x52, 0xe5, {{ 0x41, 0x46, 0x42, 0x47, 0x43, 0x48, 0x44, 0x49, 0x45 }} }
};*/


/***********************************************************************
 *
 *  Entry Points
 *
 ***********************************************************************/

#ifdef _TRACE
#define SHOWTRACE(p) \
{ \
  tUChar argsTrace[4]; \
  *((tULong*)(&(argsTrace[0]))) = EndianSwap32(p); \
  NativeCPC->call68KFuncP(NativeCPC->emulStateP, \
                          NativeCPC->TraceAlertPtr, \
                          &argsTrace, \
                          (sizeof(argsTrace)/sizeof(argsTrace[0]))); \
}
#endif /* _TRACE */

#ifdef __PALMOS__
tULong PNOMain(const tVoid* emulStateP,
               tVoid* userData68KP,
               Call68KFuncType* call68KFuncP)
#else
tULong Engine_DiskFormat(tDiskOperation* DiskOperation)
#endif
/***********************************************************************
 *
 *  PNOMain
 *
 ***********************************************************************/
{
#ifdef __PALMOS__
tDiskOperation* DiskOperation = (tDiskOperation*)userData68KP;
#endif
tNativeCPC* NativeCPC;
tDiskFormat* disk_format;
tDrive* Drive;
tUChar* pbDiskPtr;
tUChar* pbDataPtr;
tUChar* pbTempPtr;
tULong Result = errNone;
tULong track;
tULong side;
tULong sector;
tULong dwTrackSize;
tULong dwSectorSize;
tULong dwSectors;
tULong dwDiskSize;
tUChar CHRN[4];

#ifdef __PALMOS__
  // Palm OS 68K interface
  Drive = (tDrive*)EndianSwap32(DiskOperation->Drive);
  NativeCPC = (tNativeCPC*)EndianSwap32(DiskOperation->NativeCPC);
  disk_format = (tDiskFormat*)EndianSwap32(DiskOperation->DiskFormatTableP);
#else
Drive = DiskOperation->Drive;
#ifdef _TESTU
NativeCPC = DiskOperation->NativeCPC;
#endif /* _TESTU */
disk_format = DiskOperation->DiskFormatTableP;
#endif

#ifdef _TESTU
  Result = PerformTestU(NativeCPC);
#endif /* _TESTU */

  Drive->tracks = disk_format[DiskOperation->FormatType].tracks;
  if (Drive->tracks > DSK_TRACKMAX) // compare against upper limit
  {
    Drive->tracks = DSK_TRACKMAX; // limit to maximum
  }

  Drive->sides = disk_format[DiskOperation->FormatType].sides;
  if (Drive->sides > DSK_SIDEMAX) // abort if more than maximum
  {
    return vfsErrBadData;
  }

  dwSectorSize = 0x80 << disk_format[DiskOperation->FormatType].sector_size; // determine sector size in bytes
  dwSectors = disk_format[DiskOperation->FormatType].sectors;
  if (dwSectors > DSK_SECTORMAX) // abort if sector count greater than maximum
  {
    return vfsErrBadData;
  }
  dwTrackSize = dwSectorSize * dwSectors; // determine track size in bytes, minus track header

  //
  // Allocate memory for entire disk
  //
  dwDiskSize = 0x100 /* Track Header */ + dwTrackSize /* Track Data */;
  dwDiskSize *= Drive->tracks; // Multiply by the number of tracks
  dwDiskSize *= Drive->sides; // Multiply by the number of sides
  dwDiskSize += 0x100; // Add disk header
#ifdef __PALMOS__
  Drive->dataP = (tUChar*)MemPtrNewLarge(dwDiskSize,
                                         NativeCPC->MemPtrNewPtr,
                                         emulStateP,
                                         call68KFuncP);
#else
  Drive->dataP = (tUChar*)NewMemFct(dwDiskSize);
#endif
  if (Drive->dataP == cNull)
  {
    return memErrNotEnoughSpace;
  }
  Drive->data_size = dwDiskSize;

  // Begin of first track
  pbDiskPtr = Drive->dataP + 0x100;

  // zero base number of sides
  Drive->sides--;

  for (track=0; track < Drive->tracks; track++) // loop for all tracks
  {
    for (side=0; side <= Drive->sides; side++) // loop for all sides
    {
      Drive->track[track][side].sectors = dwSectors; // store sector count
      Drive->track[track][side].size = dwTrackSize; // store track size
      Drive->track[track][side].data = pbDiskPtr + 0x100; // Begin of first sector

      pbDataPtr = Drive->track[track][side].data; // pointer to start of memory buffer
      pbTempPtr = pbDataPtr; // keep a pointer to the beginning of the buffer for the current track
      CHRN[0] = (tUChar)track;
      CHRN[1] = (tUChar)side;
      CHRN[3] = (tUChar)disk_format[DiskOperation->FormatType].sector_size;
      for (sector = 0; sector < dwSectors; sector++) // loop for all sectors
      {
        CHRN[2] = disk_format[DiskOperation->FormatType].sector_ids[side][sector];

        MemMove(Drive->track[track][side].sector[sector].CHRN,
                CHRN,
                4); // copy CHRN

        Drive->track[track][side].sector[sector].size = dwSectorSize;
        Drive->track[track][side].sector[sector].data = pbDataPtr; // store pointer to sector data

        pbDataPtr += dwSectorSize;
      }

      MemSet(pbTempPtr,
             dwTrackSize,
             disk_format[DiskOperation->FormatType].filler_byte);

      // Begin of next track
      pbDiskPtr = Drive->track[track][side].data + dwTrackSize;
    }
  }

  Drive->altered = 1; // flag disk as having been modified

  return Result;
}
/*----------------------------------------------------------------------------*/



//==============================================================================
//
// Routines
//
//==============================================================================

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


static tVoid MemSet(tUChar* destP,
                    tULong numBytes,
                    tUChar value)
/***********************************************************************
 *
 *  MemSet
 *
 ***********************************************************************/
{
  while (numBytes--)
  {
    *(destP++) = value;
  }
}
/*----------------------------------------------------------------------------*/


//==============================================================================
//
// Palm OS routines
//
//==============================================================================

#ifdef __PALMOS__
static tVoid* MemPtrNewLarge(tULong size,
                             tULong routineP,
                             const tVoid* emulStateP,
                             Call68KFuncType* call68KFuncP)
/***********************************************************************
 *
 *  MemPtrNewLarge
 *
 ***********************************************************************/
{
tUChar args[4];

  *((tULong*)(&(args[0]))) = (tULong)EndianSwap32(size);

  return (tVoid*)call68KFuncP(emulStateP,
                              routineP,
                              &args,
                              (sizeof(args)/sizeof(args[0])) | kPceNativeWantA0);
}
/*----------------------------------------------------------------------------*/
#endif /* __PALMOS__ */

//==============================================================================
//
// Unitary Tests
//
//==============================================================================
#ifdef _TESTU
// Prototypes of TestU fonctions
static tUShort TestU_MemSet_1(tNativeCPC* NativeCPC,
                              tUChar NoTest);
static tUShort TestU_MemMove_1(tNativeCPC* NativeCPC,
                               tUChar NoTest);


static tUShort PerformTestU(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  PerformTestU
 *
 ***********************************************************************/
{
tUShort Result = errNone;
tUChar NoTest = 1;

  /* 1 */ if (Result == errNone) Result = TestU_MemSet_1(NativeCPC,
                                                         NoTest++);
  /* 2 */ if (Result == errNone) Result = TestU_MemMove_1(NativeCPC,
                                                          NoTest++);

  return (Result);
}
/*----------------------------------------------------------------------------*/


static tUShort TestU_MemSet_1(tNativeCPC* NativeCPC,
                              tUChar NoTest)
/***********************************************************************
 *
 *  TestU_MemSet_1
 *
 ***********************************************************************/
{
tUChar ArrayA[100];
tUChar Loop;
tUChar Result = errNone;

  // Prepare conditions
  for (Loop=0; Loop<100; Loop++)
    ArrayA[Loop] = 0x55;
  // Perform operation
  MemSet(ArrayA, 95, 0xAA);
  // Check Result
  for (Loop=0; Loop<95; Loop++)
  {
    if (ArrayA[Loop] != 0xAA)
      Result=testUErrorClass+NoTest;
  }
  for (Loop=95; Loop<100; Loop++)
  {
    if (ArrayA[Loop] != 0x55)
      Result=testUErrorClass+NoTest;
  }

  return (Result);
}
/*----------------------------------------------------------------------------*/

static tUShort TestU_MemMove_1(tNativeCPC* NativeCPC,
                               tUChar NoTest)
/***********************************************************************
 *
 *  TestU_MemMove_1
 *
 ***********************************************************************/
{
tUChar SrcArrayA[100];
tUChar DstArrayA[100];
tUChar Loop;
tUChar Result = errNone;

  // Prepare conditions
  for (Loop=0; Loop<100; Loop++)
  {
    SrcArrayA[Loop] = 0x55;
    DstArrayA[Loop] = 0xAA;
  }
  // Perform operation
  MemMove(DstArrayA, SrcArrayA, 95);
  // Check Result
  for (Loop=0; Loop<100; Loop++)
  {
    if (SrcArrayA[Loop] != 0x55)
      Result=testUErrorClass+NoTest;
  }
  for (Loop=0; Loop<95; Loop++)
  {
    if (DstArrayA[Loop] != 0x55)
      Result=testUErrorClass+NoTest;
  }
  for (Loop=95; Loop<100; Loop++)
  {
    if (DstArrayA[Loop] != 0xAA)
      Result=testUErrorClass+NoTest;
  }

  return (Result);
}
/*----------------------------------------------------------------------------*/

#endif /* _TESTU */


#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

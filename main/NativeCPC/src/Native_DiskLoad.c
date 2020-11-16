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
static tUShort MemCmp(tUChar* s1,
                      tUChar* s2,
                      tULong numBytes);
static tVoid MemMove(tUChar* destP,
                     tUChar* sourceP,
                     tULong numBytes);
static tVoid MemSet(tUChar* destP,
                    tULong numBytes,
                    tUChar value);
//
// TestU
//
#ifdef _TESTU
static tUShort PerformTestU(tNativeCPC* NativeCPC);
#endif /* _TESTU */


// copied from CoreTraps.h
#define sysTrapMemPtrNew              0xA013

// copied from ErrorBase.h
#define errNone                       0x0000  // No error
#define memErrorClass                 0x0100  // Memory Manager

// copied from VFSMgr.h
#define vfsErrorClass                 0x2A00    // Post-3.5 this is defined in ErrorBase.h
#define vfsErrBadData                 (vfsErrorClass | 12)  // operation could not be completed because of invalid data (i.e., import DB from .PRC file)
#define vfsErrDSKNoData               (vfsErrorClass + 2)

// copied from MemoryMgr.h
#define memErrNotEnoughSpace          (memErrorClass | 2)


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
tULong Engine_DiskLoad(tDiskOperation* DiskOperation)
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
tMemPtrNewFct NewMemFct = (tMemPtrNewFct)DiskOperation->NativeCPC->MemPtrNewPtr;
#if defined(_TRACE) || defined(_TESTU)
tNativeCPC* NativeCPC;
#endif /* _TRACE || _TESTU */
tDrive* Drive;
tUChar* pbPtr;
tUChar* pbDataPtr;
tUChar* pbTrackSizeTable;
tULong Result = errNone;
tULong track;
tULong side;
tULong sector;
tULong dwTrackSize;
tULong dwSectorSize;
tULong dwSectors;

#ifndef _TRACE
#ifdef __PALMOS__
  NOT_USED(emulStateP);
  NOT_USED(call68KFuncP);
#endif
#endif /* !_TRACE */

#ifdef __PALMOS__
  // Palm OS 68K interface
  Drive = (tDrive*)EndianSwap32(DiskOperation->Drive);
#if defined(_TRACE) || defined(_TESTU)
  NativeCPC = (tNativeCPC*)EndianSwap32(DiskOperation->NativeCPC);
#endif /* _TRACE || _TESTU */
  Drive->dataP = (tUChar*)EndianSwap32(DiskOperation->DiskContentP);
  Drive->data_size = (tULong)EndianSwap32(DiskOperation->disk_size);
#else
  Drive = DiskOperation->Drive;
#if defined(_TRACE) || defined(_TESTU)
  NativeCPC = DiskOperation->NativeCPC;
#endif /* _TRACE || _TESTU */
  Drive->dataP = DiskOperation->DiskContentP;
  Drive->data_size = DiskOperation->disk_size;
#endif
#ifdef _TESTU
  Result = PerformTestU(NativeCPC);
#endif /* _TESTU */

  if ( (Drive->dataP == cNull) || (Drive->data_size == 0) )
  {
  return vfsErrDSKNoData;
  }
  pbPtr = Drive->dataP;

  if (MemCmp(pbPtr, (tUChar*)("MV - CPC"), 8) == 0) // normal DSK image?
  {
    Drive->tracks = *(pbPtr + 0x30); // number of tracks
    if (Drive->tracks > DSK_TRACKMAX) // limit to maximum possible
    {
      Drive->tracks = DSK_TRACKMAX;
    }

    Drive->sides = *(pbPtr + 0x31); // number of sides
    if (Drive->sides > DSK_SIDEMAX) // abort if more than maximum
    {
      return vfsErrBadData;
    }
    Drive->sides--; // zero base number of sides

    dwTrackSize = *(pbPtr + 0x32) + (*(pbPtr + 0x33) << 8) - 0x100; // determine track size in bytes, minus track header

    // Begin of first track
    pbPtr += 0x100;

    for (track = 0; track < Drive->tracks; track++) // loop for all tracks
    {
      for (side = 0; side <= Drive->sides; side++) // loop for all sides
      {
        if (MemCmp(pbPtr, (tUChar*)("Track-Info"), 10) != 0) // abort if ID does not match
          return vfsErrBadData;

        dwSectorSize = 0x80 << *(pbPtr + 0x14); // determine sector size in bytes
        dwSectors = *(pbPtr + 0x15); // grab number of sectors
        if (dwSectors > DSK_SECTORMAX) // abort if sector count greater than maximum
        {
          return vfsErrBadData;
        }
        Drive->track[track][side].sectors = dwSectors; // store sector count
        Drive->track[track][side].size = dwTrackSize; // store track size
        Drive->track[track][side].data = pbPtr + 0x100;

        pbDataPtr = Drive->track[track][side].data; // pointer to start of memory buffer

        for (sector = 0; sector < dwSectors; sector++) // loop for all sectors
        {
          MemMove(Drive->track[track][side].sector[sector].CHRN, (pbPtr + 0x18), 4); // copy CHRN
          MemMove(Drive->track[track][side].sector[sector].flags, (pbPtr + 0x1c), 2); // copy ST1 & ST2
          Drive->track[track][side].sector[sector].size = dwSectorSize;
          Drive->track[track][side].sector[sector].data = pbDataPtr; // store pointer to sector data
          pbDataPtr += dwSectorSize;
          pbPtr += 8;
        }

        // Begin of next track
        pbPtr = Drive->track[track][side].data + dwTrackSize;
      }
    }
    Drive->altered = 0; // disk is as yet unmodified
  }
  else if (MemCmp(pbPtr, (tUChar*)("EXTENDED"), 8) == 0)
  {
    Drive->tracks = *(pbPtr + 0x30); // number of tracks
    if (Drive->tracks > DSK_TRACKMAX) // limit to maximum possible
    {
      Drive->tracks = DSK_TRACKMAX;
    }

    Drive->random_DEs = *(pbPtr + 0x31) & 0x80; // simulate random Data Errors?
    Drive->sides = *(pbPtr + 0x31) & 3; // number of sides
    if (Drive->sides > DSK_SIDEMAX) // abort if more than maximum
    {
      return vfsErrBadData;
    }

    pbTrackSizeTable = pbPtr + 0x34; // pointer to track size table in DSK header
    Drive->sides--; // zero base number of sides

    // Begin of first track
    pbPtr += 0x100;

    for (track = 0; track < Drive->tracks; track++) // loop for all tracks
    {
      for (side = 0; side <= Drive->sides; side++) // loop for all sides
      {
        dwTrackSize = (*pbTrackSizeTable++ << 8); // track size in bytes
        if (dwTrackSize != 0) // only process if track contains data
        {
          dwTrackSize -= 0x100; // compensate for track header

          if (MemCmp(pbPtr, (tUChar*)("Track-Info"), 10) != 0) // abort if ID does not match
          {
            return vfsErrBadData;
          }

          dwSectors = *(pbPtr + 0x15); // number of sectors for this track
          if (dwSectors > DSK_SECTORMAX) // abort if sector count greater than maximum
          {
            return vfsErrBadData;
          }

          Drive->track[track][side].sectors = dwSectors; // store sector count
          Drive->track[track][side].size = dwTrackSize; // store track size
          Drive->track[track][side].data = pbPtr + 0x100;

          pbDataPtr = Drive->track[track][side].data; // pointer to start of memory buffer
          for (sector = 0; sector < dwSectors; sector++) // loop for all sectors
          {
            MemMove(Drive->track[track][side].sector[sector].CHRN,
                    (pbPtr + 0x18),
                    4); // copy CHRN
            MemMove(Drive->track[track][side].sector[sector].flags,
                    (pbPtr + 0x1c),
                    2); // copy ST1 & ST2
            dwSectorSize = *(pbPtr + 0x1e) + (*(pbPtr + 0x1f) << 8); // sector size in bytes
            Drive->track[track][side].sector[sector].size = dwSectorSize;
            Drive->track[track][side].sector[sector].data = pbDataPtr; // store pointer to sector data
            pbDataPtr += dwSectorSize;
            pbPtr += 8;
          }

          // Begin of next track
          pbPtr = Drive->track[track][side].data + dwTrackSize;
        }
        else
        {
          MemSet((tUChar*)&Drive->track[track][side],
                 sizeof(tTrack),
                 0); // track not formatted
        }
      }
    }
    Drive->altered = 0; // disk is as yet unmodified
  }
  else
  {
    Result = vfsErrBadData;
  }

  return Result;
}
/*----------------------------------------------------------------------------*/



//==============================================================================
//
// Routines
//
//==============================================================================

static tUShort MemCmp(tUChar* s1,
                      tUChar* s2,
                      tULong numBytes)
/***********************************************************************
 *
 *  MemCmp
 *
 ***********************************************************************/
{
  while (numBytes--)
  {
    if (*(s1++) != *(s2++))
      return 1;
  }

  return 0;
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
// Unitary Tests
//
//==============================================================================
#ifdef _TESTU
// Prototypes of TestU fonctions
static tUShort TestU_MemSet_1(tNativeCPC* NativeCPC,
                              tUChar NoTest);
static tUShort TestU_MemMove_1(tNativeCPC* NativeCPC,
                               tUChar NoTest);
static tUShort TestU_MemCmp_1(tNativeCPC* NativeCPC,
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
  /* 3 */ if (Result == errNone) Result = TestU_MemCmp_1(NativeCPC,
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

static tUShort TestU_MemCmp_1(tNativeCPC* NativeCPC,
                              tUChar NoTest)
/***********************************************************************
 *
 *  TestU_MemCmp_1
 *
 ***********************************************************************/
{
tUChar S1ArrayA[100];
tUChar S2ArrayA[100];
tUChar Loop;
tUChar Result = errNone;

  // Prepare conditions
  for (Loop=0; Loop<50; Loop++)
  {
    S1ArrayA[Loop] = Loop;
    S2ArrayA[Loop] = Loop;
  }
  for (Loop=50; Loop<100; Loop++)
  {
    S1ArrayA[Loop] = 1;
    S2ArrayA[Loop] = 2;
  }
  // Perform operation
  // and Check Result
  if (MemCmp(S1ArrayA, S2ArrayA, 50) != 0)
      Result=testUErrorClass+NoTest;

  if (MemCmp(S1ArrayA, S2ArrayA, 51) != 1)
      Result=testUErrorClass+NoTest;

  return (Result);
}
/*----------------------------------------------------------------------------*/

#endif /* _TESTU */


#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

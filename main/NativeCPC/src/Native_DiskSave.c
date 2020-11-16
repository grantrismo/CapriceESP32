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
static tVoid MemMove(tUChar* destP,
                     tUChar* sourceP,
                     tULong numBytes);
static tVoid MemSet(tUChar* destP,
                    tULong numBytes,
                    tUChar value);
static tVoid StrCopy(tChar* destP,
                     tChar* srcP);
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
#define memErrInvalidParam            (memErrorClass | 3)   /* invalid param or requested size is too big */


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
tULong Engine_DiskSave(tDiskOperation* DiskOperation)
#endif
/***********************************************************************
 *
 *  PNOMain
 *
 ***********************************************************************/
{
#ifdef __PALMOS_
tDiskOperation* DiskOperation = (tDiskOperation*)userData68KP;
#endif
tMemPtrDeleteFct DeleteMemFct = (tMemPtrDeleteFct)DiskOperation->NativeCPC->MemPtrDeletePtr;
#if defined(_TRACE) || defined(_TESTU)
tNativeCPC* NativeCPC;
#endif /* _TRACE || _TESTU */
tDrive* Drive;
tDSKHeader* dhP;
tTrackHeader* thP;
tUChar* dataP;
tUShort Result = errNone;
tUShort pos;
tUChar track;
tUChar side;
tUChar sector;

#ifdef __PALMOS__
  NOT_USED(emulStateP);
  NOT_USED(call68KFuncP);
#endif

#ifdef __PALMOS__
  // Palm OS 68K interface
  Drive = (tDrive*)EndianSwap32(DiskOperation->Drive);
#if defined(_TRACE) || defined(_TESTU)
  NativeCPC = (tNativeCPC*)EndianSwap32(DiskOperation->NativeCPC);
#endif /* _TRACE || _TESTU */
#else
  Drive = DiskOperation->Drive;
#if defined(_TRACE) || defined(_TESTU)
  NativeCPC = DiskOperation->NativeCPC;
#endif /* _TRACE || _TESTU */
#endif

#ifdef _TESTU
  Result = PerformTestU(NativeCPC);
#endif /* _TESTU */

  if (Drive->dataP == cNull)
    return memErrInvalidParam;

  // Memory allocation
  dhP = (tDSKHeader*)Drive->dataP;;

  MemSet((tUChar*)dhP, sizeof(tDSKHeader), 0);
  StrCopy((tChar*)dhP->id, (tChar*)"EXTENDED CPC DSK File\r\nDisk-Info\r\n");
  StrCopy((tChar*)dhP->unused1, (tChar*)"CaPriCe\r\n");
  dhP->tracks = Drive->tracks;
  dhP->sides = (Drive->sides+1) | (Drive->random_DEs); // correct side count and indicate random DEs, if necessary

  //
  // Fill Disk header track size data
  //
  pos=0;
  for (track=0; track < Drive->tracks; track++) // loop for all tracks
  {
    for (side=0; side <= Drive->sides; side++) // loop for all sides
    {
      if (Drive->track[track][side].size) // track is formatted?
      {
        dhP->track_size[pos] = (Drive->track[track][side].size + sizeof(tTrackHeader)) >> 8; // track size + header in bytes
      }
      pos++;
    }
  }

  // Begin of first track
  dataP = Drive->dataP + sizeof(tDSKHeader);

  for (track=0; track < Drive->tracks; track++) // loop for all tracks
  {
    for (side=0; side <= Drive->sides; side++) // loop for all sides
    {
      thP = (tTrackHeader*)dataP;

      MemSet((tUChar*)thP,
             sizeof(tTrackHeader),
             0);
      StrCopy((tChar*)thP->id,
              (tChar*)"Track-Info\r\n");

      if (Drive->track[track][side].size) // track is formatted?
      {
        thP->track = track;
        thP->side = side;
        thP->bps = 2;
        thP->sectors = Drive->track[track][side].sectors;
        thP->gap3 = 0x4e;
        thP->filler = 0xe5;

        for (sector=0; sector < thP->sectors; sector++)
        {
          MemMove(&thP->sector[sector][0],
                  Drive->track[track][side].sector[sector].CHRN,
                  4); // copy CHRN
          MemMove(&thP->sector[sector][4],
                  Drive->track[track][side].sector[sector].flags,
                  2); // copy ST1 & ST2
          thP->sector[sector][6] = Drive->track[track][side].sector[sector].size & 0xff;
          thP->sector[sector][7] = (Drive->track[track][side].sector[sector].size >> 8) & 0xff; // sector size in bytes
        }
      }

      dataP += Drive->track[track][side].size + sizeof(tTrackHeader); // Next track
    }
  }

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


static tVoid StrCopy(tChar* destP,
                     tChar* srcP)
/***********************************************************************
 *
 *  MemSet
 *
 ***********************************************************************/
{
  while (*srcP != 0)
  {
    *(destP++) = *(srcP++);
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

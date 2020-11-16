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

// copied from ErrorBase.h
#define errNone                       0x0000  // No error

// copied from CoreTraps.h
#define sysTrapMemChunkFree           0xA012


//
// Routines
//
static tVoid MemSet(tUChar* destP,
                    tULong numBytes,
                    tUChar value);
//
// Palm OS routines
//
#ifdef __PALMOS__
static tUShort MemPtrFree(tVoid* chunkDataP,
                          const tVoid* emulStateP,
                          Call68KFuncType* call68KFuncP);
#endif
//
// TestU
//
#ifdef _TESTU
static tUShort PerformTestU(tNativeCPC* NativeCPC);
#endif /* _TESTU */



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
                          4); \
}
#endif /* _TRACE */

#ifdef __PALMOS__
tULong PNOMain(const tVoid* emulStateP,
               tVoid* userData68KP,
               Call68KFuncType* call68KFuncP)
#else
tULong Engine_DiskEject(tDiskOperation* DiskOperation)
#endif
/***********************************************************************
 *
 *  PNOMain
 *
 ***********************************************************************/
{
#ifdef __PALMOS__
tDiskOperation* DiskOperation = (tDiskOperation*)userData68KP;
#else
tMemPtrDeleteFct DeleteMemFct = (tMemPtrDeleteFct)DiskOperation->NativeCPC->MemPtrDeletePtr;
#endif /* __PALMOS__ */

#if defined(_TRACE) || defined(_TESTU)
tNativeCPC* NativeCPC;
#endif /* _TRACE || _TESTU */
tDrive* Drive;
tUShort Result = errNone;
tUChar CurrentTrack;

#ifdef __PALMOS__
  NOT_USED(emulStateP);
  NOT_USED(call68KFuncP);
#endif /* !_TRACE */

#ifdef __PALMOS__
  // Palm OS 68K interface
  Drive = (tDrive*)EndianSwap32(DiskOperation->Drive);
#if defined(_TESTU)
  NativeCPC = (tNativeCPC*)EndianSwap32(DiskOperation->NativeCPC);
#endif /* _TESTU */
#else
  Drive = DiskOperation->Drive;
#if defined(_TESTU)
  NativeCPC = DiskOperation->NativeCPC;
#endif /* _TESTU */
#endif /* __PALMOS__ || __WIN32__ */

  // Free disk data memory
  if (Drive->dataP != cNull)
  {
#if defined(__PALMOS__)
        // Free disk data memory
        MemPtrFree(Drive->dataP,
                   emulStateP,
                   call68KFuncP);
#else
        // Free disk data memory
        DeleteMemFct(Drive->dataP);
#endif

    Drive->dataP = cNull;
  }

  CurrentTrack = Drive->current_track; // save the drive head position
  MemSet((tUChar*)Drive,
         sizeof(tDrive),
         0); // clear drive info structure
  Drive->current_track = CurrentTrack;

  return (tULong)Result;
}



//==============================================================================
//
// Routines
//
//==============================================================================

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
static tUShort MemPtrFree(tVoid* chunkDataP,
                          const tVoid* emulStateP,
                          Call68KFuncType* call68KFuncP)
{
tUChar args[4];

  // Err MemChunkFree(MemPtr chunkDataP);
  // MemChunkFree Parameters
  *((tULong*)(&(args[0]))) = (tULong)EndianSwap32(chunkDataP);

  return (tUShort)call68KFuncP(emulStateP,
                               PceNativeTrapNo(sysTrapMemChunkFree),
                               &args,
                               (sizeof(args)/sizeof(args[0])));
}
#endif

//==============================================================================
//
// Unitary Tests
//
//==============================================================================
#ifdef _TESTU
// Prototypes of TestU fonctions
static tUShort TestU_MemSet_1(tNativeCPC* NativeCPC,
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

  if (Result == errNone) Result = TestU_MemSet_1(NativeCPC,
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

#endif /* _TESTU */


#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

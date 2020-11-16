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

#ifdef __PALMOS__
// copied from CoreTraps.h
#define sysTrapMemChunkFree           0xA012
#define sysTrapMemHandleUnlock        0xA022
#define sysTrapDmReleaseResource      0xA061
#endif

// copied from ErrorBase.h
#define errNone                       0x0000  // No error
#define memErrorClass                 0x0100  // Memory Manager

//
// Palm OS routines
//
#ifdef __PALMOS__
static tUShort MemPtrFree(tVoid* chunkDataP,
                          const tVoid* emulStateP,
                          Call68KFuncType* call68KFuncP);
static tUShort MemHandleUnlock(tVoid* h,
                               const tVoid* emulStateP,
                               Call68KFuncType* call68KFuncP);
static tUShort DmReleaseResource(void* resourceH,
                                 const tVoid* emulStateP,
                                 Call68KFuncType* call68KFuncP);
#endif

/***********************************************************************
 *
 *  Entry Points
 *
 ***********************************************************************/
#ifdef __PALMOS__
tULong PNOMain(const tVoid* emulStateP,
               tVoid* userData68KP,
               Call68KFuncType* call68KFuncP)
#else
tULong Engine_CPCStop(tNativeCPC* NativeCPC)
#endif
/***********************************************************************
 *
 *  PNOMain
 *
 ***********************************************************************/
{
#ifdef __PALMOS__
tNativeCPC* NativeCPC = (tNativeCPC*)userData68KP;
#endif
  // D�sallocation de la m�moire
  if (NativeCPC->pbROMlo != cNull)
  {
#ifdef __PALMOS__
    MemHandleUnlock(NativeCPC->hMemROMlo,
                    emulStateP,
                    call68KFuncP);
    DmReleaseResource(NativeCPC->hMemROMlo,
                      emulStateP,
                      call68KFuncP);
#endif
    NativeCPC->pbROMlo = cNull;
    NativeCPC->hMemROMlo = cNull;
    NativeCPC->pbROMhi = cNull;
    NativeCPC->pbExpansionROM = cNull;
  }

  if (NativeCPC->memmap_ROM != cNull)
  {
    if (NativeCPC->memmap_ROM[7] != cNull)
    {
#ifdef __PALMOS__
      MemHandleUnlock(NativeCPC->hMemAMSDOS,
                      emulStateP,
                      call68KFuncP);
      DmReleaseResource(NativeCPC->hMemAMSDOS,
                        emulStateP,
                        call68KFuncP);
#endif
      NativeCPC->hMemAMSDOS = cNull;
       NativeCPC->memmap_ROM[7] = cNull;
    }
  }

  return (tULong)errNone;
}

#ifdef __PALMOS__
//==============================================================================
//
// Palm OS routines
//
//==============================================================================

static tUShort MemPtrFree(tVoid* chunkDataP,
                          const tVoid* emulStateP,
                          Call68KFuncType* call68KFuncP)
/***********************************************************************
 *
 *  MemPtrFree
 *
 ***********************************************************************/
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
/*----------------------------------------------------------------------------*/


static tUShort MemHandleUnlock(tVoid* h,
                               const tVoid* emulStateP,
                               Call68KFuncType* call68KFuncP)
/***********************************************************************
 *
 *  MemHandleUnlock
 *
 ***********************************************************************/
{
tUChar args[4];

  // Err MemHandleUnlock(MemHandle h);
  // MemChunkFree Parameters
  *((tULong*)(&(args[0]))) = (tULong)EndianSwap32(h);

  return (tUShort)call68KFuncP(emulStateP,
                               PceNativeTrapNo(sysTrapMemHandleUnlock),
                               &args,
                               (sizeof(args)/sizeof(args[0])));
}
/*----------------------------------------------------------------------------*/


static tUShort DmReleaseResource(void* resourceH,
                                 const tVoid* emulStateP,
                                 Call68KFuncType* call68KFuncP)
/***********************************************************************
 *
 *  DmReleaseResource
 *
 ***********************************************************************/
{
tUChar args[4];

  // Err DmReleaseResource(MemHandle resourceH);
  // DmReleaseResource Parameters
  *((tULong*)(&(args[0]))) = (tULong)EndianSwap32(resourceH);

  return (tUShort)call68KFuncP(emulStateP,
                               PceNativeTrapNo(sysTrapDmReleaseResource),
                               &args,
                               (sizeof(args)/sizeof(args[0])));
}
/*----------------------------------------------------------------------------*/
#endif /*__PALMOS__*/

#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

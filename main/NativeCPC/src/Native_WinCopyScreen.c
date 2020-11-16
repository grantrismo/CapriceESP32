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

/*
** Make sure we can call this stuff from C++.
*/
#ifdef __cplusplus
extern "C" {
#endif

// Copied from PalmTypes.h
typedef tUShort				Err;

#include "Native_CPC.h"


//===================
// PATCH begin
#ifdef _PATCH_ENABLE

#endif /* _PATCH_ENABLE */
// PATCH end
//===================

#ifdef __PALMOS__
tULong PNOMain(const tVoid*,
               tVoid*,
               Call68KFuncType*);
#endif


// copied from ErrorBase.h
#define errNone                       0x0000  // No error


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
                          (sizeof(argsTrace)/sizeof(argsTrace[0]))); \
}
#endif /* _TRACE */

#ifdef __PALMOS__
tULong PNOMain(const tVoid* emulStateP,
               tVoid* userData68KP,
               Call68KFuncType* call68KFuncP)
#else
tULong Engine_WinCopyScreen(tNativeCPC* NativeCPC)
#endif
/***********************************************************************
 *
 *  PNOMain
 *
 ***********************************************************************/
{
#ifdef __PALMOS__
tNativeCPC* NativeCPC;
#endif
tUChar* Source;
tUChar* Destination;
tULong RowIndex, ColuwnIndex;
tULong SourceContent;
tUShort Result = errNone;

#ifdef __PALMOS__
  NativeCPC = (tNativeCPC*)userData68KP;
#endif

#ifdef _TESTU
  Result = PerformTestU(NativeCPC);
#endif /* _TESTU */

  // Prepare Copy
  Source = (tUChar*)NativeCPC->OffscreenStartBits;
  Destination = (tUChar*)NativeCPC->OnscreenStartBits;

  // Screen Copy
  for (RowIndex=NativeCPC->OffscreenCopyHeight; RowIndex; RowIndex--)
  {
    for (ColuwnIndex=NativeCPC->OffscreenCopyWidth; ColuwnIndex; ColuwnIndex--)
    {
      *Destination = *Source++;
      Destination -= NativeCPC->OnscreenPixelGap;
    }

    Source += NativeCPC->OffscreenAlignGap;
    Destination += NativeCPC->OnscreenAlignGap;
  }

  return (tULong)Result;
}
/*----------------------------------------------------------------------------*/



//==============================================================================
//
// Unitary Tests
//
//==============================================================================
#ifdef _TESTU
// Prototypes of TestU fonctions

static tUShort PerformTestU(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  PerformTestU
 *
 ***********************************************************************/
{
tUShort Result = errNone;
tUChar NoTest = 1;

  return (Result);
}
/*----------------------------------------------------------------------------*/

#endif /* _TESTU */


#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

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
tULong PNOMain(tVoid*,
               tULong,
               tVoid*,
               tULong*);
#endif
//
// Routines
//
static tVoid ShortMemMove(tUShort* destP,
                          tUShort* sourceP,
                          tULong numBytes);
static tVoid LongMemMove(tULong* destP,
                         tULong* sourceP,
                         tULong numBytes);


// copied from ErrorBase.h
#define errNone                       0x0000  // No error


/***********************************************************************
 *
 *  Entry Points
 *
 ***********************************************************************/
#ifdef __PALMOS__
tULong PNOMain(void *userDataP,
               tULong stream,
               tVoid* bufferP,
               tULong* bufferSizeP)
#else
tULong Engine_SoundCallback(tSoundCallbackParam* paramP, tVoid* bufferP, tULong* bufferSizeP)
#endif
/***********************************************************************
 *
 *  PNOMain
 *
 ***********************************************************************/
{
#ifdef __PALMOS
tSoundCallbackParam* paramP = (tSoundCallbackParam*)userDataP;
#endif
tUChar* CurrentPosP;
tULong FilledBufferSize;


  // Get Current Pos
  CurrentPosP = *paramP->CurrentPosPP;

  // Get and Reset Current Size
  FilledBufferSize = *paramP->CurrentSizeP;
  *paramP->CurrentSizeP = 0;


  // Buffer fully filled
  if ( (paramP->LastPosP >= CurrentPosP) && FilledBufferSize )
  {
  	*bufferSizeP = (tULong)paramP->SoundBufferEndP - (tULong)paramP->LastPosP + 1;
  	CurrentPosP = paramP->SoundBufferStartP;
    paramP->BufferRead = 1;
  }
  else
  {
  	*bufferSizeP = (tULong)CurrentPosP - (tULong)paramP->LastPosP;
  }

  // Copy requested amount to data
  if ((*bufferSizeP & 3) == 0)
  {
  	// 4-bytes copy
    LongMemMove((tULong*)bufferP,
                (tULong*)paramP->LastPosP,
                (*bufferSizeP / 4));
  }
  else
  {
  	// 2-bytes copy
    ShortMemMove((tUShort*)bufferP,
                 (tUShort*)paramP->LastPosP,
                 (*bufferSizeP / 2));
  }

  // Update Last position
  paramP->LastPosP = CurrentPosP;

  return errNone;
}
/*----------------------------------------------------------------------------*/


//==============================================================================
//
// Routines
//
//==============================================================================

static tVoid ShortMemMove(tUShort* destP,
                          tUShort* sourceP,
                          tULong Size)
/***********************************************************************
 *
 *  ShortMemMove
 *
 ***********************************************************************/
{
  while (Size--)
  {
    *destP++ = *sourceP++;
  }
}
/*----------------------------------------------------------------------------*/


static tVoid LongMemMove(tULong* destP,
                         tULong* sourceP,
                         tULong Size)
/***********************************************************************
 *
 *  LongMemMove
 *
 ***********************************************************************/
{
  while (Size--)
  {
    *destP++ = *sourceP++;
  }
}
/*----------------------------------------------------------------------------*/


#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

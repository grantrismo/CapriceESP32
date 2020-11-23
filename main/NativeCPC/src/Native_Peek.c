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

#include "types.h"
#include "Native_CPC.h"

/*
** Make sure we can call this stuff from C++.
*/
#ifdef __cplusplus
extern "C" {
#endif

// Copied from PalmTypes.h
typedef tUShort				Err;

// copied from ErrorBase.h
#define errNone                       0x0000  // No error





/***********************************************************************
 *
 *  Entry Points
 *
 ***********************************************************************/

tUChar Engine_Peek(tNativeCPC* NativeCPC, tULong addr)

/***********************************************************************
 *
 *  PNOMain
 *
 ***********************************************************************/
{

  //
  // CAUTION : addr MUST be 16-bits
  //

  {
    //Original
    //return (*(NativeCPC->membank_read[addr >> 14] + (addr & 0x3fff))); // returns a byte from a 16KB memory bank
    return (*(NativeCPC->membank_read[(addr >> 14) & 3] + (addr & 0x3fff))); // returns a byte from a 16KB memory bank
  }

}
/*----------------------------------------------------------------------------*/





#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

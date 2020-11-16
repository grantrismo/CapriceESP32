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

#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Types
typedef void                    tVoid;
typedef unsigned char           tBool;
typedef signed char             tChar;
typedef unsigned char           tUChar;
typedef signed short            tShort;
typedef unsigned short          tUShort;
typedef int32_t                 tLong;
typedef uint32_t                tULong;
typedef float                   tFloat;
typedef double                  tDouble;
typedef uint32_t                UInt32;
typedef bool                    Boolean;
typedef uint8_t                 UInt8;
typedef uint16_t                UInt16;
typedef char                    Char;
typedef int32_t                 Int32;


typedef tUShort DmResID;
typedef void* MemHandle;
typedef void* MemPtr;
typedef tUShort	Err;

typedef union
{
  tULong ULong;
  struct { tUShort usH, usL; } UShort;
  struct { tUChar ucHH, ucHL, ucLH, ucLL; } UChar;
} tFieldULong;
typedef union
{
  tULong ULong;
  struct { tUShort usL, usH; } UShort;
  struct { tUChar ucLL, ucLH, ucHL, ucHH; } UChar;
} tNativeULongField;


typedef union
{
  tUShort UShort;
  struct { tUChar ucH, ucL; } UChar;
} tFieldUShort;
typedef union
{
  tUShort UShort;
  struct { tUChar ucL, ucH; } UChar;
} tNativeUShortField;


typedef union
{
  struct { tUChar l, h, h2, h3; } b;
  struct { tUShort l, h; } w;
  tULong d;
} tRegister;


typedef enum
{
  ResultSucceed,
  ResultFailed
} tResult;


#define cNull                   ((tVoid*)0)
#define cFalse                  (0)
#define cTrue                   (!cFalse)


//
// MACRO TO AVOID WARNINGS FOR UNUSED PARAMETER OR VARIABLE
//
#ifndef NOT_USED
#define NOT_USED(p) (tVoid)(p);
#endif /* ! NOT_USED */

#endif

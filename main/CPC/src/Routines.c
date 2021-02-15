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
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#ifndef SIM
#include "esp_heap_caps.h"
#endif

#include "Resources.h"
#include "Routines.h"
//#include "CaPriCe.h"
//#include "Trace.h"
//#include "..\NativeCode\Native_CRC32.h"

#define memNewChunkFlagAllowLarge     0x1000
#define errNone                       0x0000  // No error
#define memErrorClass                 0x0100  // Memory Manager
// copied from MemoryMgr.h
#define memErrNotEnoughSpace          (memErrorClass | 2)

UInt32 TimGetTicks() {
/***********************************************************************
 *
 *  TimGetTicks
 *
 ***********************************************************************/
    struct timeval te;
    long            ms; // Milliseconds
    time_t          s;  // Seconds

    gettimeofday(&te, NULL); // get current time

    UInt32 milliseconds = (UInt32)(te.tv_sec*1000L + te.tv_usec/1000);
    return milliseconds;

}

UInt32 SysTicksPerSecond() {
/***********************************************************************
 *
 *  SysTicksPerSecond
 *
 ***********************************************************************/
	return 1000;
}

Err MemPtrNewLarge(UInt32 size,
                   void **newPtr)
/***********************************************************************
 *
 *  MemPtrNewLarge
 *
 ***********************************************************************/
{
  printf("Requesting %d Bytes\n",size);
  #ifdef SIM
  *newPtr = malloc(size);
  #else
  *newPtr = heap_caps_malloc(size,  MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  #endif

  if (*newPtr == NULL) return memErrNotEnoughSpace;

  return errNone;
}

void MemPtrFreeLarge(void* newPtr)
/***********************************************************************
 *
 *  MemPtrFreeLarge
 *
 ***********************************************************************/
{
  if (newPtr != NULL)
#ifdef SIM
  free(newPtr);
#else
  heap_caps_free(newPtr);
#endif
}

tVoid MemSet(tUChar* destP,
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

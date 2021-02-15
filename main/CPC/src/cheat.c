/*
    CaPriCe for ESP 32 - Amstrad CPC 464/664/6128 emulator for ESP32 devices

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
#include <stdlib.h>
#include <string.h>

#include "cheat.h"
#include "Files.h"
#include "types.h"
#include "Routines.h"

#define errOutOfRange                       0x0001

// some globals
CMF_header* cmf_header_base = NULL;
CMF_cheat* cmf_cheat_base = NULL;
CMF_poke*  cmf_poke_base = NULL;

// prototypes


// functions
void cmf_free()
/***********************************************************************
*
* 	 cmf_free()
*
***********************************************************************/
{
  if (cmf_header_base != NULL)
    MemPtrFreeLarge(cmf_header_base);

  cmf_cheat_base = NULL;
  cmf_poke_base = NULL;
  cmf_header_base = NULL;
}

Err cmf_read(const char* filename)
/***********************************************************************
*
* 	 cmf_read()
*
***********************************************************************/
{
  UInt16 volRefNum = 0;
  FileRef CheatFileRef;
  UInt32 fileSize;
  UInt32 numBytesRead;
  Err Result;


  // check if file exists
  Result = VFSFileOpen(0, filename, vfsModeRead, &CheatFileRef);
  if (Result != errNone)
  {
    printf("Cheat file not found\n");
    return Result;
  }

  // Get File size
  Result = VFSFileSize(CheatFileRef, &fileSize);
  if (Result != errNone)
  {
    printf("File size read issue\n");
    return Result;
  }

  // free in case allocated
  if (cmf_header_base != NULL)
    cmf_free();

  // Allocate memory for whole disk data
  MemPtrNewLarge(fileSize, (void**)&cmf_header_base);

  if (cmf_header_base == NULL)
  {
    printf("Memory allocation issue\n");
    Result = memErrNotEnoughSpace;
    return Result;
  }

  do
  {
    // Read entire disk image file
    VFSFileRead(CheatFileRef, fileSize, (void*)(cmf_header_base), &numBytesRead);
    VFSFileClose(CheatFileRef);

    // Check read
    if (numBytesRead != fileSize)
    {
      printf("File Read Error\n");
      Result = vfsErrBadData;
      continue;
    }

    // check file Header
    if ((cmf_header_base)->magic != CHEAT_FILE_MAGIC)
    {
      printf("File Magic Error\n");
      Result = vfsErrBadData;
      continue;
    }
    else
    {
      printf("Cheat file loaded and valid\n");
      cmf_cheat_base = (CMF_cheat*)(cmf_header_base + 1);
      cmf_poke_base = (CMF_poke*)(cmf_cheat_base + cmf_header_base->length);
      Result = errNone;
      return Result;
    }
  }
  while(0);

  cmf_free();

  VFSFileClose(CheatFileRef);

  return (Result);
}

void cmf_print()
/***********************************************************************
*
* 	 cmf_print()
*
***********************************************************************/
{

  if (cmf_header_base == NULL)
    return;

  printf ("Game: %s\n", cmf_header_base->title);
  printf ("Cheat used for the game: %d\n", cmf_header_base->index);
  printf ("Number of Cheats: %d\n", cmf_header_base->length);

  for (int i=0; i<cmf_header_base->length; i++)
  {
    printf ("Cheat description: %s\n", cmf_cheat_base[i].description);
    printf ("Running index number: %d\n", cmf_cheat_base[i].index);
    printf ("Number of Pokes part of the Cheat: %d\n", cmf_cheat_base[i].length);

    for (int j=cmf_cheat_base[i].index; j<cmf_cheat_base[i].length; j++)
    {
      printf ("Poke address: 0x%04x\n",cmf_poke_base[i].address);
      printf ("Poke value: 0x%02x\n",cmf_poke_base[i].cheat_code);
      printf ("Original code: 0x%02x\n",cmf_poke_base[i].original_code);
    }
  }
}

CMF_cheat* cmf_getcheat()
/***********************************************************************
*
* 	 cmf_getcheat()
*
***********************************************************************/
{
  if ((cmf_header_base != NULL) && (cmf_cheat_base != NULL))
  {
    if (cmf_header_base->index <= cmf_header_base->length)
      return (&(cmf_cheat_base[cmf_header_base->index]));
  }

  return NULL;
}


CMF_poke* cmf_getpoke(int poke_index )
/***********************************************************************
*
* 	 cmf_getpoke()
*
***********************************************************************/
{
  if ((cmf_header_base != NULL) && (cmf_cheat_base != NULL))
  {
      if (cmf_header_base->index <= cmf_header_base->length)
      {
        int poke_list_start = cmf_cheat_base[cmf_header_base->index].index;
        int poke_list_lenght = cmf_cheat_base[cmf_header_base->index].length;
        if ((poke_index >= poke_list_start) && (poke_index < (poke_list_start + poke_list_lenght)))
          return (&(cmf_poke_base[poke_index]));
      }
  }

  return (NULL);
}

Err cmf_setcheat(int cheat_index)
/***********************************************************************
*
* 	 cmf_setcheat()
*
***********************************************************************/
{
  if ((cmf_header_base != NULL) && (cmf_cheat_base != NULL))
  {
      if (cheat_index <= cmf_header_base->length)
      {
        cmf_header_base->index = cheat_index;
        return (errNone);
      }
  }
  return (errOutOfRange);
}

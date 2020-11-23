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

/* Cheat mote data format

 1 File name idetical with game with the extension . cheat mode format .cmf
*/

#ifndef __CHEAT_H
#define __CHEAT_H

#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#include "vfsfile.h"
#include "types.h"

#define CMF_MAXTEXTLENGTH (64)

typedef struct
{                             // an atom Cheat consists of the trupple: address, the poke code and the original code
 	uint16_t address;                   // memory addess
 	uint8_t cheat_code;                 // poke value
 	uint8_t original_code;              // original value, please note: in case the original_code does not match, the cheat will not be applied
} CMF_poke;

typedef struct
{                             // a single cheat contains of:
 	char description[CMF_MAXTEXTLENGTH];         // description of the Cheat (lives, power , unvolunaribility,..)
  uint32_t index;                              // the list index (running number from 0 to x)
 	uint32_t length;                          // the lenght of the Cheat. Can contain multiple pokes
} CMF_cheat;

typedef struct
{                             // the file header
  uint32_t magic;                       // magic is .cmf == 0x2E636D66
 	char title[CMF_MAXTEXTLENGTH];        // normally the game title the cheat corresbonds to
  uint32_t  index;                 // which cheat shall be used in the game
 	uint32_t  length;                 // number if individual cheats the file contains
} CMF_header;

extern Err cmf_read(const char* filename);
extern void cmf_print();
extern void cmf_free();
extern CMF_cheat* cmf_getcheat();
extern CMF_poke* cmf_getpoke(int poke_index);
extern Err cmf_setcheat(int cheat_index);

#endif

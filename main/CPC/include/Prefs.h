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

#ifndef PREFS_H
#define PREFS_H

#include "types.h"
#include "sections.h"
#include "Native_CPC.h"
#include "Files.h"


#define ROCKER_CENTER_CPC_ESCAPE_INDEX      0
#define ROCKER_CENTER_CPC_RETURN_INDEX      1
#define ROCKER_CENTER_CPC_CONTROL_INDEX     2
#define ROCKER_CENTER_CPC_COPY_INDEX        3

#define HARDKEY_NOT_USED_INDEX              0
#define HARDKEY_ROCKER_CENTER_INDEX         1
#define HARDKEY_MENU_INDEX                  2
#define HARDKEY_MINI_KEYBOARD_INDEX         3
#define HARDKEY_FULLSCREEN_INDEX            4
#define HARDKEY_DECATHLON_INDEX             5
#define HARDKEY_CPCKEYCODE_A_INDEX          6
#define HARDKEY_CPCKEYCODE_B_INDEX          7
#define HARDKEY_CPCKEYCODE_C_INDEX          8
#define HARDKEY_CPCKEYCODE_D_INDEX          9

#define memErrorClass                 0x0100  // Memory Manager
// copied from MemoryMgr.h
#define memErrNotEnoughSpace          (memErrorClass | 2)

extern tPreferences* prefP;

extern Err LirePreferences(tPreferences** prefPP) SECTION_PREF;
extern void EcrirePreferences(tPreferences* prefP) SECTION_PREF;

//FC!! Provoque Crash
/*extern void SetHardKeysMask(const tPreferences* prefP,
                              UInt32 originKeyMask) SECTION_PREF;*/
extern void SetHardKeysMask(const tPreferences* prefP,
                            UInt32 originKeyMask);

#endif /* ! PREFS_H */

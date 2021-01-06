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

#ifndef SOUND_H
#define SOUND_H

#include "types.h"
#include "sections.h"
#include "Native_CPC.h"

extern tUChar** SoundBufferP[2];
extern tUChar SoundBufferCurrent;
extern tULong SoundBytesToWrite;
extern tUChar SoundBufferRead;
extern tUChar *pbSndBufferLastP;

extern Err SoundBufferAlloc() SECTION_SOUND;
extern tVoid SoundBufferFree() SECTION_SOUND;
extern Err SoundStart(tNativeCPC* NativeCPC) SECTION_SOUND;
extern Err SoundBufferReset(tNativeCPC* NativeCPC) SECTION_SOUND;
extern tVoid SoundStop(tNativeCPC* NativeCPC) SECTION_SOUND;
extern tVoid SoundPlay(tNativeCPC* NativeCPC) SECTION_SOUND;
extern tVoid SoundPause(tNativeCPC* NativeCPC) SECTION_SOUND;
extern Int32 SoundPush(tNativeCPC* NativeCPC) SECTION_SOUND;
extern Int32 SoundVariableCallback(tNativeCPC* NativeCPC, UInt8 *data, Int32 len) SECTION_SOUND;

extern tVoid Sound_Calculate_Level_Tables(tNativeCPC* NativeCPC) SECTION_SOUND;
extern tBool IsBufferRead() SECTION_SOUND;

extern Err SoundIncreaseVolume(tUChar step) SECTION_SOUND;
extern Err SoundDecreaseVolume(tUChar step) SECTION_SOUND;

#endif // SOUND_H

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

#ifndef _SECTIONS_H
#define _SECTIONS_H

#ifdef SIM
#define SECTION_DISPLAY         __attribute__ ((section ("display")))
#define SECTION_SOUND           __attribute__ ((section ("sound")))
#define SECTION_MATH            __attribute__ ((section ("math")))
#define SECTION_KEYBOARD        __attribute__ ((section ("keyboard")))
#define SECTION_FILES           __attribute__ ((section ("files")))
#define SECTION_ROUTINES        __attribute__ ((section ("routines")))
#define SECTION_PREF            __attribute__ ((section ("pref")))
#define SECTION_GUI             __attribute__ ((section ("gui")))
#else
#define SECTION_DISPLAY
#define SECTION_SOUND
#define SECTION_MATH
#define SECTION_KEYBOARD
#define SECTION_FILES
#define SECTION_ROUTINES
#define SECTION_PREF
#define SECTION_GUI
#endif

#endif

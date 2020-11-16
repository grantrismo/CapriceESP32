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

#ifndef MATH_MATH_H
#define MATH_MATH_H

#include "types.h"
#include "sections.h"

extern tDouble math_exp(tDouble x) SECTION_MATH;
extern tDouble math_rint(tDouble x) SECTION_MATH;

#define LOG_2								  3.01029995664e-01
#define M_LOG_2               0.693147180559945309417


#endif

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

#ifndef TRACE_H
#define TRACE_H

#ifdef __PALMOS__
#include <PalmOS.h>
#endif

#include "Resources.h"
#include <stdio.h>

#define _TRACEME

#ifdef _TRACEME
#define TRACE_SHOW_INT(title, val) \
{ \
  printf("%s : %d\n",title, val); \
}
#else
#define TRACE_SHOW_INT(title, val)
#endif /* _TRACE */


#ifdef _TRACEME
#define TRACE_SHOW_HEX(title, val) \
{ \
    printf("%s : %x\n",title, val); \
}
#else
#define TRACE_SHOW_HEX(title, val)
#endif /* _TRACE */



#ifdef _TRACEME
#define TRACE_SHOW_TEXT(title, text) \
{ \
  printf("%s : %s\n",title, val); \
}
#else
#define TRACE_SHOW_TEXT(title, val)
#endif /* _TRACE */


#endif /* TRACE_H */

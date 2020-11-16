/*
    CaPriCe for Palm OS - Amstrad CPC 464/664/6128 emulator for Palm devices
    Copyright (C) 2009  Frédéric Coste

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

#ifndef TRACE_DATA_H
#define TRACE_DATA_H

#include "types.h"


#define TRACE_DATA(function, index, data) ((tULong)( ((function&0xffff) << 16) | \
                                           ((index&0xff) << 8) | ((data&0xff) )))


// List of functions
#define TRACE_FN_CPCExecute_Main                0x0001
#define TRACE_FN_z80_pfx_cb                     0x0002
#define TRACE_FN_z80_pfx_dd                     0x0003
#define TRACE_FN_z80_pfx_ddcb                   0x0004
#define TRACE_FN_z80_pfx_ed                     0x0005
#define TRACE_FN_z80_pfx_fd                     0x0006
#define TRACE_FN_z80_pfx_fdcb                   0x0007
#define TRACE_FN_AutoStart                      0x0008
#define TRACE_FN_DetectCatalog                  0x0009

#endif /* TRACE_H */

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

#ifndef NATIVE_DISK_H
#define NATIVE_DISK_H

#include "Native_CPC.h"


/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/
 
#define MAX_DISK_FORMAT 										2
#define FIRST_CUSTOM_DISK_FORMAT						2

#define DISK_FORMAT_DATA										0
#define DISK_FORMAT_VENDOR									1

#define DEFAULT_DISK_FORMAT									DISK_FORMAT_DATA

typedef enum
{
	AutoStartOK = 0,
	DiskFormatUnknown,
	NoValidFileEntry,
	TooManyFileEntries
} tAutoStartReturnCode;


typedef struct 
{
   tUChar label[40]; // label to display in options dialog
   tULong tracks; // number of tracks
   tULong sides; // number of sides
   tULong sectors; // sectors per track
   tULong sector_size; // sector size as N value
   tULong gap3_length; // GAP#3 size
   tUChar filler_byte; // default byte to use
   tUChar sector_ids[2][16]; // sector IDs
} tDiskFormat;


typedef struct
{
   tChar id[34];
   tChar unused1[14];
   tUChar tracks;
   tUChar sides;
   tUChar unused2[2];
   tUChar track_size[DSK_TRACKMAX * DSK_SIDEMAX];
} tDSKHeader;


typedef struct {
   tChar id[12];
   tChar unused1[4];
   tUChar track;
   tUChar side;
   tUChar unused2[2];
   tUChar bps;
   tUChar sectors;
   tUChar gap3;
   tUChar filler;
   tUChar sector[DSK_SECTORMAX][8];
} tTrackHeader;


typedef struct
{
	tNativeCPC* NativeCPC;
	tDrive* Drive;
	tDiskFormat* DiskFormatTableP;
	tUChar* DiskContentP;
	tVoid* Param;
	tULong disk_size;
	tUChar FormatType;
} tDiskOperation;


#endif /* ! NATIVE_DISK_H */

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

#ifndef VFSFILE_H
#define VFSFILE_H

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include "types.h"
#include "sections.h"

/************************************************************
 * Error codes
 *************************************************************/
#define	vfsErrorClass				0x2A00	// Virtual Filesystem Manager and Filesystem library
#define vfsErrBufferOverflow			(vfsErrorClass | 1)	// passed in buffer is too small
#define vfsErrFileGeneric				(vfsErrorClass | 2)	// Generic file error.
#define vfsErrFileBadRef				(vfsErrorClass | 3)	// the fileref is invalid (has been closed, or was not obtained from VFSFileOpen())
#define vfsErrFileStillOpen			(vfsErrorClass | 4)	// returned from FSFileDelete if the file is still open
#define vfsErrFilePermissionDenied	(vfsErrorClass | 5)	// The file is read only
#define vfsErrFileAlreadyExists		(vfsErrorClass | 6)	// a file of this name exists already in this location
#define vfsErrFileEOF					(vfsErrorClass | 7)	// file pointer is at end of file
#define vfsErrFileNotFound				(vfsErrorClass | 8)	// file was not found at the path specified
#define vfsErrVolumeBadRef				(vfsErrorClass | 9)	// the volume refnum is invalid.
#define vfsErrVolumeStillMounted		(vfsErrorClass | 10)	// returned from FSVolumeFormat if the volume is still mounted
#define vfsErrNoFileSystem				(vfsErrorClass | 11)	// no installed filesystem supports this operation
#define vfsErrBadData					(vfsErrorClass | 12)	// operation could not be completed because of invalid data (i.e., import DB from .PRC file)
#define vfsErrDirNotEmpty				(vfsErrorClass | 13)	// can't delete a non-empty directory
#define vfsErrBadName					(vfsErrorClass | 14)	// invalid filename, or path, or volume label or something...
#define vfsErrVolumeFull				(vfsErrorClass | 15)	// not enough space left on volume
#define vfsErrUnimplemented			(vfsErrorClass | 16)	// this call is not implemented
#define vfsErrNotADirectory			(vfsErrorClass | 17)	// This operation requires a directory
#define vfsErrIsADirectory          (vfsErrorClass | 18) // This operation requires a regular file, not a directory
#define vfsErrDirectoryNotFound		(vfsErrorClass | 19) // Returned from VFSFileCreate when the path leading up to the new file does not exist
#define vfsErrNameShortened			(vfsErrorClass | 20) // A volume name or filename was automatically shortened to conform to filesystem spec

#define vfsModeExclusive			(0x0001U)		// don't let anyone else open it
#define vfsModeRead					(0x0002U)		// open for read access
#define vfsModeWrite					(0x0004U | vfsModeExclusive)		// open for write access, implies exclusive
#define vfsModeCreate				(0x0008U)		// create the file if it doesn't already exist.  Implemented in VFS layer, no FS lib call will ever have to handle this.
#define vfsModeTruncate				(0x0010U)		// Truncate file to 0 bytes after opening, removing all existing data.  Implemented in VFS layer, no FS lib call will ever have to handle this.
#define vfsModeReadWrite			(vfsModeWrite | vfsModeRead)		// open for read/write access
#define vfsModeLeaveOpen			(0x0020U)		// Leave the file open even if when the foreground task closes
#define vfsModeDirExist       (0x0040U)   // added to check for directory existanc

#define vfsIteratorStart              0
#define vfsIteratorStop               1

#define vfsOriginBeginning		0	// from the beginning (first data byte of file)
#define vfsOriginCurrent		1	// from the current position
#define vfsOriginEnd				2	// from the end of file (one position beyond last data byte, only negative offsets are legal)

typedef FILE*               FileRef;
typedef UInt16	            FileOrigin;
#define _PTR void *

// Odroid I2S Mutex Sdcard access Wrapper
#ifdef SIM
#define _fopen fopen
#define _fclose fclose
#define _opendir opendir
#define _readdir readdir
#define _closedir closedir
#define _fseek fseek
#define _ftell ftell
#define _fread fread
#define _fwrite fwrite
#define _fgets fgets
#define _fputs fputs
#define _remove remove
#define _rename rename
#else
extern DIR* _opendir(const char* name)  SECTION_FILES;
extern struct dirent* _readdir(DIR* pdir) SECTION_FILES;
extern int _closedir(DIR* f) SECTION_FILES;
extern FILE* _fopen(const char *__restrict _name, const char *__restrict _type) SECTION_FILES;
extern int _fclose(FILE* file) SECTION_FILES;
extern int _fseek(FILE * f, long a, int b) SECTION_FILES;
extern long _ftell( FILE * f) SECTION_FILES;
extern size_t _fread(_PTR __restrict p, size_t _size, size_t _n, FILE *__restrict f) SECTION_FILES;
extern size_t _fwrite(const _PTR __restrict p , size_t _size, size_t _n, FILE * f) SECTION_FILES;
extern char * _fgets(char *__restrict c, int i, FILE *__restrict f) SECTION_FILES;
extern int _fputs(const char *__restrict c, FILE *__restrict f) SECTION_FILES;
extern int _remove(const char * f) SECTION_FILES;
extern int _rename(const char * f, const char * nf) SECTION_FILES;
#endif

// Caprice PalmOS Wrapper
extern Err VFSVolumeEnumerate(UInt16 *volRefNumP, UInt32 *volIteratorP) SECTION_FILES;
extern Err VFSFileOpen(UInt16 volRefNum, const char *pathNameP, UInt16 openMode, FileRef* fileRefP) SECTION_FILES;
extern Err VFSFileClose(FileRef fileRef) SECTION_FILES;
extern Err VFSDirCreate(UInt16 volRefNum, const char *dirNameP) SECTION_FILES;
extern Err VFSFileCreate(UInt16 volRefNum, const char *pathNameP) SECTION_FILES;
extern Err VFSFileSize(FileRef fileRef, UInt32 *fileSizeP) SECTION_FILES;
extern Err VFSFileRead(FileRef fileRef, UInt32 numBytes, void *bufP, UInt32 *numBytesReadP) SECTION_FILES;
extern Err VFSFileGetAttributes(FileRef fileRef, UInt32 *attributesP) SECTION_FILES;
extern Err VFSFileWrite(FileRef fileRef, UInt32 numBytes, const void *dataP, UInt32 *numBytesWrittenP) SECTION_FILES;
extern Err VFSFileSeek(FileRef fileRef, FileOrigin origin, Int32 offset) SECTION_FILES;
extern Err VFSFileDelete(UInt16 volRefNum, const Char *pathNameP) SECTION_FILES;

#endif

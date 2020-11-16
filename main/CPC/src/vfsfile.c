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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#include "types.h"
#include "vfsfile.h"

#define errNone                       0x0000  // No error
//typedef FILE                FileRef;

Err VFSVolumeEnumerate(UInt16 *volRefNumP, UInt32 *volIteratorP)
{
  *volIteratorP = vfsIteratorStop;
  *volRefNumP = 1;

  printf("VFSVolumeEnumerate - Done\n");
  return errNone;
}
/************************************************************************************/

Err VFSFileOpen(UInt16 volRefNum, const char *pathNameP, UInt16 openMode, FileRef* fileRefP)
{

  DIR* dir = NULL;
  printf("VFSFileOpen: %s\n", pathNameP);


  switch (openMode)
  {
    case vfsModeExclusive:	// don't let anyone else open it
    *fileRefP = fopen(pathNameP, "rb");
    break;

    case vfsModeRead:		  	// open for read access
    *fileRefP = fopen(pathNameP, "rb");
    break;

    case vfsModeWrite:			// open for write access, implies exclusive
    *fileRefP = fopen(pathNameP, "wb");
    break;

    case vfsModeCreate:				// / create the file if it doesn't already exist.  Implemented in VFS layer, no FS lib call will ever have to handle this.
    *fileRefP = fopen(pathNameP, "wb");
    break;

    case vfsModeTruncate:		 // Truncate file to 0 bytes after opening, removing all existing data.  Implemented in VFS layer, no FS lib call will ever have to handle this.
    *fileRefP = fopen(pathNameP, "wb");
    break;

    case vfsModeReadWrite:	   // open for read/write access
    *fileRefP = fopen(pathNameP, "wb+");
    break;

    case vfsModeLeaveOpen: 	 // Leave the file open even if when the foreground task closes
    *fileRefP = fopen(pathNameP, "wb+");
    break;

    case vfsModeDirExist:
      dir = opendir(pathNameP);
      if (dir) {
          /* Directory exists. */
          closedir(dir);
          *fileRefP = NULL;
          return errNone;
      } else if (ENOENT == errno) {
        return(vfsErrNotADirectory);
      } else {
          return(vfsErrDirectoryNotFound);
      }
      break;
  }

  if (*fileRefP == NULL)
    return vfsErrFileNotFound;

  return errNone;
}
/************************************************************************************/

Err VFSFileClose(FileRef fileRef)
{
  if (fileRef!=NULL)
    fclose(fileRef);
  return errNone;
}
/************************************************************************************/

Err VFSDirCreate(UInt16 volRefNum, const char *dirNameP)
{
  // volRefNum returned from VFSVolumeEnumerate
  // pointer to the full dir
  return errNone;
}

/************************************************************************************/
Err VFSFileCreate(UInt16 volRefNum, const char *pathNameP)
{
  // volRefNum returned from VFSVolumeEnumerate
  // pointer to the full path of the filename

  //
  // vfsErrBadName
  // vfsErrFileAlreadyExists
  return errNone;
}

/************************************************************************************/
Err VFSFileSize(FileRef fileRef, UInt32 *fileSizeP)
{
  /*Move file point at the end of file.*/
  fseek(fileRef,0,SEEK_END);
  *fileSizeP=ftell(fileRef);
  fseek(fileRef, 0, SEEK_SET);
  printf("VFSFileSize: %d\n",*fileSizeP);
  return errNone;
}

/************************************************************************************/
Err VFSFileRead(FileRef fileRef, UInt32 numBytes, void *bufP, UInt32 *numBytesReadP)
{
  *numBytesReadP = fread(bufP, 1, numBytes, fileRef);

  printf("VFSFileRead: %d %d %p\n",*numBytesReadP,numBytes,fileRef );
  return errNone;
}

/************************************************************************************/
Err VFSFileGetAttributes(FileRef fileRef, UInt32 *attributesP)
{
  return errNone;
}

/************************************************************************************/

Err VFSFileWrite(FileRef fileRef, UInt32 numBytes, const void *dataP, UInt32 *numBytesWrittenP)
{
  return errNone;
}

/************************************************************************************/
Err VFSFileSeek(FileRef fileRef, FileOrigin origin, Int32 offset)
{
  return errNone;
}

/************************************************************************************/
Err VFSFileDelete(UInt16 volRefNum, const Char *pathNameP)
{
  return errNone;
}

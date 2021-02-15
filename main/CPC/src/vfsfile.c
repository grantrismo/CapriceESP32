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

#include "display.h"
#include "types.h"
#include "vfsfile.h"

#define errNone                       0x0000  // No error
//typedef FILE                FileRef;

/************************************************************************************/
// ODRIOD I2S SDCARD Wrapper
/************************************************************************************/
#ifndef SIM
int openfiles = 0;
FILE* _fopen(const char *__restrict _name, const char *__restrict _type)
{

    STOP_DISPLAY_FUNCTION();
    FILE* f = fopen(_name, _type);
    RESUME_DISPLAY_FUNCTION();
    //printf("fopen: %s (%s) %p\n", buffer, _type, f);
    if (f)openfiles++;
    //printf("open files: %d (%d)\n",openfiles,f);

    return f;

}
int _fclose(FILE* file)
{

    STOP_DISPLAY_FUNCTION();
    fflush(file);
    int res =  fclose(file);
    RESUME_DISPLAY_FUNCTION();
    if (!res)openfiles--;
    //printf("open files: %d\n",openfiles );
    return res;
}

DIR* _opendir(const char* name)
{
    STOP_DISPLAY_FUNCTION();
    DIR* d = opendir(name);
    RESUME_DISPLAY_FUNCTION();
    return d;

}

struct dirent* _readdir(DIR* pdir)
{
    STOP_DISPLAY_FUNCTION();
    struct dirent* r = readdir(pdir);
    RESUME_DISPLAY_FUNCTION();
    return r;
}

int _closedir(DIR* f)
{
    int res;
    STOP_DISPLAY_FUNCTION();
    res = closedir(f);
    RESUME_DISPLAY_FUNCTION();
    return res;
}

int _fseek(FILE * f, long a, int b)
{
    STOP_DISPLAY_FUNCTION();
    int ret = fseek(f, a, b);
    RESUME_DISPLAY_FUNCTION();
    return ret;
}

long _ftell( FILE * f)
{
    STOP_DISPLAY_FUNCTION();
    long r = ftell(f);
    RESUME_DISPLAY_FUNCTION();
    return r;
}

size_t _fread(_PTR __restrict p, size_t _size, size_t _n, FILE *__restrict f)
{

    size_t s;
    STOP_DISPLAY_FUNCTION();
    s= fread(p, _size, _n, f);
    RESUME_DISPLAY_FUNCTION();
    return s;
    /*size_t size = _size*_n;
    size_t readed = 0;
    for (int i = 0; i < size; i+=0x100) {
        int toRead = 0x100;
        if (i + 0x100 > size) toRead = size%0x100;
        STOP_DISPLAY_FUNCTION();
        size_t r =fread(p+readed, toRead, 1, f);
        RESUME_DISPLAY_FUNCTION();
        if (r == 1) readed += toRead;
        if (r == 0) break;

    }

    return _n;*/
}

size_t _fwrite(const _PTR __restrict p , size_t _size, size_t _n, FILE * f) {
    // i had problems with fwrite (sd write errors, if the datasize is > 0x100)

    size_t size = _size*_n;
    int written = 0;
    for (int i = 0; i < size; i+=0x100) {
        int toWrite = 0x100;
        if (i + 0x100 > size) toWrite = size%0x100;
        STOP_DISPLAY_FUNCTION();
        written +=fwrite(p+i, 1, toWrite, f);
        RESUME_DISPLAY_FUNCTION();
    }
    if (written > 0) {
        fflush(f);
        fsync(fileno(f));

    }
    return written;
}

char * _fgets(char *__restrict c, int i, FILE *__restrict f)
{
    STOP_DISPLAY_FUNCTION();
    char * res = fgets(c, i, f);
    RESUME_DISPLAY_FUNCTION();
    return res;
}

int _fputs(const char *__restrict c, FILE *__restrict f)
{
    STOP_DISPLAY_FUNCTION();
    int res = fputs(c,f);
    RESUME_DISPLAY_FUNCTION();
    return res;
}


int _remove(const char * f)
{
    int res;
    STOP_DISPLAY_FUNCTION();
    res = remove(f);
    RESUME_DISPLAY_FUNCTION();
    return res;
}

int _rename(const char * f, const char * nf)
{
    int res;
    STOP_DISPLAY_FUNCTION();
    res = rename(f, nf);
    RESUME_DISPLAY_FUNCTION();
    return res;
}
#endif
/************************************************************************************/
// CAPRICE VFS Wrapper
/************************************************************************************/

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
    *fileRefP = _fopen(pathNameP, "rb");
    break;

    case vfsModeRead:		  	// open for read access
    *fileRefP = _fopen(pathNameP, "rb");
    break;

    case vfsModeWrite:			// open for write access, implies exclusive
    *fileRefP = _fopen(pathNameP, "wb");
    break;

    case vfsModeCreate:				// / create the file if it doesn't already exist.  Implemented in VFS layer, no FS lib call will ever have to handle this.
    *fileRefP = _fopen(pathNameP, "wb");
    break;

    case vfsModeTruncate:		 // Truncate file to 0 bytes after opening, removing all existing data.  Implemented in VFS layer, no FS lib call will ever have to handle this.
    *fileRefP = _fopen(pathNameP, "wb");
    break;

    case vfsModeReadWrite:	   // open for read/write access
    *fileRefP = _fopen(pathNameP, "wb+");
    break;

    case vfsModeLeaveOpen: 	 // Leave the file open even if when the foreground task closes
    *fileRefP = _fopen(pathNameP, "wb+");
    break;

    case vfsModeDirExist:
      dir = _opendir(pathNameP);
      if (dir) {
          /* Directory exists. */
          _closedir(dir);
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
    _fclose(fileRef);
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
  _fseek(fileRef,0,SEEK_END);
  *fileSizeP=_ftell(fileRef);
  _fseek(fileRef, 0, SEEK_SET);
  printf("VFSFileSize: %d\n",*fileSizeP);
  return errNone;
}

/************************************************************************************/
Err VFSFileRead(FileRef fileRef, UInt32 numBytes, void *bufP, UInt32 *numBytesReadP)
{
  *numBytesReadP = _fread(bufP, 1, numBytes, fileRef);

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
  *numBytesWrittenP = _fwrite(dataP , 1, numBytes, fileRef);
  printf("VFSFileWrite: %d %d %p\n",*numBytesWrittenP, numBytes, fileRef);
  return errNone;
}

/************************************************************************************/
Err VFSFileSeek(FileRef fileRef, FileOrigin origin, Int32 offset)
{
  Err Error;

  switch (origin)
  {
    case vfsOriginCurrent:
      Error = _fseek(fileRef,offset,SEEK_CUR);
      break;

    case vfsOriginEnd:
      Error = _fseek(fileRef,offset,SEEK_END);
    	break;

    default: //vfsOriginBeginning
      Error = _fseek(fileRef,offset,SEEK_SET);
  }
  return(Error);
}

/************************************************************************************/
Err VFSFileDelete(UInt16 volRefNum, const Char *pathNameP)
{
  return errNone;
}

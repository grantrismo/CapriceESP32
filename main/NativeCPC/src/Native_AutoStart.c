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
#ifdef __PALMOS__
#include <PceNativeCall.h>
#include <ByteOrderUtils.h>
#endif

#include "types.h"
#include "trace_data.h"


/*
** Make sure we can call this stuff from C++.
*/
#ifdef __cplusplus
extern "C" {
#endif


#include "Native_CPC.h"


//===================
// PATCH begin
#ifdef _PATCH_ENABLE

#endif /* _PATCH_ENABLE */
// PATCH end
//===================



#define FILE_ENTRY_SIZE                32

#ifdef __PALMOS__
tULong PNOMain(const tVoid*,
               tVoid*,
               Call68KFuncType*);
#endif

#if defined(__GNUC__)
// This seemingly bogus function satisfies the Windows LoadLibrary() call,
// (and must be the first function in our PNO .dll).
int /*WINAPI*/ my_bogus_cygwin_dll_entry(unsigned long /*HANDLE*/ h,
                                         unsigned long /*DWORD*/ reason,
                                         void* p)
{
  return 1;
}
#endif


//
// Internal functions
//
static tULong DetectCatalog(tDrive* driveP,
                            tTrack** trackPP,
                            tUChar** firstFileEntryPP
#if defined(_TRACE)
                            ,
                            tNativeCPC* NativeCPC
#endif /* _TRACE */
                           );




/***********************************************************************
 *
 *  TRACE
 *
 ***********************************************************************/
#ifdef _TRACE
#  define SHOWTRACE(p) \
{ \
tUChar argsTrace[4]; \
  if (NativeCPC->TraceDisplay) \
  { \
    NativeCPC->TraceFollowUp = (tULong)p; \
    *((tULong*)(&(argsTrace[0]))) = EndianSwap32(p); \
    NativeCPC->call68KFuncP(NativeCPC->emulStateP, \
                            NativeCPC->TraceAlertPtr, \
                            &argsTrace, \
                            (sizeof(argsTrace)/sizeof(argsTrace[0]))); \
  } \
}
#else /* _TRACE */
#  define SHOWTRACE(p)
#endif /* _TRACE */


/***********************************************************************
 *
 *  Entry Points
 *
 ***********************************************************************/
#ifdef __PALMOS__
tULong PNOMain(const tVoid* emulStateP,
               tVoid* userData68KP,
               Call68KFuncType* call68KFuncP)
#else
tULong Engine_DiskAutoStart(tDiskOperation* DiskOperation)
#endif
/***********************************************************************
 *
 *  PNOMain
 *
 ***********************************************************************/
{
#ifdef __PALMOS__
tDiskOperation* DiskOperation = (tDiskOperation*)userData68KP;
#endif
#if defined(_TRACE)
tNativeCPC* NativeCPC;
#endif /* _TRACE */
tDrive* driveP;
tTrack* expectedCatalogTrackP;
tTrack* effectiveCatalogTrackP;
tSector* sectorP;
tChar* CommandP;
tUChar* firstValidEntryP;
tULong nbValidEntries;
tULong detectCPM;
tULong loop;
tULong nbCharOfCommand;

#ifdef __PALMOS__
  //
  // Palm OS 68K interface
  //
  // Disk content read by Native_DiskLoad
  driveP = (tDrive*)EndianSwap32(DiskOperation->Drive);
#if defined(_TRACE)
  NativeCPC = (tNativeCPC*)EndianSwap32(DiskOperation->NativeCPC);
#endif /* _TRACE */
  // Pointer on autostart command
  CommandP = (tChar*)EndianSwap32(DiskOperation->Param);
#else
  driveP = DiskOperation->Drive;
// Pointer on autostart command
  CommandP = DiskOperation->Param;
#endif

  SHOWTRACE(TRACE_DATA(TRACE_FN_AutoStart, 0, 0));

  // Default autostart command is NULL
  *CommandP = 0;
  detectCPM = 0;
  //
  // Read disk format
  //
  sectorP = &driveP->track[0][0].sector[0]; // First sector pointer

  SHOWTRACE(TRACE_DATA(TRACE_FN_AutoStart, 1, sectorP->CHRN[SECTOR_CHRN_R]));

  switch (sectorP->CHRN[SECTOR_CHRN_R] & 0xC0)
  {
    case 0xC0: // DATA format
    {
  	  // For DATA format, Catalogue is expected in Track 0 Side 0.
  	  expectedCatalogTrackP = &driveP->track[0][0];
  	  detectCPM = 0;
    }
    break;

    case 0x40: // VENDOR format
    {
  	  // For VENDOR format, Catalogue is expected in Track 2 Side 0.
  	  expectedCatalogTrackP = &driveP->track[2][0];
  	  detectCPM = 1;
    }
    break;

    case 0x00: // IBM format
    {
  	  // For IBM format, Catalogue is expected in Track 1 Side 0.
  	  expectedCatalogTrackP = &driveP->track[1][0];
  	  detectCPM = 0;
    }
    break;

    default:
    {
  	  return DiskFormatUnknown;
    }
  }

  SHOWTRACE(TRACE_DATA(TRACE_FN_AutoStart, 2, 0));

  //
  // Find and Parse Catalog
  //
  nbValidEntries = DetectCatalog(driveP,
                                 &effectiveCatalogTrackP,
                                 &firstValidEntryP
#if defined(_TRACE)
                                 ,
                                 NativeCPC
#endif /* _TRACE */
                                 );

#ifdef _TRACE
  // Debug purpose ONLY
  if (nbValidEntries > 100)
  {
  	return nbValidEntries;
  }
#endif /* _TRACE */

  SHOWTRACE(TRACE_DATA(TRACE_FN_AutoStart, 3, nbValidEntries));

  //
  // Detect and Launch CP/M
  //
  if (detectCPM)
  {
  	tULong CPMDetected = 0;

    SHOWTRACE(TRACE_DATA(TRACE_FN_AutoStart, 4, nbValidEntries));

    // No valid entries, it should be CP/M !!
    if (!nbValidEntries)
    {
    	CPMDetected = 1;
    }
    // Catalog is not located where expected !!
    else if (effectiveCatalogTrackP != expectedCatalogTrackP)
    {
    	CPMDetected = 1;
    }

    if (CPMDetected)
    {
      SHOWTRACE(TRACE_DATA(TRACE_FN_AutoStart, 8, nbValidEntries));

      // Prepare command to execute
      CommandP[0] = '|';
      CommandP[1] = 'C';
      CommandP[2] = 'P';
      CommandP[3] = 'M';
      CommandP[4] = '\n'; // RETURN
      CommandP[5] = '\0';

      return AutoStartOK;
    }
  }

  SHOWTRACE(TRACE_DATA(TRACE_FN_AutoStart, 9, nbValidEntries));

  //
  // Try to RUN
  //
  if (!nbValidEntries)
    return NoValidFileEntry;
  if (nbValidEntries > 1)
    return TooManyFileEntries;

  SHOWTRACE(TRACE_DATA(TRACE_FN_AutoStart, 10, 0));

  // Prepare command to execute
  nbCharOfCommand = 0;
  CommandP[nbCharOfCommand++] = 'R';
  CommandP[nbCharOfCommand++] = 'U';
  CommandP[nbCharOfCommand++] = 'N';
  CommandP[nbCharOfCommand++] = '\"';

  // Add filename
  for (loop=1; loop <= 8; loop++)
  {
  	// Do not take spaces into account
  	if (*(firstValidEntryP+loop) != 0x20)
  	{
  	  CommandP[nbCharOfCommand++] = *(firstValidEntryP+loop);
  	}
  }

  /*// Add Point to separation filename and extension
  CommandP[nbCharOfCommand++] = '.';

  // Add extension
  for (loop=9; loop <= 11; loop++)
  {
  	// Do not take spaces into account
  	if (*(firstValidEntryP+loop) != 0x20)
  	{
  	  CommandP[nbCharOfCommand++] = *(firstValidEntryP+loop) & 0x7f; // Do not copy Read-only and Hidden flags
  	}
  }*/

  // End of command
  CommandP[nbCharOfCommand++] = '\n'; // RETURN
  CommandP[nbCharOfCommand] = '\0';

  SHOWTRACE(TRACE_DATA(TRACE_FN_AutoStart, 11, 0));

  return AutoStartOK;
}
/*----------------------------------------------------------------------------*/


static tULong DetectCatalog(tDrive* driveP,
                            tTrack** trackPP,
                            tUChar** firstFileEntryPP
#if defined(_TRACE)
                            ,
                            tNativeCPC* NativeCPC
#endif /* _TRACE */
                           )
/***********************************************************************
 *
 *  DetectCatalog
 *
 ***********************************************************************/
{
tTrack* actualTrackP;
tSector* sectorP;
tUChar* notVisiblefirstFileP = 0;
tTrack* notVisiblefirstFileTrackP = 0;
tULong nbValidEntries = 0;
tULong indexTrack;
tULong indexSector;
tULong loop;

  SHOWTRACE(TRACE_DATA(TRACE_FN_DetectCatalog, 0, 0));

  // Initialize caller pointer to NULL as non detection default values
  *trackPP = 0;
  *firstFileEntryPP = 0;

  //
  // Parse catalog from Track 0 to Track 2 until valid entries detected
  //
  for (indexTrack = 0; (indexTrack <= 2) && !nbValidEntries; indexTrack++)
  {
    SHOWTRACE(TRACE_DATA(TRACE_FN_DetectCatalog, 1, indexTrack));

    actualTrackP = &driveP->track[indexTrack][0]; // Side = 0
    // Do not take into account empty or missing tracks
    if (!actualTrackP->size)
      continue;

    SHOWTRACE(TRACE_DATA(TRACE_FN_DetectCatalog, 2, indexTrack));

    indexSector = 1; // Begin to find the first sector

    // Only find file entries into four first sectors.
    while (indexSector <= 4)
    {
      tULong nbofentries;
  	  tUChar* fileEntryP;
  	  tULong ValidExtension;

      SHOWTRACE(TRACE_DATA(TRACE_FN_DetectCatalog, 3, indexSector));

      //
      // Find sector
      //
      sectorP = 0;
      for (loop=0; loop < actualTrackP->sectors; loop++)
      {
    	  if ((actualTrackP->sector[loop].CHRN[SECTOR_CHRN_R] & 0x1f) == indexSector)
    	  {
          sectorP = &actualTrackP->sector[loop];
    	  }
      }

      SHOWTRACE(TRACE_DATA(TRACE_FN_DetectCatalog, 4, indexSector));

  	  // Do not take into account empty or missing sector
  	  if (!sectorP ||
  	      (!sectorP->size) )
  	  {
  	  	// Next sector
  	  	indexSector++;
  	  	continue;
  	  }

      nbofentries = sectorP->size / FILE_ENTRY_SIZE; // 32 bytes per file entry
  	  fileEntryP = sectorP->data; // Point at beginning of sector.

      SHOWTRACE(TRACE_DATA(TRACE_FN_DetectCatalog, 5, indexSector));

      for (loop=0; loop < nbofentries; loop++, fileEntryP += FILE_ENTRY_SIZE)
      {
    	  tUChar* bytePtr;

        SHOWTRACE(TRACE_DATA(TRACE_FN_DetectCatalog, 6, indexSector+loop));

    	  //
    	  // Parse File Entry
    	  //
    	  // Entry documentation
    	  // ===================
    	  // Byte 0x0 = USER; 229(0xe5) for deleted files, 0 by default.
    	  // Byte 0x1 to 0x8 = filename
    	  // Byte 0x9 to 0xb = extension. Byte 0x9 bit7 = 1 = Read Only. Byte 0xa bit7 = 1 = Hidden.
    	  // Byte 0xc to 0xe = Not used ??
    	  // Byte 0xf = size in records (1 record=128 bytes). 0x80 for full entry => check another entry
    	  // Byte 0x10 to 0x1f = list of numbers of 1k blocks (2 sectors length).

    	  // How to location a block
    	  // =======================
    	  // Cylinder (C) = INT(block * 2 / 9); 9 sectors per track !! For EXTENDED, check number of sectors per Tracks
    	  // Head     (H) = Always 0
    	  // Sector   (R) = (block * 2) MODULO 9 + 1 + TYPE (0xC0 for DATA, 0x40 for VENDOR)
    	  // Size     (N) = Size (x256 bytes)

    	  // USER must be 0.
    	  if (fileEntryP[0x0] != 0)
    	    continue;

        SHOWTRACE(TRACE_DATA(TRACE_FN_DetectCatalog, 7, indexSector+loop));

        SHOWTRACE(TRACE_DATA(TRACE_FN_DetectCatalog, 8, indexSector+loop));

        // Entry size is non null
        if (!fileEntryP[0xf])
          continue;

        SHOWTRACE(TRACE_DATA(TRACE_FN_DetectCatalog, 9, indexSector+loop));

    	  // Check filename and extension
    	  bytePtr = &fileEntryP[0x1]; // Point at the beginning of the filename.
    	  while ( ((*bytePtr & 0x7f) >= 0x20 /*SPACE*/) &&
    	          ((*bytePtr & 0x7f) <= 0x5a /*Z*/)) bytePtr++; // Loop until non printable character
        if (bytePtr < fileEntryP+0xc)
          continue;

        SHOWTRACE(TRACE_DATA(TRACE_FN_DetectCatalog, 10, indexSector+loop));

        // Check executable extension
        ValidExtension = 0;
        // "   " Extension
        if ( ((fileEntryP[0x9] & 0x7f) == ' ') &&
             ((fileEntryP[0xa] & 0x7f) == ' ') &&
             ((fileEntryP[0xb] & 0x7f) == ' ') )
        {
        	ValidExtension = 1;
        }
        // "BAS" Extension
        else if ( ((fileEntryP[0x9] & 0x7f) == 'B') &&
                  ((fileEntryP[0xa] & 0x7f) == 'A') &&
                  ((fileEntryP[0xb] & 0x7f) == 'S') )
        {
        	ValidExtension = 1;
        }
        // "BIN" Extension
        else if ( ((fileEntryP[0x9] & 0x7f) == 'B') &&
                  ((fileEntryP[0xa] & 0x7f) == 'I') &&
                  ((fileEntryP[0xb] & 0x7f) == 'N') )
        {
        	ValidExtension = 1;
        }
        if (!ValidExtension)
          continue;

        SHOWTRACE(TRACE_DATA(TRACE_FN_DetectCatalog, 11, indexSector+loop));

    	  // File is not visible.
    	  if (fileEntryP[0xa] & 0x80)
    	  {
    	    if (!notVisiblefirstFileP)
    	    {
    	      notVisiblefirstFileP = fileEntryP;
    	      notVisiblefirstFileTrackP = actualTrackP;
    	    }
    	    continue;
    	  }

        SHOWTRACE(TRACE_DATA(TRACE_FN_DetectCatalog, 12, indexSector+loop));

        //
        // Entry is valid
        //
    		// ONLY store location of first valid entry.
    		if (!*firstFileEntryPP)
    		{
    	    *trackPP = actualTrackP;
    		  *firstFileEntryPP = fileEntryP;

    		  nbValidEntries++;
    		}
    		else // Check possible other valid entries
    		{
    		  // Compare filename and extension to the first valid entry
    		  for (loop=0x1; loop < 0xc; loop++)
    		  {
    		    if (fileEntryP[loop] != (*firstFileEntryPP)[loop])
    		      break;
    		  }

    		  // Filename is different, increment valid entries count
    		  if ( (loop < 0xc) ||
    		       ((*firstFileEntryPP)[0xf] != 0x80) )
    		  {
    		    nbValidEntries++;
    		  }
    		}
      }

      indexSector++; // Next sector
    }
  }

  // No valid entry found
  if (!nbValidEntries)
  {
    // If not visible file found
    if (notVisiblefirstFileP)
    {
    	*firstFileEntryPP = notVisiblefirstFileP;
    	*trackPP = notVisiblefirstFileTrackP;

      nbValidEntries++;
    }
  }

  return nbValidEntries;
}


#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

/*
    CaPriCe for Palm OS - Amstrad CPC 464/664/6128 emulator for Palm devices
    Copyright (C) 2006-2011 by Fr�d�ric Coste
    CaPriCe Forever - Amstrad CPC 464/664/6128 emulator
    Copyright (C) 2015-2018 by Fr�d�ric Coste

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

#if defined(__PALMOS__)
#include <PceNativeCall.h>
#include <ByteOrderUtils.h>
#endif /* __PALMOS__ */


#include "Native_CPC.h"



/*
** Make sure we can call this stuff from C++.
*/
#ifdef __cplusplus
extern "C" {
#endif


//===================
// PATCH begin
#ifdef _PATCH_ENABLE

#endif /* _PATCH_ENABLE */
// PATCH end
//===================



#define FILE_ENTRY_SIZE                32


#if defined(__PALMOS__)
tULong PNOMain(const tVoid*,
               tVoid*,
               Call68KFuncType*);
#endif /* __PALMOS__ */



/***********************************************************************
 *
 *  Entry Points
 *
 ***********************************************************************/

#if defined(__PALMOS__)
tULong PNOMain(const tVoid* emulStateP,
               tVoid* userData68KP,
               Call68KFuncType* call68KFuncP)
#else
tVoid Engine_DiskReadCatalogue(tDiskOperation* DiskOperation)
#endif /* __PALMOS__ || __WIN32__ */
/***********************************************************************
 *
 *  PNOMain
 *
 ***********************************************************************/
{
#if defined(__PALMOS__)
tDiskOperation* DiskOperation = (tDiskOperation*)userData68KP;
#endif /* __PALMOS__ */
#if defined(_TRACE)
tNativeCPC* NativeCPC;
#endif /* _TRACE */
tDrive* driveP;
tTrack* actualTrackP;
tSector* sectorP;
tULong indexTrack;
tULong indexSector;
tULong loopSector;
tULong loopEntry;
tULong loopCatalogEntry;
tULong loopChar;
tULong detectCPM;

#if defined(__PALMOS__)

  //
  // Palm OS 68K interface
  //
  // Disk content read by Native_DiskLoad
  driveP = (tDrive*)EndianSwap32(DiskOperation->Drive);

#else /* __PALMOS__ */

  driveP = DiskOperation->Drive;

#endif /* __PALMOS__ */

  // Reset the number of entries
  DiskOperation->NbCatalogueEntries = 0;

  //
  // Read disk format
  //
  sectorP = &driveP->track[0][0].sector[0]; // First sector pointer
  detectCPM = 0;

  switch (sectorP->CHRN[SECTOR_CHRN_R] & 0xC0)
  {
    case 0xC0: // DATA format
    {
      detectCPM = 0;
    }
    break;

    case 0x40: // VENDOR format
    {
      detectCPM = 1;
    }
    break;

    case 0x00: // IBM format
    {
      detectCPM = 0;
    }
    break;
  }

  //
  // Parse catalog from Track 0 to Track 2 until valid entries detected
  //

  for (indexTrack = 0;
       (indexTrack <= 2) && !DiskOperation->NbCatalogueEntries;
       indexTrack++)
  {
    actualTrackP = &driveP->track[indexTrack][0]; // Side = 0

    // Do not take into account empty or missing tracks
    if (!actualTrackP->size)
      continue;

    // Only find file entries into four first sectors.
    indexSector = 1; // Begin to find the first sector
    while (indexSector <= 4)
    {
      tULong nbofentries;
  	  tUChar* fileEntryP;

      //
      // Find sector
      //
      sectorP = 0;
      for (loopSector=0; loopSector < actualTrackP->sectors; loopSector++)
      {
    	  if ( ((actualTrackP->sector[loopSector].CHRN[SECTOR_CHRN_R] & 0x0FU) == indexSector)
             && (actualTrackP->sector[loopSector].CHRN[SECTOR_CHRN_N] == 2) )
    	  {
          sectorP = &actualTrackP->sector[loopSector];
          break;
    	  }
      }

  	  // Do not take into account empty or missing sector
  	  if (!sectorP || (!sectorP->size) )
  	  {
  	  	// Next sector
  	  	indexSector++;
  	  	continue;
  	  }

      // Prepare sector read
      nbofentries = sectorP->size / FILE_ENTRY_SIZE; // 32 bytes per file entry
  	  fileEntryP = sectorP->data; // Point at beginning of sector.

      for (loopEntry=0; loopEntry < nbofentries; loopEntry++, fileEntryP += FILE_ENTRY_SIZE)
      {
        tUChar* bytePtr;
        tBool FilenameFound;

        //
        // Parse File Entry
        //
        // Entry documentation
        // ===================
        // Byte 0x0 = USER; 0-15, 229(0xe5) for deleted files, 0 by default.
        // Byte 0x1 to 0x8 = filename
        // Byte 0x9 to 0xb = extension. Byte 0x9 bit7 = 1 = Read Only. Byte 0xa bit7 = 1 = Hidden.
        // Byte 0xc = entry index (First entry = 0)
        // Byte 0xd to 0xe = Not used ??
        // Byte 0xf = size in records (1 record=128 bytes). 0x80 for full entry => check another entry
        // Byte 0x10 to 0x1f = list of numbers of 1k blocks (2 sectors length).

        // How to locate a block
        // =====================
        // Cylinder (C) = INT(block * 2 / 9); 9 sectors per track !! For EXTENDED, check number of sectors per Tracks
        // Head     (H) = Always 0
        // Sector   (R) = (block * 2) MODULO 9 + 1 + TYPE (0xC0 for DATA, 0x40 for VENDOR)
        // Size     (N) = Size (*256 bytes)

        // Entry size is non null
        if (!fileEntryP[0xf])
          continue;

        // Check USER
        if ( (fileEntryP[0x0] > 15) && (fileEntryP[0x0] != 0xe5) )
          continue;

        // Check ZERO bytes
        if ( (fileEntryP[0xd] != 0) || (fileEntryP[0xe] != 0) )
          continue;

        // Check filename
        bytePtr = &fileEntryP[0x1]; // Point at the beginning of the filename.
        while (    (*bytePtr >= 0x20 /*SPACE*/)
                && (*bytePtr <= 0x5a /*Z*/)
                && (bytePtr < (fileEntryP+0x9)) )
        {
          bytePtr++; // Loop until non printable character
        }
        if (bytePtr < (fileEntryP+0x9))
          continue;

        // Check extension
        bytePtr = &fileEntryP[0x9]; // Point at the beginning of the filename.
        while (    ((*bytePtr & 0x7f) >= 0x20 /*SPACE*/)
                && ((*bytePtr & 0x7f) <= 0x5a /*Z*/)
                && (bytePtr < (fileEntryP+0xc)) )
        {
          bytePtr++; // Loop until non printable character
        }
        if (bytePtr < (fileEntryP+0xc))
          continue;

        // Check filename with valid letters/figures
        bytePtr = &fileEntryP[0x1]; // Point at the beginning of the filename.
        while (bytePtr <= (fileEntryP+0xc))
        {
          // Figure ?
          if ( ((*bytePtr & 0x7f) >= 0x30 /*0*/) && ((*bytePtr & 0x7f) <= 0x39 /*9*/) ) break;
          // Letter ?
          if ( ((*bytePtr & 0x7f) >= 0x41 /*A*/) && ((*bytePtr & 0x7f) <= 0x5a /*Z*/) ) break;
          bytePtr++;
        }
        // No valid characters ?
        if (bytePtr > (fileEntryP + 0xc))
          continue;

        // Keep Catalog track
        DiskOperation->CatalogueTrack = indexTrack;

        //
        // Entry is valid
        //
        FilenameFound = cFalse;
        // Search already stored filename
        for (loopCatalogEntry=0;
             loopCatalogEntry < DiskOperation->NbCatalogueEntries;
             loopCatalogEntry++)
        {
    		  // Compare filename and extension
  	      tUChar* fileStoredEntryP = DiskOperation->CatalogueEntries[loopCatalogEntry];
    	    bytePtr = &fileEntryP[0x1]; // Point at the beginning of the filename.
    		  for (loopChar=0; loopChar < 11; loopChar++)
    		  {
    		    if ((*bytePtr++ & 0x7f) != *fileStoredEntryP++)
    		      break;
    		  }
          // Matching filename ?
          // AND Matching USER
          if ( (loopChar == 11)
               && (fileEntryP[0x0] == DiskOperation->CatalogueEntryUser[loopCatalogEntry]) )
          {
            tUChar loopBlocks;
            tUChar numberBlocks = 0;

            // Get number of blocks
            for (loopBlocks=0; loopBlocks < 16; loopBlocks++)
            {
              if (fileEntryP[0x10+loopBlocks])
              {
                numberBlocks++;
              }
            }

            FilenameFound = cTrue;

            // Update file size
            DiskOperation->CatalogueEntrySize[loopCatalogEntry] += numberBlocks * 1024;
            break;
          }
        }

        // Add new entry
        if (FilenameFound == cFalse)
        {
          tUChar* fileStoredEntryP = DiskOperation->CatalogueEntries[DiskOperation->NbCatalogueEntries];
          tUChar loopBlocks;
          tUChar numberBlocks = 0;

          // Add filename + extension
    	    bytePtr = &fileEntryP[0x1]; // Point at the beginning of the filename.
          for (loopChar=0; loopChar < 11; loopChar++)
          {
            *fileStoredEntryP++ = *bytePtr++ & 0x7f;  // Do not copy Read-only and Hidden flags
          }

          // Get number of blocks
          for (loopBlocks=0; loopBlocks < 16; loopBlocks++)
          {
            if (fileEntryP[0x10+loopBlocks])
            {
              numberBlocks++;
            }
          }

          // Add file info
          DiskOperation->CatalogueEntrySize[DiskOperation->NbCatalogueEntries] = numberBlocks * 1024;
          DiskOperation->CatalogueEntryUser[DiskOperation->NbCatalogueEntries] = fileEntryP[0x0];

          // Update number of entries
          DiskOperation->NbCatalogueEntries++;
        }
      }

      indexSector++; // Next sector
    }
  }
  if (detectCPM & (!DiskOperation->NbCatalogueEntries))
  {
    DiskOperation->CatalogueEntries[0][0] = '|';
    DiskOperation->CatalogueEntries[0][1] = 'C';
    DiskOperation->CatalogueEntries[0][2] = 'P';
    DiskOperation->CatalogueEntries[0][3] = 'M';
    DiskOperation->CatalogueEntries[0][4] = '\0';
    DiskOperation->CatalogueEntrySize[0] = 0;
    DiskOperation->CatalogueEntryUser[0] = 0;
    DiskOperation->NbCatalogueEntries++;
  }
}
/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

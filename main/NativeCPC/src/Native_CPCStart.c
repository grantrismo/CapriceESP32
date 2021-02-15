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

#ifdef __PALMOS__
#include <PceNativeCall.h>
#include <ByteOrderUtils.h>
#endif

#include "types.h"
#include "Native_CPC.h"
#include "Sound.h"

#include "amsdos.h"
#include "parados.h"
#include "cpc464.h"
#include "cpc664.h"
#include "cpc6128.h"

/*
** Make sure we can call this stuff from C++.
*/
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __PALMOS__
tULong PNOMain(const tVoid*,
               tVoid*,
               Call68KFuncType*);
#endif

// copied from CoreTraps.h
#define sysTrapMemHandleLock          0xA021
#define sysTrapDmGetResource          0xA05F
#define sysTrapFtrPtrNew              0xA35A

// copied from VFSMgr.h
#define vfsTrapFileRead               6


// copied from ErrorBase.h
#define errNone                       0x0000  // No error
#define memErrorClass                 0x0100  // Memory Manager
#define sysErrorClass                 0x0500  // System Manager
// copied from MemoryMgr.h
#define memErrNotEnoughSpace          (memErrorClass | 2)
// copied from SystemMgr.h
#define sysErrNoFreeResource          (sysErrorClass | 3)

#ifdef __PALMOS__
// copied from DataMgr.h
typedef tULong  DmResType;
typedef tUShort DmResID;
#endif

static void CPCInit(tNativeCPC* NativeCPC);
static void Z80Init(tNativeCPC* NativeCPC);
static void CRTCInit(tNativeCPC* NativeCPC);
static void FDCInit(tFDC* FDC);
static void ga_init_banking(tNativeCPC* NativeCPC);
static void audio_init(tNativeCPC* NativeCPC);

#ifdef ENABLE_TAPE
static void TapeInit(tNativeCPC* NativeCPC);
#endif /* ENABLE_TAPE */

//
// Palm OS routines
//
#ifdef __PALMOS__
static tVoid* DmGetResource(DmResType type,
                            DmResID resID,
                            const tVoid* emulStateP,
                            Call68KFuncType* call68KFuncP);
static tVoid* MemHandleLock(tVoid* h,
                            const tVoid* emulStateP,
                            Call68KFuncType* call68KFuncP);
#endif
//
// TestU
//
//#define _TESTU
#ifdef _TESTU
static tUShort PerformTestU(tNativeCPC* NativeCPC);
#endif /* _TESTU */


/***********************************************************************
 *
 *  Entry Points
 *
 ***********************************************************************/

#ifdef _TRACE
#define SHOWTRACE(p) \
{ \
tUChar argsTrace[4]; \
  *((tULong*)(&(argsTrace[0]))) = EndianSwap32(p); \
  NativeCPC->call68KFuncP(NativeCPC->emulStateP, \
                          NativeCPC->TraceAlertPtr, \
                          &argsTrace, \
                          4); \
}
#endif /* _TRACE */

#ifdef __PALMOS__
tULong PNOMain(const tVoid* emulStateP,
               tVoid* userData68KP,
               Call68KFuncType* call68KFuncP)
#else
tULong Engine_CPCStart(tNativeCPC* NativeCPC)
#endif
/***********************************************************************
 *
 *  PNOMain
 *
 ***********************************************************************/
{
#ifdef __PALMOS__
tNativeCPC* NativeCPC;
DmResID resID;
#endif

#ifdef __PALMOS__
  NativeCPC = (tNativeCPC*)userData68KP;

  // Palm OS 68K interface
  NativeCPC->emulStateP = emulStateP;
  NativeCPC->call68KFuncP = call68KFuncP;
  NativeCPC->prefP = (tPreferences*)EndianSwap32(NativeCPC->prefP);
  NativeCPC->contextP = (tUChar*)EndianSwap32(NativeCPC->contextP);
  NativeCPC->BmpOffScreenBits = (tVoid*)EndianSwap32(NativeCPC->BmpOffScreenBits);
  NativeCPC->BmpOffScreenBytesRow = EndianSwap16(NativeCPC->BmpOffScreenBytesRow);
  NativeCPC->TraceAlertPtr = EndianSwap32(NativeCPC->TraceAlertPtr);
  NativeCPC->MemPtrNewPtr = EndianSwap32(NativeCPC->MemPtrNewPtr);
  NativeCPC->WinPalettePtr = EndianSwap32(NativeCPC->WinPalettePtr);
  NativeCPC->SoundCalculateLevelTablesPtr = EndianSwap32(NativeCPC->SoundCalculateLevelTablesPtr);
  NativeCPC->colours_rgb = (colours_rgb_entry*)EndianSwap32(NativeCPC->colours_rgb);
  NativeCPC->colours_green = (colours_rgb_entry*)EndianSwap32(NativeCPC->colours_green);
  NativeCPC->FDCCommandTable = (fdc_cmd_table_def*)EndianSwap32(NativeCPC->FDCCommandTable);
  NativeCPC->DAATable = (tUShort*)EndianSwap32(NativeCPC->DAATable);
#endif

#ifdef __NEWMEMLAYOUT__
  printf("ENGINE CPC START - __NEWMEMLAYOUT__\n");
#else
  printf("ENGINE CPC START\n");
#endif
  NativeCPC->paused = 0;
  NativeCPC->FirstInitToPerform = 1;
  //
  // CPC structure
  //
  CPCInit(NativeCPC);

  //
  // Z80 structure
  //

  #ifndef __NEWMEMLAYOUT__
  NativeCPC->Z80 = (tZ80*)(NativeCPC->contextP + CONTEXT_OFFSET_Z80);
  NativeCPC->Z80->SZ = (tUChar*)(NativeCPC->contextP + CONTEXT_OFFSET_Z80_SZ);
  NativeCPC->Z80->SZ_BIT = (tUChar*)(NativeCPC->contextP + CONTEXT_OFFSET_Z80_SZ_BIT);
  NativeCPC->Z80->SZP = (tUChar*)(NativeCPC->contextP + CONTEXT_OFFSET_Z80_SZP);
  NativeCPC->Z80->SZHV_inc = (tUChar*)(NativeCPC->contextP + CONTEXT_OFFSET_Z80_SZHV_inc);
  NativeCPC->Z80->SZHV_dec = (tUChar*)(NativeCPC->contextP + CONTEXT_OFFSET_Z80_SZHV_dec);
  #endif

  Z80Init(NativeCPC);

  //
  // Drives structure
  //
  NativeCPC->DriveA = (tDrive*)(NativeCPC->contextP + CONTEXT_OFFSET_DRIVE_A);
  NativeCPC->DriveB = (tDrive*)(NativeCPC->contextP + CONTEXT_OFFSET_DRIVE_B);

  //
  // CRTC structure
  //
  #ifndef __NEWMEMLAYOUT__
  NativeCPC->CRTC = (tCRTC*)(NativeCPC->contextP + CONTEXT_OFFSET_CRTC);
  NativeCPC->mode0_table = (tUChar*)(NativeCPC->contextP + CONTEXT_OFFSET_MODE0_TABLE);
  NativeCPC->mode1_table = (tUChar*)(NativeCPC->contextP + CONTEXT_OFFSET_MODE1_TABLE);
  #endif
  CRTCInit(NativeCPC);


  //
  // GateArray structure
  //
  #ifndef __NEWMEMLAYOUT__
  NativeCPC->GateArray = (tGateArray*)(NativeCPC->contextP + CONTEXT_OFFSET_GATEARRAY);
  #endif

  //
  // FDC structure
  //
  NativeCPC->FDC = (tFDC*)(NativeCPC->contextP + CONTEXT_OFFSET_FDC);
  FDCInit(NativeCPC->FDC);


  //
  // PPI structure
  //
  #ifndef __NEWMEMLAYOUT__
  NativeCPC->PPI = (tPPI*)(NativeCPC->contextP + CONTEXT_OFFSET_PPI);
  #endif

  //
  // Audio
  //
  audio_init(NativeCPC);

  //
  // VDU structure
  //
  #ifndef __NEWMEMLAYOUT__
  NativeCPC->VDU = (tVDU*)(NativeCPC->contextP + CONTEXT_OFFSET_VDU);
  #endif

  //
  // Allocation de la m�moire
  //
  NativeCPC->pbGPBuffer = (tUChar*)(NativeCPC->contextP + CONTEXT_OFFSET_GPBUFFER);
  NativeCPC->pbRAM = (tUChar*)(NativeCPC->contextP + CONTEXT_OFFSET_RAM);
  NativeCPC->memmap_ROM = (tUChar**)(NativeCPC->contextP + CONTEXT_OFFSET_MEMMAP_ROM);

  //
  // Mapping m�moire
  //
  // init the CPC memory banking map
  PROFILE_ADD_NATIVE(PROFILE_ga_init_banking);
  ga_init_banking(NativeCPC);

  //
  // Charger les ROMs
  //
  // CPC ROM en ROMlo
  if (NativeCPC->prefP->CPCModel == CPC_MODEL_664)
  {
    NativeCPC->hMemROMlo = (void*)&cpc664Rom;
  }
  else if (NativeCPC->prefP->CPCModel == CPC_MODEL_6128)
  {
    NativeCPC->hMemROMlo = (void*)&cpc6128Rom;
  }
  else /* CPC_MODEL_464 */
  {
    NativeCPC->hMemROMlo = (void*)&cpc464Rom;
  }

  NativeCPC->pbROMlo = (tUChar*)NativeCPC->hMemROMlo;
  NativeCPC->pbROMhi = NativeCPC->pbExpansionROM = NativeCPC->pbROMlo + 16384;

  // AMSDOS � l'emplacement 7
  if (NativeCPC->prefP->UseParados == 0)
  {
    NativeCPC->hMemAMSDOS = (void*)&amsdosRom;
  }
  else // Use PARADOS
  {
    NativeCPC->hMemAMSDOS = (void*)&paradosRom;
  }
  NativeCPC->memmap_ROM[7] = (tUChar*)NativeCPC->hMemAMSDOS;

#ifdef ENABLE_TAPE
  TapeInit(NativeCPC);
#endif /* ENABLE_TAPE */


#ifdef _TESTU
  return PerformTestU(NativeCPC);
#else /* _TESTU */
  return errNone;
#endif /* _TESTU */
}
/*----------------------------------------------------------------------------*/



static void CPCInit(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  CPCInit
 *
 ***********************************************************************/
{
tPreferences* prefP = NativeCPC->prefP;

  NativeCPC->jumpers = prefP->CPCJumpers;

  if ( (prefP->CPCModel == CPC_MODEL_6128) || (prefP->Use64kMemoryExtension) )
  {
    NativeCPC->ram_size = 128;
  }
  else
  {
    NativeCPC->ram_size = 64;
  }

  NativeCPC->printer = 0;

  // Prepare color tables (meed to be emabled, taken  from CPF)
  //PrepareGreenColoursTable();
  //NativeCPC->colours_rgb = colours_rgb;
  //NativeCPC->colours_green = green_rgb;

  NativeCPC->scr_tube = prefP->ScreenType; // 0 = Color, 1 = Green
  NativeCPC->scr_intensity = prefP->ScreenIntensity;

  NativeCPC->night_mode = prefP->NightModeActive;
}
/*----------------------------------------------------------------------------*/


static void Z80Init(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  Z80Init
 *
 ***********************************************************************/
{
tZ80* Z80 = NativeCPC->Z80;
tUShort i, p;

  for (i = 0; i < 256; i++)
  {
    p = 0;

    if (i & 0x01) ++p;
    if (i & 0x02) ++p;
    if (i & 0x04) ++p;
    if (i & 0x08) ++p;
    if (i & 0x10) ++p;
    if (i & 0x20) ++p;
    if (i & 0x40) ++p;
    if (i & 0x80) ++p;

    Z80->SZ[i] = i ? i & Sflag : Zflag;
    Z80->SZ[i] |= (i & Xflags);
    Z80->SZ_BIT[i] = i ? i & Sflag : Zflag | Pflag;
    Z80->SZ_BIT[i] |= (i & Xflags);
    Z80->SZP[i] = Z80->SZ[i] | ((p & 1) ? 0 : Pflag);

    Z80->SZHV_inc[i] = Z80->SZ[i];

    if (i == 0x80)
    {
    	Z80->SZHV_inc[i] |= Vflag;
    }
    if ((i & 0x0f) == 0x00)
    {
    	Z80->SZHV_inc[i] |= Hflag;
    }

    Z80->SZHV_dec[i] = Z80->SZ[i] | Nflag;

    if (i == 0x7f)
    {
    	Z80->SZHV_dec[i] |= Vflag;
    }
    if ((i & 0x0f) == 0x0f)
    {
    	Z80->SZHV_dec[i] |= Hflag;
    }
  }
}
/*----------------------------------------------------------------------------*/


static void CRTCInit(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  CRTCInit
 *
 ***********************************************************************/
{
tUShort idx, n;
tUChar p1, p2, p3, p4;

  // Tables initialisation
  idx = 0;
  for (n = 0; n < 256; n++) // calculate mode0 byte-to-pixel translation table
  {
    p1 = ((n & 0x80) >> 7) + ((n & 0x08) >> 2) + ((n & 0x20) >> 3) + ((n & 0x02) << 2);
    p2 = ((n & 0x40) >> 6) + ((n & 0x04) >> 1) + ((n & 0x10) >> 2) + ((n & 0x01) << 3);
    NativeCPC->mode0_table[idx++] = p1;
    NativeCPC->mode0_table[idx++] = p2;
  }

  idx = 0;
  for (n = 0; n < 256; n++) // calculate mode1 byte-to-pixel translation table
  {
    p1 = ((n & 0x80) >> 7) + ((n & 0x08) >> 2);
    p2 = ((n & 0x40) >> 6) + ((n & 0x04) >> 1);
    p3 = ((n & 0x20) >> 5) +  (n & 0x02);
    p4 = ((n & 0x10) >> 4) + ((n & 0x01) << 1);

    NativeCPC->mode1_table[idx++] = p1;
    NativeCPC->mode1_table[idx++] = p2;
    NativeCPC->mode1_table[idx++] = p3;
    NativeCPC->mode1_table[idx++] = p4;
  }
}
/*----------------------------------------------------------------------------*/


static void FDCInit(tFDC* FDC)
/***********************************************************************
 *
 *  FDCInit
 *
 ***********************************************************************/
{
  FDC->read_status_delay = 0;
}
/*----------------------------------------------------------------------------*/


static void ga_init_banking(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  ga_init_banking
 *
 ***********************************************************************/
{
tUChar *romb0, *romb1, *romb2, *romb3, *romb4, *romb5, *romb6, *romb7;
tUChar *pbRAMbank;

  romb0 = NativeCPC->pbRAM;
  romb1 = NativeCPC->pbRAM + ((tULong)1*16384);
  romb2 = NativeCPC->pbRAM + ((tULong)2*16384);
  romb3 = NativeCPC->pbRAM + ((tULong)3*16384);

  pbRAMbank = NativeCPC->pbRAM + ((NativeCPC->GateArray->RAM_bank + 1) * 65536);
  romb4 = pbRAMbank;
  romb5 = pbRAMbank + ((tULong)1*16384);
  romb6 = pbRAMbank + ((tULong)2*16384);
  romb7 = pbRAMbank + ((tULong)3*16384);

  NativeCPC->membank_config[0][0] = romb0;
  NativeCPC->membank_config[0][1] = romb1;
  NativeCPC->membank_config[0][2] = romb2;
  NativeCPC->membank_config[0][3] = romb3;

  NativeCPC->membank_config[1][0] = romb0;
  NativeCPC->membank_config[1][1] = romb1;
  NativeCPC->membank_config[1][2] = romb2;
  NativeCPC->membank_config[1][3] = romb7;

  NativeCPC->membank_config[2][0] = romb4;
  NativeCPC->membank_config[2][1] = romb5;
  NativeCPC->membank_config[2][2] = romb6;
  NativeCPC->membank_config[2][3] = romb7;

  NativeCPC->membank_config[3][0] = romb0;
  NativeCPC->membank_config[3][1] = romb3;
  NativeCPC->membank_config[3][2] = romb2;
  NativeCPC->membank_config[3][3] = romb7;

  NativeCPC->membank_config[4][0] = romb0;
  NativeCPC->membank_config[4][1] = romb4;
  NativeCPC->membank_config[4][2] = romb2;
  NativeCPC->membank_config[4][3] = romb3;

  NativeCPC->membank_config[5][0] = romb0;
  NativeCPC->membank_config[5][1] = romb5;
  NativeCPC->membank_config[5][2] = romb2;
  NativeCPC->membank_config[5][3] = romb3;

  NativeCPC->membank_config[6][0] = romb0;
  NativeCPC->membank_config[6][1] = romb6;
  NativeCPC->membank_config[6][2] = romb2;
  NativeCPC->membank_config[6][3] = romb3;

  NativeCPC->membank_config[7][0] = romb0;
  NativeCPC->membank_config[7][1] = romb7;
  NativeCPC->membank_config[7][2] = romb2;
  NativeCPC->membank_config[7][3] = romb3;
}
/*----------------------------------------------------------------------------*/


static void audio_init(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  audio_init
 *
 ***********************************************************************/
{

  printf("audio_init_entered\n");
#ifndef __NEWMEMLAYOUT__
  NativeCPC->PSG = (tPSG*)(NativeCPC->contextP + CONTEXT_OFFSET_PSG);
#endif

  tPSG* PSG = NativeCPC->PSG;
  tPreferences* prefP = NativeCPC->prefP;

#ifdef __PALMOS__
  tUChar args[4];
#endif

  PSG->Level_PP = (tLong*)(NativeCPC->contextP + CONTEXT_OFFSET_PSG_LEVEL_PP);
  PSG->Level_AR = (tLong*)(NativeCPC->contextP + CONTEXT_OFFSET_PSG_LEVEL_AR);
  PSG->Level_AL = (tLong*)(NativeCPC->contextP + CONTEXT_OFFSET_PSG_LEVEL_AL);
  PSG->Level_BR = (tLong*)(NativeCPC->contextP + CONTEXT_OFFSET_PSG_LEVEL_BR);
  PSG->Level_BL = (tLong*)(NativeCPC->contextP + CONTEXT_OFFSET_PSG_LEVEL_BL);
  PSG->Level_CR = (tLong*)(NativeCPC->contextP + CONTEXT_OFFSET_PSG_LEVEL_CR);
  PSG->Level_CL = (tLong*)(NativeCPC->contextP + CONTEXT_OFFSET_PSG_LEVEL_CL);


  PSG->snd_enabled = 0; // Always disabled at start.
  PSG->snd_volume = prefP->SoundVolume;
  PSG->snd_pp_device = 0;

  // Initializes Sound buffer pointers
#ifdef __NEWMEMLAYOUT__
  PSG->pbSndBuffer = (tUChar*)SoundBufferP[0];
#else
  PSG->pbSndBuffer = (tUChar*)(NativeCPC->contextP + CONTEXT_OFFSET_SND_BUFFER);
#endif
  PSG->pbSndBufferEnd = PSG->pbSndBuffer + SND_BUFFER_SIZE - 1;
  PSG->snd_bufferptr = PSG->pbSndBuffer; // init write cursor

  PSG->FilledBufferSize = 0;

  //
  // Init AY
  //
#ifdef __PALMOS__
  *((tULong*)(&(args[0]))) = (tULong)EndianSwap32(NativeCPC);
  // Calculate_Level_Tables();
  NativeCPC->call68KFuncP(NativeCPC->emulStateP,
                          NativeCPC->SoundCalculateLevelTablesPtr,
                          &args,
                          4);
#else
   Sound_Calculate_Level_Tables(NativeCPC);
#endif
}
/*----------------------------------------------------------------------------*/


#ifdef ENABLE_TAPE

static void TapeInit(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  InitTape
 *
 ***********************************************************************/
{
  NativeCPC->dwTapeStage = TAPE_END;

  NativeCPC->pbTapeImage = cNull;
  NativeCPC->pbTapeImageEnd = cNull;
}
/*----------------------------------------------------------------------------*/

#endif /* ENABLE_TAPE */


//==============================================================================
//
// Palm OS routines
//
//==============================================================================
#ifdef __PALMOS__
static tVoid* DmGetResource(DmResType type,
                            DmResID resID,
                            const tVoid* emulStateP,
                            Call68KFuncType* call68KFuncP)
/***********************************************************************
 *
 *  DmGetResource
 *
 ***********************************************************************/
{
tUChar args[6]; // Size should be multiple of 2

  // MemHandle DmGetResource(DmResType type, DmResID resID);

  // DmGetResource Parameters
  *((tULong*)(&(args[0]))) = (tULong)EndianSwap32(type);
  *((tUShort*)(&(args[4]))) = (tUShort)EndianSwap16(resID);

  return (tVoid*)call68KFuncP(emulStateP,
                              PceNativeTrapNo(sysTrapDmGetResource),
                              &args,
                              (sizeof(args)/sizeof(args[0])) | kPceNativeWantA0);
}
/*----------------------------------------------------------------------------*/


static tVoid* MemHandleLock(tVoid* h,
                            const tVoid* emulStateP,
                            Call68KFuncType* call68KFuncP)
/***********************************************************************
 *
 *  MemHandleLock
 *
 ***********************************************************************/
{
tUChar args[4]; // Size should be multiple of 2

  // MemPtr MemHandleLock(MemHandle h);

  // MemHandleLock Parameters
  *((tULong*)(&(args[0]))) = (tULong)EndianSwap32(h);

  return (tVoid*)call68KFuncP(emulStateP,
                              PceNativeTrapNo(sysTrapMemHandleLock),
                              &args,
                              (sizeof(args)/sizeof(args[0])) | kPceNativeWantA0);
}
/*----------------------------------------------------------------------------*/
#endif /*__PALMOS__*/

//==============================================================================
//
// Unitary Tests
//
//==============================================================================
#ifdef _TESTU
// Prototypes of TestU fonctions
static tUShort TestU_DWORD_UPPER_ALIGN(tNativeCPC* NativeCPC,
                                       tUChar NoTest);
static tUShort TestU_MemoryAllocation(tNativeCPC* NativeCPC,
                                      tUChar NoTest);
#ifdef __ESP32__
static tUShort TestU_MemoryPointers(tNativeCPC* NativeCPC,
                                    tUChar NoTest);
#endif


static tUShort PerformTestU(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  PerformTestU
 *
 ***********************************************************************/
{
tUShort Result = errNone;
tUChar NoTest = 1;

  /* 1 */ if (Result == errNone) Result = TestU_DWORD_UPPER_ALIGN(NativeCPC,
                                                                  NoTest++);
  /* 2 */ if (Result == errNone) Result = TestU_MemoryAllocation(NativeCPC,
                                                                 NoTest++);
#ifdef __ESP32__
  /* 3 */ if (Result == errNone) Result = TestU_MemoryPointers(NativeCPC,
                                                               NoTest++);
#endif
  printf("Memory Test %x\n",Result);
  return (Result);
}
/*----------------------------------------------------------------------------*/


static tUShort TestU_DWORD_UPPER_ALIGN(tNativeCPC* NativeCPC,
                                       tUChar NoTest)
/***********************************************************************
 *
 *  TestU_DWORD_UPPER_ALIGN
 *
 ***********************************************************************/
{
tUShort Result = errNone;

  if (DWORD_UPPER_ALIGN(0) != 0)
  {
    Result = testUErrorClass + NoTest;
  }
  if ( (DWORD_UPPER_ALIGN(1) != 4) ||
       (DWORD_UPPER_ALIGN(3) != 4) ||
       (DWORD_UPPER_ALIGN(127) != 128) )
  {
    Result = testUErrorClass + NoTest;
  }
  if ( (DWORD_UPPER_ALIGN(64) != 64) ||
       (DWORD_UPPER_ALIGN(256) != 256) ||
       (DWORD_UPPER_ALIGN(65536) != 65536) )
  {
    Result = testUErrorClass + NoTest;
  }
  if ( (DWORD_UPPER_ALIGN(1457) != 1460) ||
       (DWORD_UPPER_ALIGN(75079) != 75080) )
  {
    Result = testUErrorClass + NoTest;
  }

  return (Result);
}
/*----------------------------------------------------------------------------*/

static tUShort TestU_MemoryAllocation(tNativeCPC* NativeCPC,
                                      tUChar NoTest)
/***********************************************************************
 *
 *  TestU_MemoryAllocation
 *
 ***********************************************************************/
{
tUShort Result = errNone;
tUChar* contexteP = NativeCPC->contextP;

  if ( ((tULong)NativeCPC != ((tULong)contexteP + CONTEXT_OFFSET_NATIVECPC)) ||
       ((tULong)NativeCPC->Z80 != ((tULong)contexteP + CONTEXT_OFFSET_Z80)) ||
       ((tULong)NativeCPC->Z80->SZ != ((tULong)contexteP + CONTEXT_OFFSET_Z80_SZ)) ||
       ((tULong)NativeCPC->Z80->SZ_BIT != ((tULong)contexteP + CONTEXT_OFFSET_Z80_SZ_BIT)) ||
       ((tULong)NativeCPC->Z80->SZP != ((tULong)contexteP + CONTEXT_OFFSET_Z80_SZP)) ||
       ((tULong)NativeCPC->Z80->SZHV_inc != ((tULong)contexteP + CONTEXT_OFFSET_Z80_SZHV_inc)) ||
       ((tULong)NativeCPC->Z80->SZHV_dec != ((tULong)contexteP + CONTEXT_OFFSET_Z80_SZHV_dec)) ||
       ((tULong)NativeCPC->DriveA != ((tULong)contexteP + CONTEXT_OFFSET_DRIVE_A)) ||
       ((tULong)NativeCPC->DriveB != ((tULong)contexteP + CONTEXT_OFFSET_DRIVE_B)) ||
       ((tULong)NativeCPC->CRTC != ((tULong)contexteP + CONTEXT_OFFSET_CRTC)) ||
       ((tULong)NativeCPC->mode0_table != ((tULong)contexteP + CONTEXT_OFFSET_MODE0_TABLE)) ||
       ((tULong)NativeCPC->mode1_table != ((tULong)contexteP + CONTEXT_OFFSET_MODE1_TABLE)) ||
       ((tULong)NativeCPC->GateArray != ((tULong)contexteP + CONTEXT_OFFSET_GATEARRAY)) ||
       ((tULong)NativeCPC->FDC != ((tULong)contexteP + CONTEXT_OFFSET_FDC)) ||
       ((tULong)NativeCPC->PPI != ((tULong)contexteP + CONTEXT_OFFSET_PPI)) ||
       ((tULong)NativeCPC->PSG != ((tULong)contexteP + CONTEXT_OFFSET_PSG)) ||
       ((tULong)NativeCPC->PSG->Level_PP != ((tULong)contexteP + CONTEXT_OFFSET_PSG_LEVEL_PP)) ||
       ((tULong)NativeCPC->PSG->Level_AR != ((tULong)contexteP + CONTEXT_OFFSET_PSG_LEVEL_AR)) ||
       ((tULong)NativeCPC->PSG->Level_AL != ((tULong)contexteP + CONTEXT_OFFSET_PSG_LEVEL_AL)) ||
       ((tULong)NativeCPC->PSG->Level_BR != ((tULong)contexteP + CONTEXT_OFFSET_PSG_LEVEL_BR)) ||
       ((tULong)NativeCPC->PSG->Level_BL != ((tULong)contexteP + CONTEXT_OFFSET_PSG_LEVEL_BL)) ||
       ((tULong)NativeCPC->PSG->Level_CR != ((tULong)contexteP + CONTEXT_OFFSET_PSG_LEVEL_CR)) ||
       ((tULong)NativeCPC->PSG->Level_CL != ((tULong)contexteP + CONTEXT_OFFSET_PSG_LEVEL_CL)) ||
       ((tULong)NativeCPC->PSG->pbSndBuffer != ((tULong)contexteP + CONTEXT_OFFSET_SND_BUFFER)) ||
       ((tULong)NativeCPC->VDU != ((tULong)contexteP + CONTEXT_OFFSET_VDU)) ||
       ((tULong)NativeCPC->pbGPBuffer != ((tULong)contexteP + CONTEXT_OFFSET_GPBUFFER)) ||
       ((tULong)NativeCPC->memmap_ROM != ((tULong)contexteP + CONTEXT_OFFSET_MEMMAP_ROM)) ||
       ((tULong)NativeCPC->pbRAM != ((tULong)contexteP + CONTEXT_OFFSET_RAM)) )
  {
    Result = testUErrorClass + NoTest;
  }

  return (Result);
}
/*----------------------------------------------------------------------------*/

#ifdef __ESP32__
static tUShort TestU_MemoryPointers(tNativeCPC* NativeCPC,
                                    tUChar NoTest)
/***********************************************************************
 *
 *  TestU_MemoryPointers
 *
 ***********************************************************************/
{
tUShort Result = errNone;

  //
  // tNativeCPC
  //
  if ( ((tULong)NativeCPC + STATIC_SIZE_NATIVECPC) !=
       (tULong)&NativeCPC->DriveA )
  {
    Result = testUErrorClass + NoTest;
  }
  if ( ((tULong)NativeCPC + STATIC_SIZE_NATIVECPC + DYNAMIC_SIZE_NATIVECPC) !=
       (tULong)&NativeCPC->dwTapePulseCycles )
  {
    Result = testUErrorClass + NoTest;
  }

  //
  // tZ80
  //
  if ( ((tULong)NativeCPC->Z80 + STATIC_SIZE_Z80) !=
       (tULong)&NativeCPC->Z80->iCycleCount )
  {
    Result = testUErrorClass + NoTest;
  }

  //
  // tPSG
  //
  if ( ((tULong)NativeCPC->PSG + STATIC_SIZE_PSG) !=
       (tULong)&NativeCPC->PSG->snd_bufferptr )
  {
    Result = testUErrorClass + NoTest;
  }
  if ( ((tULong)NativeCPC->PSG + STATIC_SIZE_PSG + DYNAMIC_SIZE_PSG) !=
       (tULong)&NativeCPC->PSG->Case_Env )
  {
    Result = testUErrorClass + NoTest;
  }

  //
  // tFDC
  //
  if ( ((tULong)NativeCPC->FDC + STATIC_SIZE_FDC) !=
       (tULong)&NativeCPC->FDC->active_drive )
  {
    Result = testUErrorClass + NoTest;
  }
  if ( ((tULong)NativeCPC->FDC + STATIC_SIZE_FDC + DYNAMIC_SIZE_FDC) !=
       (tULong)&NativeCPC->FDC->cmd_handler )
  {
    Result = testUErrorClass + NoTest;
  }

  return (Result);
}
#endif
/*----------------------------------------------------------------------------*/


#endif /* _TESTU */


#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

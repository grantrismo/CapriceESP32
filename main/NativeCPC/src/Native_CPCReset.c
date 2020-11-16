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

// Copied from Window.h
typedef tUChar IndexedColorType;      // 1, 2, 4, or 8 bit index

#include "Native_CPC.h"


//===================
// PATCH begin
#ifdef _PATCH_ENABLE

#endif /* _PATCH_ENABLE */
// PATCH end
//===================


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
#define sysTrapMemSet                 0xA027

// copied from ErrorBase.h
#define errNone                       0x0000  // No error


static tVoid Z80Reset(tNativeCPC* NativeCPC);
static tVoid CPCReset(tNativeCPC* NativeCPC);
static tVoid VDUReset(tNativeCPC* NativeCPC);
static tVoid CRTCReset(tNativeCPC* NativeCPC);
static tVoid GateArrayReset(tNativeCPC* NativeCPC);
static tVoid ga_init_banking(tNativeCPC* NativeCPC);
static tVoid PPIReset(tNativeCPC* NativeCPC);
static tVoid PSGReset(tPSG* PSG);
static tVoid FDCReset(tNativeCPC* NativeCPC);
//
// Routines
//
static tVoid MemSet(tUChar* destP,
                    tULong numBytes,
                    tUChar value);
//
// TestU
//
#ifdef _TESTU
static tUShort PerformTestU(tNativeCPC* NativeCPC);
#endif /* _TESTU */


// CRTC Reset values
static const tUChar CRTC_values[2][14] =
{
   /*            R0,   R1,   R2,   R3,   R4,   R5,   R6,   R7,   R8,   R9,  R10,  R11,  R12,  R13 */
   /* 60Hz */ {0x3f, 0x28, 0x2e, 0x8e, 0x1f, 0x06, 0x19, 0x1b, 0x00, 0x07, 0x00, 0x00, 0x30, 0x00},
   /* 50Hz */ {0x3f, 0x28, 0x2e, 0x8e, 0x26, 0x00, 0x19, 0x1e, 0x00, 0x07, 0x00, 0x00, 0x30, 0x00}
};


/***********************************************************************
 *
 *  Entry Points
 *
 ***********************************************************************/

#ifdef _TRACE
#  define SHOWTRACE(p) \
{ \
tUChar argsTrace[4]; \
  *((tULong*)(&(argsTrace[0]))) = EndianSwap32(p); \
  NativeCPC->call68KFuncP(NativeCPC->emulStateP, \
                          NativeCPC->TraceAlertPtr, \
                          &argsTrace, \
                          4); \
}
#else
#  define SHOWTRACE(p)
#endif /* _TRACE */

#ifdef __PALMOS__
tULong PNOMain(const tVoid* emulStateP,
               tVoid* userData68KP,
               Call68KFuncType* call68KFuncP)
#else
tULong Engine_CPCReset(tNativeCPC* NativeCPC)
#endif
/***********************************************************************
 *
 *  PNOMain
 *
 ***********************************************************************/
{
#ifdef __PALMOS__
tNativeCPC* NativeCPC = (tNativeCPC*)userData68KP;
#endif
tUShort Result = errNone;
tUChar n;

#ifdef __PALMOS__
  NOT_USED(emulStateP);
  NOT_USED(call68KFuncP);
#endif /* !__PALMOS__ */

#ifdef _TESTU
  Result = PerformTestU(NativeCPC);
#endif /* _TESTU */

  Z80Reset(NativeCPC);
  CPCReset(NativeCPC);
  VDUReset(NativeCPC);
  CRTCReset(NativeCPC);
  GateArrayReset(NativeCPC);
  PPIReset(NativeCPC);
  PSGReset(NativeCPC->PSG);
  FDCReset(NativeCPC);

  // clear all memory used for CPC RAM
  MemSet((tUChar*)NativeCPC->pbRAM,
         ((tULong)NativeCPC->ram_size*1024),
         0);

  for (n = 0; n < 4; n++) // initialize active read/write bank configuration
  {
    NativeCPC->membank_read[n] = NativeCPC->membank_config[0][n];
    NativeCPC->membank_write[n] = NativeCPC->membank_config[0][n];
  }
  NativeCPC->membank_read[0] = NativeCPC->pbROMlo; // 'page in' lower ROM
  NativeCPC->membank_read[3] = NativeCPC->pbROMhi; // 'page in' upper ROM

  NativeCPC->cycle_count = CYCLE_COUNT_INIT;

  NativeCPC->lightgun_beam_detect = 0;
  NativeCPC->lightgun_beam_state = 0;

  return Result;
}
/*----------------------------------------------------------------------------*/


static tVoid Z80Reset(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  Z80Reset
 *
 ***********************************************************************/
{
tZ80* Z80 = NativeCPC->Z80;

  MemSet((tUChar*)&Z80->Regs,
         sizeof(tZ80Regs),
         0); // clear result codes buffer

  _IX = _IY = 0xffff; // IX and IY are FFFF after a reset!
  _F = Zflag; // set zero flag
  Z80->Regs.breakpoint = 0xffffffff; // clear break point
}
/*----------------------------------------------------------------------------*/


static tVoid CPCReset(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  CPCReset
 *
 ***********************************************************************/
{
  // clear CPC keyboard matrix
  MemSet((tUChar*)NativeCPC->keyboard_matrix,
         sizeof(NativeCPC->keyboard_matrix),
         0xff);

  NativeCPC->tape_motor = 0;
  NativeCPC->tape_play_button = 0;
  NativeCPC->printer_port = 0xff;
}
/*----------------------------------------------------------------------------*/


static tVoid VDUReset(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  VDUReset
 *
 ***********************************************************************/
{
tVDU* VDU = NativeCPC->VDU;

  // clear VDU data structure
  MemSet((tUChar*)VDU,
         sizeof(tVDU),
         0);

  VDU->hsw = VDU->hsw_active = 4;
  VDU->scr_base = (tULong*)NativeCPC->BmpOffScreenBits; // reset to surface start
  VDU->scr_root = (tULong*)NativeCPC->BmpOffScreenBits;
  VDU->scr_line_offs = NativeCPC->BmpOffScreenBytesRow / 4;
  VDU->scr_current = VDU->scr_base;

  VDU->palette = NativeCPC->GateArray->palette;
  VDU->pbRAM = NativeCPC->pbRAM;
  VDU->mode0_table = NativeCPC->mode0_table;
  VDU->mode1_table = NativeCPC->mode1_table;
}
/*----------------------------------------------------------------------------*/


static tVoid CRTCReset(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  CRTCReset
 *
 ***********************************************************************/
{
tCRTC* CRTC = NativeCPC->CRTC;
tDrawHandlerPtr oldDrawHandler[4];
tUChar n, val1, val2;

  // Save draw handler
  for (n = 0; n < 4; n++)
  {
    oldDrawHandler[n] = CRTC->DrawHandler[n];
  }

  MemSet((tUChar*)CRTC,
         sizeof(tCRTC),
         0); // clear CRTC data structure

  for (n = 0; n < 14; n++) // program CRTC with 'valid' data
  {
    CRTC->registers[n] = CRTC_values[(NativeCPC->jumpers & 0x10) >> 4][n];
  }
  CRTC->flags = HDT_flag | VDT_flag;
  CRTC->hsw = CRTC->hsw_active = CRTC->registers[3] & 0x0f;
  CRTC->vsw = CRTC->registers[3] >> 4;
  CRTC->vt_adjust = CRTC->registers[5] & 0x1f;
  CRTC->skew = (CRTC->registers[8] >> 4) & 3;
  if (CRTC->skew == 3) // no output?
  {
    CRTC->skew = 0xff;
  }
  CRTC->max_raster = CRTC->registers[9] << 3;
  val1 = CRTC->registers[12] & 0x3f;
  val2 = val1 & 0x0f; // isolate screen size
  val1 = (val1 << 1) & 0x60; // isolate CPC RAM bank
  val2 |= val1; // combine
  CRTC->addr = CRTC->requested_addr = (CRTC->registers[13] + (val2 << 8)) << 1;
  CRTC->last_hdisp = 0x28;

  CRTC->current_mode = CRTC->requested_mode = 1; // Start in Mode 1

  // Restore draw handler
  for (n = 0; n < 4; n++)
  {
    CRTC->DrawHandler[n] = oldDrawHandler[n];
  }
}
/*----------------------------------------------------------------------------*/


static tVoid GateArrayReset(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  GateArrayReset
 *
 ***********************************************************************/
{
tGateArray* GateArray = NativeCPC->GateArray;

  // clear GA data structure
  MemSet((tUChar*)GateArray,
         sizeof(tGateArray),
         0);

  ga_init_banking(NativeCPC);
}
/*----------------------------------------------------------------------------*/


static tVoid ga_init_banking(tNativeCPC* NativeCPC)
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


static tVoid PPIReset(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  PPIReset
 *
 ***********************************************************************/
{
  // clear PPI data structure
  MemSet((tUChar*)NativeCPC->PPI,
         sizeof(tPPI),
         0);
}
/*----------------------------------------------------------------------------*/


static tVoid PSGReset(tPSG* PSG)
/***********************************************************************
 *
 *  PSGReset
 *
 ***********************************************************************/
{
  PSG->control = 0;

  PSG->Ton_Counter_A = 0;
  PSG->Ton_Counter_B = 0;
  PSG->Ton_Counter_C = 0;

  PSG->Noise_Counter = 0;

  PSG->Envelope_Counter = 0;

  PSG->Ton_A = 0;
  PSG->Ton_B = 0;
  PSG->Ton_C = 0;

  PSG->Left_Chan = 0;
  PSG->Right_Chan = 0;

  PSG->Noise.Seed = 0xffff;

  PSG->cycle_count = 0;
  PSG->LoopCount = SND_LOOP_COUNT_INIT;
}
/*----------------------------------------------------------------------------*/


static tVoid FDCReset(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  FDCReset
 *
 ***********************************************************************/
{
tFDC* FDC = NativeCPC->FDC;

  // clear FDC data structure
  MemSet((tUChar*)FDC,
         sizeof(tFDC),
         0);

  FDC->phase = CMD_PHASE;
  FDC->flags = STATUSDRVA_flag | STATUSDRVB_flag;

  FDC->FDCCommandTable = NativeCPC->FDCCommandTable;
}
/*----------------------------------------------------------------------------*/



//==============================================================================
//
// Routines
//
//==============================================================================
static tVoid MemSet(tUChar* destP,
                    tULong numBytes,
                    tUChar value)
/***********************************************************************
 *
 *  MemSet
 *
 ***********************************************************************/
{
  while (numBytes--)
  {
    *(destP++) = value;
  }
}
/*----------------------------------------------------------------------------*/


//==============================================================================
//
// Unitary Tests
//
//==============================================================================
#ifdef _TESTU
// Prototypes of TestU fonctions
static tUShort TestU_MemSet_1(tNativeCPC* NativeCPC,
                              tUChar NoTest);


static tUShort PerformTestU(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  PerformTestU
 *
 ***********************************************************************/
{
tUShort Result = errNone;
tUChar NoTest = 1;

  if (Result == errNone) Result = TestU_MemSet_1(NativeCPC,
                                                 NoTest++);

  return (Result);
}
/*----------------------------------------------------------------------------*/


static tUShort TestU_MemSet_1(tNativeCPC* NativeCPC,
                              tUChar NoTest)
/***********************************************************************
 *
 *  TestU_MemSet_1
 *
 ***********************************************************************/
{
tUChar ArrayA[100];
tUChar Loop;
tUChar Result = errNone;

  // Prepare conditions
  for (Loop=0; Loop<100; Loop++)
    ArrayA[Loop] = 0x55;
  // Perform operation
  MemSet(ArrayA, 95, 0xAA);
  // Check Result
  for (Loop=0; Loop<95; Loop++)
  {
    if (ArrayA[Loop] != 0xAA)
      Result=testUErrorClass+NoTest;
  }
  for (Loop=95; Loop<100; Loop++)
  {
    if (ArrayA[Loop] != 0x55)
      Result=testUErrorClass+NoTest;
  }

  return (Result);
}
/*----------------------------------------------------------------------------*/

#endif /* _TESTU */


#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

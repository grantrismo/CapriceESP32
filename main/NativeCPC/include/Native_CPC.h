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

#ifndef NATIVE_CPC_H
#define NATIVE_CPC_H

#ifdef __PALMOS__
#include <PceNativeCall.h>
#endif

//#define _PROFILE
#undef _PROFILE

#include "../common/types.h"
#include "../common/Profile.h"

//
// !! CPC native engine should be rebuilt following to any modification of thie file !!
//


/***********************************************************************
 *
 *  Application Defines
 *
 ***********************************************************************/

#undef ENABLE_TAPE
//#define ENABLE_TAPE

#undef ENABLE_PRINTER
//#define ENABLE_PRINTER

// CAUTION: Palm TE doesn't accept native soundstream
#undef SNDSTREAM_NATIVE
//#define SNDSTREAM_NATIVE

#define __NEWMEMLAYOUT__
//#undef __NEWMEMLAYOUT__

// Simulator doesn't accept native soundstream
#ifdef __SIMU__
#undef SNDSTREAM_NATIVE
#endif /* __SIMU__ */


/***********************************************************************
 *
 *  Common macros
 *
 ***********************************************************************/

#define DWORD_UPPER_ALIGN(x)               ((x+3) & ~3)

#define NUMBER_OF_ITEMS(array)             (sizeof(array) / sizeof(array[0]))
#define IS_POWER_OF_2(x)                   ((x) == ((x) & (~(x) + 1)))
#define ROUND_UPPER_8(x)                   ((x+7) & ~7)
#define OFFSET_OF(STRUCTURE,FIELD)         ((long)((char*)&((STRUCTURE*)0)->FIELD))

#define ROUND_UPPER_POWER_OF_2(val,trig)   { val=1; while (val<trig) val<<=1; }



/***********************************************************************
 *
 *  Internal Constants
 *
 ***********************************************************************/

// Execute return code
#define EC_BREAKPOINT             0
#define EC_FRAME_COMPLETE         1
#define EC_CYCLE_COUNT            2
#define EC_SOUND_BUFFER           3
#define EC_END_INSTRUCTION        4

#define CLOCK_Z80                 4000000 // 4MHz
#define CYCLES_PER_SECOND         50
#define CYCLES_MS                 (1000 / CYCLES_PER_SECOND)
#define CYCLE_COUNT_INIT          (CLOCK_Z80 / CYCLES_PER_SECOND) // 4MHz divided by 50Hz = number of CPU cycles per frame
#define CLOCK_PER_MS(MS)          (CLOCK_Z80 * MS / 1000)

#define CLOCK_AY                  125000 // 1MHz / 8

#define SND_FREQ                  (44100/2)
#define SND_SAMPLES               (SND_FREQ / CYCLES_PER_SECOND) //??
#define SND_16BITS                1 /* 0=8bits, 1=16bits */
#define SND_STEREO                1 /* 0=Mono, 1=Stereo */

// Number of CPU cycles per sample
#define SND_CYCLE_SOUND_INIT      (((CLOCK_Z80 << 8) / SND_FREQ) << 8) // ((CLOCK_Z80 << 16) / SND_FREQ)
// Number of AY cycles per sample (Most significant short)
#define SND_LOOP_COUNT_INIT       (((CLOCK_AY << 8) / SND_FREQ) << 8) // ((CLOCK_AY << 16) / SND_FREQ)

// Lightguns
#define LIGHTGUN_BEAM_MS          1 // beam pulse duration (in ms) (Best found=5 for GunStick)
#define LIGHTGUN_RASTER_INIT      ((LIGHTGUN_BEAM_MS * 1000) / 64) // 64�s per raster


// Samples requested par SndStreamCreate callback
// 8bits=2048, 16bits=1024, Mono=4096
#if   SND_SAMPLES < 1024
#  define SND_BUFFER_SAMPLES      2048
#elif SND_SAMPLES < 512
#  define SND_BUFFER_SAMPLES      1024
#elif SND_SAMPLES < 256
#  define SND_BUFFER_SAMPLES      512
#else
#  define SND_BUFFER_SAMPLES      256
#endif

#if SND_STEREO == 1
#  if SND_16BITS == 0
#    define SND_FRAME_SAMPLES     (SND_BUFFER_SAMPLES * 2)
#  else /* SND_16BITS == 0 */
#    define SND_FRAME_SAMPLES     SND_BUFFER_SAMPLES
#  endif /* SND_16BITS == 0 */
#else /* SND_STEREO == 1 */
#  define SND_FRAME_SAMPLES       (SND_BUFFER_SAMPLES * 4)
#endif /* SND_STEREO == 1 */

// Sample size (in bytes)
#define SND_SAMPLE_SIZE           ( (SND_16BITS + 1) * (SND_STEREO + 1) )

// Size of sound buffer (in bytes)
// Used to generate EC_SOUND_BUFFER exit condition
#define SND_BUFFER_SIZE           (SND_BUFFER_SAMPLES * SND_SAMPLE_SIZE)

// BufferSize (in frames) parameter for SndStreamCreate and SndStreamCreateExtended
#define SND_STREAM_SAMPLES        SND_SAMPLES


#define SND_SAMPLE_OFFSET_16BIT   0
#define SND_SAMPLE_OFFSET_8BIT    128

#if SND_STEREO == 1
#  if SND_16BITS == 0
#    define SND_SAMPLE_ORIGIN     SND_SAMPLE_OFFSET_8BIT
#  else /* SND_16BITS == 0 */
#    define SND_SAMPLE_ORIGIN     SND_SAMPLE_OFFSET_16BIT
#  endif /* SND_16BITS == 0 */
#else /* SND_STEREO == 1 */
#  define SND_SAMPLE_ORIGIN       SND_SAMPLE_OFFSET_8BIT
#endif /* SND_STEREO == 1 */

#define NO_BORDER_RENDER

#ifdef NO_BORDER_RENDER
#define VDU_BORDERSHIFT           50   // = TOP_VSKIP + VDU_VSTART(27)
#define CPC_SCR_WIDTH             352  // Visible width = 4+40 * 8
#define CPC_SCR_HEIGHT            312
#define CPC_VISIBLE_SCR_HEIGHT    263  // ON_SCR_HEIGHT + TOP_VSKIP - 1
#define ON_SCR_WIDTH              320  // Odroid
#define ON_SCR_HEIGHT             240  // Odroid
#else
#define CPC_SCR_WIDTH             384  // Visible width = 4+40+4 * 8
#define CPC_SCR_HEIGHT            312
#define CPC_VISIBLE_SCR_HEIGHT    272  // Visible height = 5+25+4 * 8
#define ON_SCR_WIDTH              CPC_SCR_WIDTH
#define ON_SCR_HEIGHT             CPC_SCR_HEIGHT
#endif

#define CPC_MODEL_464             0
#define CPC_MODEL_664             1
#define CPC_MODEL_6128            2

#define ROM_ID_TYPE               'tbin'
#define ROM_ID_464                1001
#define ROM_ID_664                1002
#define ROM_ID_6128               1003
#define ROM_ID_AMSDOS             2000
#define ROM_ID_PARADOS            2001
#define ROM_ID_SIZE_CPC           32768
#define ROM_ID_SIZE_AMSDOS        16384
#define ROM_ID_SIZE_PARADOS       16384

#define PATHNAME_MAXSIZE          80

#define FONT_CPC_WIDTH            8
#define FONT_CPC_HEIGHT           8

#define MEMBANK_NUMBER            4


// CPC Displayed Manufacturer Name
#define CPC_BRAND_MASK            0x0E
#define CPC_JUMPER_AMSTRAD        0x0E
#define CPC_JUMPER_ORION          0x0C
#define CPC_JUMPER_SCHNEIDER      0x0A
#define CPC_JUMPER_AWA            0x08
#define CPC_JUMPER_SOLAVOX        0x06
#define CPC_JUMPER_SAISHO         0x04
#define CPC_JUMPER_TRIUMPH        0x02
#define CPC_JUMPER_ISP            0x00
// Settings
#define CPC_JUMPER_50HZ           0x10
//
#define CPC_DEFAULT_JUMPERS       (CPC_JUMPER_AMSTRAD | CPC_JUMPER_50HZ)


#define Sflag                     0x80 // sign flag
#define Zflag                     0x40 // zero flag
#define Hflag                     0x10 // halfcarry flag
#define Pflag                     0x04 // parity flag
#define Vflag                     0x04 // overflow flag
#define Nflag                     0x02 // negative flag
#define Cflag                     0x01 // carry flag
#define Xflags                    0x28 // bit 5 & 3 flags


// CRTC flags
#define VS_flag                   1
#define HS_flag                   2
#define HDT_flag                  4
#define VDT_flag                  8
#define HT_flag                   16
#define VT_flag                   32
#define MR_flag                   64
#define VTadj_flag                128
#define VSf_flag                  256

//
// FDC
//
#define CMD_PHASE                 0
#define EXEC_PHASE                1
#define RESULT_PHASE              2

#define FDC_TO_CPU                0
#define CPU_TO_FDC                1

#define SKIP_flag                 1     // skip sectors with DDAM/DAM
#define SEEKDRVA_flag             2     // seek operation has finished for drive A
#define SEEKDRVB_flag             4     // seek operation has finished for drive B
#define RNDDE_flag                8     // simulate random DE sectors
#define OVERRUN_flag              16    // data transfer timed out
#define SCAN_flag                 32    // one of the three scan commands is active
#define SCANFAILED_flag           64    // memory and sector data does not match
#define STATUSDRVA_flag           128   // status change of drive A
#define STATUSDRVB_flag           256   // status change of drive B

#define CMD_CODE                  0
#define CMD_UNIT                  1
#define CMD_C                     2
#define CMD_H                     3
#define CMD_R                     4
#define CMD_N                     5
#define CMD_EOT                   6
#define CMD_GPL                   7
#define CMD_DTL                   8
#define CMD_STP                   8

#define RES_ST0                   0
#define RES_ST1                   1
#define RES_ST2                   2
#define RES_C                     3
#define RES_H                     4
#define RES_R                     5
#define RES_N                     6

#define OVERRUN_TIMEOUT           (128 * 4) // ??
//#define INITIAL_TIMEOUT           (OVERRUN_TIMEOUT * 4) // ??
// Fix for Orion Prime Disk accesses
#define INITIAL_TIMEOUT           (OVERRUN_TIMEOUT * 10) // ??

#define DSK_BPTMAX                8192
#define DSK_TRACKMAX              102   // max amount that fits in a DSK header
#define DSK_SIDEMAX               2
#define DSK_SECTORMAX             29    // max amount that fits in a track header

#define MAX_CMD_COUNT             15

//
// DISK
//
#define MAX_DISK_FORMAT 										2
#define FIRST_CUSTOM_DISK_FORMAT						2

#define DISK_FORMAT_DATA										0
#define DISK_FORMAT_VENDOR									1

#define DEFAULT_DISK_FORMAT									DISK_FORMAT_DATA

//
// TAPE
//
#define TAPE_PILOT_STAGE          1
#define TAPE_SYNC_STAGE           2
#define TAPE_DATA_STAGE           3
#define TAPE_SAMPLE_DATA_STAGE    4
#define TAPE_PAUSE_STAGE          5
#define TAPE_END                  6

#define TAPE_VOLUME               32

#define CYCLE_SCALE               ((((tULong)40) << 16) / 35)
#define CYCLE_ADJUST(p)           ((((tULong)p) * CYCLE_SCALE) >> 16)
#define MS_TO_CYCLES(p)           ((tULong)(p) * 4000)

#define TAPE_LEVEL_LOW            0
#define TAPE_LEVEL_HIGH           0x80

//
// Audio
//
#define AUDIO_INDEX_AL            255
#define AUDIO_INDEX_BL            170
#define AUDIO_INDEX_CL            13
#define AUDIO_INDEX_AR            13
#define AUDIO_INDEX_BR            170
#define AUDIO_INDEX_CR            255
//
// 50 minimum should be used to get strong enough signal to be able to hear weak sounds (Sorcery Channel B)
#define AUDIO_SAMPLE_VOLUME           100
#define AUDIO_PREAMP_MAX              100
#define AUDIO_INDEX_MONO              255
#define AUDIO_SPEAKER_VOLUME          120
#define AUDIO_SPEAKER_PREAMP_MAX      100
#define AUDIO_PLAYCITY_VOLUME         120
#define AUDIO_PLAYCITY_PREAMP_MAX     100

//
// Z80 registers access macros
//
#define _A        Z80->Regs.AF.b.h
#define _F        Z80->Regs.AF.b.l
#define _AF       Z80->Regs.AF.w.l
#define _AFdword  Z80->Regs.AF.d
#define _B        Z80->Regs.BC.b.h
#define _C        Z80->Regs.BC.b.l
#define _BC       Z80->Regs.BC.w.l
#define _BCdword  Z80->Regs.BC.d
#define _D        Z80->Regs.DE.b.h
#define _E        Z80->Regs.DE.b.l
#define _DE       Z80->Regs.DE.w.l
#define _DEdword  Z80->Regs.DE.d
#define _H        Z80->Regs.HL.b.h
#define _L        Z80->Regs.HL.b.l
#define _HL       Z80->Regs.HL.w.l
#define _HLdword  Z80->Regs.HL.d
#define _PC       Z80->Regs.PC.w.l
#define _PCdword  Z80->Regs.PC.d
#define _SP       Z80->Regs.SP.w.l
#define _SPdword  Z80->Regs.SP.d
//
#define _IXh      Z80->Regs.IX.b.h
#define _IXl      Z80->Regs.IX.b.l
#define _IX       Z80->Regs.IX.w.l
#define _IXdword  Z80->Regs.IX.d
#define _IYh      Z80->Regs.IY.b.h
#define _IYl      Z80->Regs.IY.b.l
#define _IY       Z80->Regs.IY.w.l
#define _IYdword  Z80->Regs.IY.d
//
#define _I        Z80->Regs.I
#define _R        Z80->Regs.R
#define _Rb7      Z80->Regs.Rb7
#define _IFF1     Z80->Regs.IFF1
#define _IFF2     Z80->Regs.IFF2
#define _IM       Z80->Regs.IM
#define _HALT     Z80->Regs.HALT

//
// Function pointers
//
typedef tVoid (*tCaseEnvType)(tVoid*);
typedef tVoid (*tDrawHandlerPtr)(tVoid*, tULong);
typedef tVoid (*tSynthesizerPtr)(tVoid*);
typedef tVoid (*tCmdHandlerPtr)(tVoid*, tVoid*);
typedef tVoid* (*tMemPtrNewFct)(tULong);
typedef tVoid  (*tMemPtrDeleteFct)(tVoid*);

// =============================================================================
// ATTENTION : 4-bytes data should be 4-bytes aligned
//
// 4-bytes data first
// then 2-bytes data
// and 1-byte data at the end
// =============================================================================


// ============================================================================
//
// colours_rgb_entry
//
typedef struct
{
  tUChar colour_r;
  tUChar colour_g;
  tUChar colour_b;
  tUChar intensity;
  tUChar shadow_index;
} colours_rgb_entry;
// ============================================================================


// ============================================================================
//
// tAutoStartReturnCode
//
typedef enum
{
	AutoStartOK = 0,
	DiskFormatUnknown,
	NoValidFileEntry,
	TooManyFileEntries
} tAutoStartReturnCode;
// ============================================================================


// ============================================================================
//
// tNoise
//
typedef union
{
  struct
  {
    tUShort Low;
    tUShort Val;
  } s;
  tULong Seed;
} tNoise;
// ============================================================================


// ============================================================================
//
// tZ80Regs
//
typedef struct
{
  tRegister AF, BC, DE, HL, PC, SP, AFx, BCx, DEx, HLx, IX, IY;
  tULong I, R, Rb7, IFF1, IFF2, IM, HALT, EI_issued, int_pending;
  tULong breakpoint;
} tZ80Regs;

#define DEBUG_SIZE_Z80              (0)
#define TRACE_SIZE_Z80              (0)
// ============================================================================


// ============================================================================
//
// tZ80
//
typedef struct
{
  //
  // Static pointers first (not affected by restoration)
  //
  tUChar* SZ;
  tUChar* SZ_BIT;
  tUChar* SZP;
  tUChar* SZHV_inc;
  tUChar* SZHV_dec;

  //
  // 32bits, 16 bits then 8 bits variables
  //
  tULong iCycleCount;
  tULong iWSAdjust;

  tZ80Regs Regs;
} tZ80;

#define STATIC_SIZE_Z80       ( sizeof(tUChar*) + /* SZ */ \
                                sizeof(tUChar*) + /* SZ_BIT */ \
                                sizeof(tUChar*) + /* SZP */ \
                                sizeof(tUChar*) + /* SZHV_inc */ \
                                sizeof(tUChar*)   /* SZHV_dec */ )
#define DYNAMIC_SIZE_Z80      0
// ============================================================================


// ============================================================================
//
// fdc_cmd_table_def
//
typedef struct
{
  //
  // 32bits, 16 bits then 8 bits variables
  //
  tCmdHandlerPtr cmd_handler;
  tULong cmd;
  tULong cmd_length;
  tULong res_length;
  tULong cmd_direction;
} fdc_cmd_table_def;
// ============================================================================


// ============================================================================
//
// tSector
//
#define SECTOR_CHRN_NBELEMENT           4
#define SECTOR_CHRN_C                   0
#define SECTOR_CHRN_H                   1
#define SECTOR_CHRN_R                   2
#define SECTOR_CHRN_N                   3
#define SECTOR_FLAGS_NBELEMENT          4
typedef struct
{
  //
  // 32bits, 16 bits then 8 bits variables
  //
  tUChar* data; // pointer to sector data
  tULong  size; // sector size in bytes
  tUChar  CHRN[SECTOR_CHRN_NBELEMENT]; // the CHRN for this sector
  tUChar  flags[SECTOR_FLAGS_NBELEMENT]; // ST1 and ST2 - reflects any possible error conditions
} tSector;
// ============================================================================


// ============================================================================
//
// tTrack
//
typedef struct
{
  //
  // 32bits, 16 bits then 8 bits variables
  //
  tUChar* data; // pointer to track data
  tULong  sectors; // sector count for this track
  tULong  size; // track size in bytes
  tSector sector[DSK_SECTORMAX]; // array of sector information structures
} tTrack;
// ============================================================================


// ============================================================================
//
// tDrive
//
#define MAX_SIZETAB_FILENAME            100
#define SIZETAB_FILENAME                (DWORD_UPPER_ALIGN((tULong)sizeof(tChar) * MAX_SIZETAB_FILENAME))

typedef struct
{
  //
  // 32bits, 16 bits then 8 bits variables
  //
  tUChar* dataP;
  tULong data_size;

  tULong tracks; // total number of tracks
  tULong current_track; // location of drive head
  tULong sides; // total number of sides
  tULong current_side; // side being accessed
  tULong current_sector; // sector being accessed
  tULong random_DEs; // sectors with Data Errors return random data?
  tULong flipped; // reverse the side to access?
  tULong write_protected; // is the image write protected?
  tULong altered; // has the image been modified?

  tTrack track[DSK_TRACKMAX][DSK_SIDEMAX]; // array of track information structures

  tChar filename[SIZETAB_FILENAME];

} tDrive;

#define DEBUG_SIZE_DRIVE       (0)
#define TRACE_SIZE_DRIVE       (0)
// ============================================================================


// ============================================================================
//
// tDiskFormat
//
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
// ============================================================================


// ============================================================================
//
// tDSKHeader
//
typedef struct
{
   tChar id[34];
   tChar unused1[14];
   tUChar tracks;
   tUChar sides;
   tUChar unused2[2];
   tUChar track_size[DSK_TRACKMAX * DSK_SIDEMAX];
} tDSKHeader;
// ============================================================================


// ============================================================================
//
// tTrackHeader
//
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
// ============================================================================


// ============================================================================
//
// tCRTC
//
#define CRTC_DRAWHANDLER_NBELEMENT          4
#define CRTC_REGISTERS_NBELEMENT            18

typedef struct
{
  //
  // Static pointers first (not affected by restoration)
  //
  tDrawHandlerPtr DrawHandler[CRTC_DRAWHANDLER_NBELEMENT];
  tDrawHandlerPtr CurrentHandler;
  tDrawHandlerPtr RequestedHandler;

  //
  // 32bits, 16 bits then 8 bits variables
  //
  tULong flags;
  tULong requested_addr;
  tULong addr;

  tULong char_count;
  tULong line_count;
  tULong raster_count;
  tULong vt_adjust;
  tULong vt_adjust_count;
  tULong skew;
  tULong max_raster;
  tULong last_hdisp;
  tULong reg_select;
  tULong registers[CRTC_REGISTERS_NBELEMENT];
  tULong start_addr;
  tULong lpen_offset;

  tULong requested_mode;
  tULong current_mode;

  tULong stop_rendering;

  tLong hsw;
  tLong hsw_active;
  tLong hsw_count;
  tLong vsw;
  tLong vsw_count;
} tCRTC;
#define STATIC_SIZE_CRTC      ( (sizeof(tDrawHandlerPtr) * CRTC_DRAWHANDLER_NBELEMENT) + /* DrawHandler */ \
                                sizeof(tDrawHandlerPtr) + /* CurrentHandler */ \
                                sizeof(tDrawHandlerPtr)   /* CurrentHandler */ )
#define DYNAMIC_SIZE_CRTC     (0)
#define DEBUG_SIZE_CRTC       (0)
#define TRACE_SIZE_CRTC       (0)
// ============================================================================


// ============================================================================
//
// tVDU
//
#define VDU_SCANLINE_MIN          CPC_VISIBLE_SCR_HEIGHT
#define VDU_HSTART                7
#define VDU_VSTART                27
#define VDU_HWIDTH                (CPC_SCR_WIDTH / 8)
#define VDU_VHEIGHT               CPC_VISIBLE_SCR_HEIGHT

typedef struct
{
  //
  // Static pointers first (not affected by restoration)
  //
  tULong* scr_base;
  tULong* scr_root;

  tULong* scr_current;
  tUChar* mode0_table;
  tUChar* mode1_table;
  tUChar* pbRAM;
  tULong* palette;

  //
  // 32bits, 16 bits then 8 bits variables replaced by Restoration
  //
  tULong scanline;
  tULong vcount;

  tULong scr_line;
  tULong scr_line_offs; // Number of ULong for a horizontal line

  tULong hdelay;
  tULong vdelay;
  tULong frame_completed;
  tULong char_count;
  tULong hcount;

  tLong hsw;
  tLong hsw_active;
  tLong hsw_count;
  tLong vsw_count;
} tVDU;

#define STATIC_SIZE_VDU       ( sizeof(tULong*) + /* scr_base */ \
                                sizeof(tUChar*) + /* scr_current */ \
                                sizeof(tUChar*) + /* mode0_table */ \
                                sizeof(tUChar*) + /* mode1_table */ \
                                sizeof(tUChar*) + /* pbRAM */ \
                                sizeof(tULong*) /* palette */ )
#define DYNAMIC_SIZE_VDU      (0)
#define DEBUG_SIZE_VDU        (0)
#define TRACE_SIZE_VDU        (0)
// ============================================================================


// ============================================================================
//
// tGateArray
//
#define GA_INK_VALUES_NBELEMENT           17
#define GA_PALETTE_NBELEMENT              17
typedef struct
{
  //
  // 32bits, 16 bits then 8 bits variables
  //
  tULong sl_count;

  tULong ROM_config;
  tULong RAM_bank;
  tULong RAM_config;
  tULong upper_ROM;
  tULong pen;

  tULong int_delay;
  tULong ink_values[GA_INK_VALUES_NBELEMENT];
  tULong palette[GA_PALETTE_NBELEMENT];

} tGateArray;

#define DEBUG_SIZE_GATEARRAY              (0)
#define TRACE_SIZE_GATEARRAY              (0)
// ============================================================================


// ============================================================================
//
// tFDC
//
#define FDC_MAX_TRACK_SIZE              (6144 - 154)

#define FDC_COMMAND_NBELEMENT           12
#define FDC_RESULT_NBELEMENT            8

#define FDC_DEBUG_NB_LAST_COMMANDS      10

typedef struct
{
  //
  // Static pointers first (not affected by restoration)
  //
  fdc_cmd_table_def* FDCCommandTable;

  //
  // Dynamic pointers (Reindexed by Restoration)
  //
  tDrive* active_drive; // reference to the currently selected drive
  tTrack* active_track; // reference to the currently selected track, of the active_drive

  tUChar* buffer_ptr;
  tUChar* buffer_endptr;

  //
  // 32bits, 16 bits then 8 bits variables
  //
  tCmdHandlerPtr cmd_handler;

  tULong flags;

  tULong led;
  tULong motor;
  tULong phase;
  tULong cmd_length;
  tULong res_length;
  tULong cmd_direction;
  tULong read_status_delay;
  tULong byte_count;
  tULong buffer_count;
  tULong cmd_handler_index;

  tULong command[FDC_COMMAND_NBELEMENT];
  tULong result[FDC_RESULT_NBELEMENT];

  tLong timeout;

#ifdef _DEBUG
  tULong debug_nb_operations;
  tULong debug_current_operation_index;
  tULong debug_lastoperations[FDC_DEBUG_NB_LAST_COMMANDS][FDC_COMMAND_NBELEMENT + FDC_RESULT_NBELEMENT];
  tULong debug_size_commands[FDC_DEBUG_NB_LAST_COMMANDS];
  tULong debug_size_results[FDC_DEBUG_NB_LAST_COMMANDS];
#endif /* _DEBUG */
} tFDC;

#define STATIC_SIZE_FDC         ( sizeof(fdc_cmd_table_def*) /* FDCCommandTable */ )
#define DYNAMIC_SIZE_FDC        ( sizeof(tDrive*) + /* active_drive */ \
                                  sizeof(tTrack*) + /* active_track */ \
                                  sizeof(tUChar*) + /* buffer_ptr */ \
                                  sizeof(tUChar*)   /* buffer_endptr */ )
#define DEBUG_SIZE_FDC          ( sizeof(tULong)  + /* debug_nb_operations */ \
                                  sizeof(tULong)  + /* debug_current_operation_index */ \
                                  (sizeof(tULong) * FDC_DEBUG_NB_LAST_COMMANDS * (FDC_COMMAND_NBELEMENT + FDC_RESULT_NBELEMENT)) + /* debug_lastcommands */ \
                                  (sizeof(tULong) * FDC_DEBUG_NB_LAST_COMMANDS) + /* debug_size_commands*/ \
                                  (sizeof(tULong) * FDC_DEBUG_NB_LAST_COMMANDS) /* debug_size_results*/ )

#define TRACE_SIZE_FDC          (0)
// ============================================================================


// ============================================================================
//
// tPPI
//
typedef struct
{
  //
  // 32bits, 16 bits then 8 bits variables
  //
  tULong control;
  tULong portA;
  tULong portB;
  tULong portC;

} tPPI;

#define DEBUG_SIZE_PPI         (0)
#define TRACE_SIZE_PPI         (0)
// ============================================================================


// ============================================================================
//
// tPSG
//
typedef struct
{
  //
  // Static pointers first (not affected by restoration)
  //
  tLong* Level_PP;
  tLong* Level_AR;
  tLong* Level_AL;
  tLong* Level_BR;
  tLong* Level_BL;
  tLong* Level_CR;
  tLong* Level_CL;

  tUChar* pbSndBuffer;
  tUChar* pbSndBufferEnd;

  //
  // Dynamic pointers (Reindexed by Restoration)
  //
  tUChar* snd_bufferptr;

  //
  // 32bits, 16 bits then 8 bits variables
  //
  tCaseEnvType Case_Env;

#ifndef _Z80_WAIT_STATES_AUDIO_OPT
  tULong cycle_count;
#else /* _Z80_WAIT_STATES_AUDIO_OPT */
  tLong cycle_count;
#endif /* _Z80_WAIT_STATES_AUDIO_OPT */
  tULong LoopCount;

  tNoise Noise;

  tULong FilledBufferSize;

  tULong Ton_Counter_A;
  tULong Ton_Counter_B;
  tULong Ton_Counter_C;

  tULong Envelope_Counter;
  tULong Noise_Counter;

  tULong Ton_EnA;
  tULong Ton_EnB;
  tULong Ton_EnC;
  tULong Noise_EnA;
  tULong Noise_EnB;
  tULong Noise_EnC;
  tULong Envelope_EnA;
  tULong Envelope_EnB;
  tULong Envelope_EnC;

  tULong Ton_A;
  tULong Ton_B;
  tULong Ton_C;

#ifdef _DEBUG
  tULong SampleCount;   // DEBUG
  tULong DebugFlags;    // DEBUG
#endif /* _DEBUG */

  tULong buffer_full;

  tULong snd_volume;
  tULong snd_pp_device;

  tULong FirstPeriod;
  tULong control;
  tULong reg_select;

  tULong bTapeLevel;
  tULong snd_enabled;

  tLong AmplitudeEnv;

  tLong Left_Chan;
  tLong Right_Chan;

  tLong LevelTape;

  union
  {
    tUChar Index[16];
    struct
    {
      tUChar TonALo;        // R0  = Channel A Fine Tune Register
      tUChar TonAHi;        // R1  = Channel A Coarse Tune Register
      tUChar TonBLo;        // R2  = Channel B Fine Tune Register
      tUChar TonBHi;        // R3  = Channel B Coarse Tune Register
      tUChar TonCLo;        // R4  = Channel C Fine Tune Register
      tUChar TonCHi;        // R5  = Channel C Coarse Tune Register
      tUChar Noise;         // R6  = Noise Generator Control
      tUChar Mixer;         // R7  = Mixer Control-I/O Enable
      tUChar AmplitudeA;    // R10 = Channel A Amplitude Control
      tUChar AmplitudeB;    // R11 = Channel B Amplitude Control
      tUChar AmplitudeC;    // R12 = Channel C Amplitude Control
      tUChar EnvelopeLo;    // R13 = Enveloppe Period Control Fine Tune
      tUChar EnvelopeHi;    // R14 = Enveloppe Period Control Coarse Tune
      tUChar EnvType;       // R15 = Enveloppe Shape/Cycle Control
      tUChar PortA;         // R16 = I/O Port Data
      tUChar PortB;         // R17 = I/O Port Data
    } UChar;
  } RegisterAY;

} tPSG;

#define STATIC_SIZE_PSG   ( sizeof(tLong*)  + /* Level_PP */ \
                            sizeof(tLong*)  + /* Level_AR */ \
                            sizeof(tLong*)  + /* Level_AL */ \
                            sizeof(tLong*)  + /* Level_BR */ \
                            sizeof(tLong*)  + /* Level_BL */ \
                            sizeof(tLong*)  + /* Level_CR */ \
                            sizeof(tLong*)  + /* Level_CL */ \
                            sizeof(tUChar*) + /* pbSndBuffer */ \
                            sizeof(tUChar*)   /* pbSndBufferEnd */ )
#define DYNAMIC_SIZE_PSG  ( sizeof(tUChar*)   /* snd_bufferptr */ )
#define DEBUG_SIZE_PSG    ( sizeof(tULong)  + /* SampleCount */ \
                            sizeof(tULong)    /* DebugFlags */ )
#define TRACE_SIZE_PSG    (0)
// ============================================================================


// ============================================================================
//
// tPreferences
//
//
// !!! CAUTION !!!
// Published version dependant structure size.
//
// DO NOT DELETE ITEMS. DO NOT RESIZE ITEMS. 8 Bits variables ONLY
//
typedef struct
{
  // Version 1
  tUChar CPCModel;                // CPC_MODEL_464, CPC_MODEL_664 ou CPC_MODEL_6128
  tUChar CPCJumpers;              // Settings jumpers
  tUChar ScreenType;              // 0 = Color, 1 = Monochrome
  tUChar CPCTrueSpeed;            // 0 = Full Speed, 1 = CPC Speed
  tChar  FilesPathname[80];
  // Version 2
  tUChar ScreenIntensity;
  tUChar SoundEnabled;            // 0 = No sound, 1 = Sound played
  tUChar SoundVolume;             // Sound volume when played
  tUChar SoundSystemVolume;       // 0 = Own volume, 1 = System Games volume
  // Added from CPF
  tUChar  SpeakerStereo;           // 0 = Mono, 1 = Stereo
  tUChar  Speaker16Bits;           // 0 = 8 bits, 1 = 16 bits
  // Version 3
  tUChar Display320x480;          // Last display mode
  tUChar AutomaticContextSave;    // 0 = Manual, 1 = Automatic
  tUChar CustomColorUse;          // Not used : 0 = Do not use custom colors, 1 = Use custom colors
  tUChar CustomColorIndexes[32];  // Not used
  tUChar Mode2AntiAliasing;       // 0 = Mode 2 Aliased, 1 = Mode 2 Anti-aliased
  // Version 4
  tUChar OnScreenDisplayActive;   // 0 = Inactive, 1 = Active
  tUChar OnScreenRockerActive;    // 0 = Inactive, 1 = Active
  tUChar RockerCenterKeyIndex;
  tUChar JoystickFireHardKey;     // No longer used
  tUChar PreferencesChanged;
  // Version 5
  tUChar AutoOffDisable;
  tUChar ScreenDisplayOffsetX;
  tUChar ScreenDisplayOffsetY;
  tUChar HardKey1Index;
  tUChar HardKey2Index;
  tUChar HardKey3Index;
  tUChar HardKey4Index;
  // Version 6
  tUChar UseParados;              // 0 = AMSDOS, 1 = PARADOS
  tUChar Use64kMemoryExtension;   // 0 = Not used, 1 = Used
  tUChar NightModeActive;
  // Version 7
  tUChar CPCKeycodeA;
  tUChar Rendering;
  // Version 8
  tUChar CPCKeycodeB;
  tUChar CPCKeycodeC;
  tUChar CPCKeycodeD;
  tUChar AutoStartEnable;
  // Version 9
  tUChar OnScreenLightPenActive;   // 0 = Inactive, 1 = Active
  tUChar OnScreenMagnumGunActive;  // 0 = Inactive, 1 = Active
  tUChar OnScreenGunstickActive;   // 0 = Inactive, 1 = Active
  tUChar OnScreenWestPhaserActive; // 0 = Inactive, 1 = Active
  //
} tPreferences;
// ============================================================================


// ============================================================================
//
// tNativeCPC
//
#define NATIVECPC_MEMBANK_CONFIG_NBELEMENT            8
#define NATIVECPC_WCYCLETABLE_NBELEMENT               2
#define NATIVECPC_KEYBOARD_MATRIX_NBELEMENT           16
#define NATIVECPC_COLOURS_NBELEMENT                   32
#define NATIVECPC_PALM_PALETTE_NBELEMENT              256
typedef struct
{
  //
  // Static pointers first (not affected by restoration)
  //
  tZ80* Z80;

  tCRTC* CRTC;
  tUChar* mode0_table;
  tUChar* mode1_table;

  tGateArray* GateArray;

  tFDC* FDC;
  fdc_cmd_table_def* FDCCommandTable; // FDC

  tPPI* PPI;

  tPSG* PSG;

  tVDU* VDU;

  tPreferences* prefP;
  tUChar* contextP;

  tUChar* pbGPBuffer;  // FDC ONLY ?
  tUChar* pbRAM;
  tUChar** memmap_ROM;

  tUShort* DAATable; // Z80

  colours_rgb_entry* colours_rgb; // Video
  colours_rgb_entry* colours_green; // Video
  colours_rgb_entry* active_colours; // Video

  tVoid* BmpOffScreenBits; // Video

  #ifdef __PALMOS__
  const tVoid* emulStateP;
  Call68KFuncType* call68KFuncP;
  #else
  tVoid* emulStateP;
  tVoid* call68KFuncP;
  #endif

  // added to fix PALMOS inconsitencies
  tMemPtrNewFct MemPtrNewPtr;
  tMemPtrDeleteFct MemPtrDeletePtr;
  colours_rgb_entry* WinPalettePtr;
  tUShort* RGB565PalettePtr;

  tULong TraceAlertPtr;
  tULong SoundCalculateLevelTablesPtr;

  tVoid*  hMemROMlo;
  tVoid*  hMemAMSDOS;
  tUChar* pbROMlo;
  tUChar* pbROMhi;
  tUChar* pbExpansionROM;

  tULong FirstInitToPerform;
  tULong RestorationPerformed;

  // Screen copy in landscape mode
  tULong* OffscreenStartBits;
  tULong  OffscreenCopyHeight;
  tULong  OffscreenCopyWidth;
  tULong  OffscreenAlignGap;
  tULong* OnscreenStartBits;
  tULong  OnscreenPixelGap;
  tULong  OnscreenAlignGap;

  //
  // Dynamic pointers (Reindexed by Restoration)
  //
  tDrive* DriveA;
  tDrive* DriveB;

  tUChar* membank_config[NATIVECPC_MEMBANK_CONFIG_NBELEMENT][MEMBANK_NUMBER];
  tUChar* membank_read[MEMBANK_NUMBER];
  tUChar* membank_write[MEMBANK_NUMBER];

  tUShort* pwTapePulseTable;  // Tape
  tUShort* pwTapePulseTableEnd; // Tape
  tUShort* pwTapePulseTablePtr; // Tape

  tUChar* pbTapeBlock;  // Tape
  tUChar* pbTapeBlockData;  // Tape

  tUChar* pbTapeImage;  // Tape
  tUChar* pbTapeImageEnd; // Tape



  //
  // 32bits, 16 bits then 8 bits variables replaced by Restoration
  //
  tULong dwTapePulseCycles; // Tape
  tULong dwTapeZeroPulseCycles; // Tape
  tULong dwTapeOnePulseCycles;  // Tape
  tULong dwTapeStage; // Tape
  tULong dwTapePulseCount;  // Tape
  tULong dwTapeDataCount; // Tape
  tULong dwTapeBitsToShift; // Tape

  tULong ram_size; // CPC

  tULong jumpers;
  tULong keyboard_line; // keyboard matrix index
  tULong tape_motor;
  tULong tape_play_button;
  tULong printer;
  tULong printer_port;
  tULong scr_tube;
  tULong scr_intensity;

  // Lightgun emulation
  tULong lightgun_x_pos;
  tULong lightgun_y_pos;
  tULong lightgun_beam_key_line;
  tULong lightgun_beam_key_mask;
  tULong lightgun_sensitivity;
  tULong lightgun_beam_detect;
  tULong lightgun_beam_state;
  tLong  lightgun_counter;
  tULong lightgun_random_crtc; // 0=inactive; 1=active
  tULong lightgun_one_shot; // FC!! DEBUG
  tULong lightgun_debug_flags; // FC!! DEBUG
  tULong lightgun_debug_counter; // FC!! DEBUG
  tULong lightgun_debug_screen_pal; // FC!! DEBUG

  // TRACE
  tULong TraceBreakpoint;
  tULong TraceFollowUp;

#ifdef _PROFILE
  tULong profileCounter[PROFILE_NB_ITEMS];
  tULong profileStates;
#endif /* _PROFILE */

  tLong cycle_count;

  tLong iTapeCycleCount; // Tape

  tUShort BmpOffScreenBytesRow; // Video

  tUShort wCycleTable[NATIVECPC_WCYCLETABLE_NBELEMENT]; // Tape

  tUChar keyboard_matrix[NATIVECPC_KEYBOARD_MATRIX_NBELEMENT];

  tUChar paused;

  tUChar bTapeData; // Tape

  tUChar colours[NATIVECPC_COLOURS_NBELEMENT]; // Video

  tUChar PalmToCPCPalette[NATIVECPC_PALM_PALETTE_NBELEMENT];
  tUChar PalmPaletteIntensity[NATIVECPC_PALM_PALETTE_NBELEMENT];

  tUChar drive_led;   // Copy of FDC->led to speed up FDC Led display update

  tUChar night_mode;

  // CPC key code for hard keys. Session dependant.
  tUChar HardKeyCPCKeyCodeA;
  tUChar HardKeyCPCKeyCodeB;
  tUChar HardKeyCPCKeyCodeC;
  tUChar HardKeyCPCKeyCodeD;

  // TRACE
  tUChar TraceDisplay;
  tUChar TraceInstruction;
  tUChar TraceStop;

} tNativeCPC;

#ifdef _DEBUG
#define STATIC_SIZE_NATIVECPC_DEBUG 0
#else /* _DEBUG */
#define STATIC_SIZE_NATIVECPC_DEBUG 0
#endif /* _DEBUG */

#define STATIC_SIZE_NATIVECPC ( STATIC_SIZE_NATIVECPC_DEBUG + \
                                sizeof(tZ80*) + /* Z80 */ \
                                sizeof(tCRTC*) + /* CRTC */ \
                                sizeof(tUChar*) + /* mode0_table */ \
                                sizeof(tUChar*) + /* mode1_table */ \
                                sizeof(tGateArray*) + /* GateArray */ \
                                sizeof(tFDC*) + /* FDC */ \
                                sizeof(fdc_cmd_table_def*) + /* FDCCommandTable */ \
                                sizeof(tPPI*) + /* PPI */ \
                                sizeof(tPSG*) + /* PSG */ \
                                sizeof(tVDU*) + /* VDU */ \
                                sizeof(tPreferences*) + /* prefP */ \
                                sizeof(tUChar*) + /* contextP */ \
                                sizeof(tUChar*) + /* pbGPBuffer */ \
                                sizeof(tUChar*) + /* pbRAM */ \
                                sizeof(tUChar**) + /* memmap_ROM */ \
                                sizeof(tUShort*) + /* DAATable */ \
                                sizeof(colours_rgb_entry*) + /* colours_rgb */ \
                                sizeof(colours_rgb_entry*) + /* colours_green */ \
                                sizeof(colours_rgb_entry*) + /* active_colours */ \
                                sizeof(tVoid*) + /* BmpOffScreenBits */ \
                                sizeof(tVoid*) + /* emulStateP */ \
                                sizeof(tVoid*) + /* call68KFuncP */ \
                                sizeof(tMemPtrNewFct) + /* MemPtrNewPtr */ \
                                sizeof(tMemPtrDeleteFct) +  /*MemPtrDeletePtr */ \
                                sizeof(tULong) + /* WinPalettePtr */ \
                                sizeof(tULong) + /* RGB565PalettePtr */ \
                                sizeof(tULong) + /* TraceAlertPtr */ \
                                sizeof(tULong) + /* SoundCalculateLevelTablesPtr */ \
                                sizeof(tVoid*) + /* hMemROMlo */ \
                                sizeof(tVoid*) + /* hMemAMSDOS */ \
                                sizeof(tUChar*) + /* pbROMlo */ \
                                sizeof(tUChar*) + /* pbROMhi */ \
                                sizeof(tUChar*) + /* pbExpansionROM */ \
                                sizeof(tULong) + /* FirstInitToPerform */ \
                                sizeof(tULong) + /* RestorationPerformed */ \
                                sizeof(tULong*) + /* OffscreenStartBits */ \
                                sizeof(tULong) + /* OffscreenCopyHeight */ \
                                sizeof(tULong) + /* OffscreenCopyWidth */ \
                                sizeof(tULong) + /* OffscreenAlignGap */ \
                                sizeof(tULong*) + /* OnscreenStartBits */ \
                                sizeof(tULong) + /* OnscreenPixelGap */ \
                                sizeof(tULong) /* OnscreenAlignGap */ )

#define DYNAMIC_SIZE_NATIVECPC  ( sizeof(tDrive*) + /* DriveA */ \
                                  sizeof(tDrive*) + /* DriveB */ \
                                  (sizeof(tUChar*) * NATIVECPC_MEMBANK_CONFIG_NBELEMENT * MEMBANK_NUMBER) + /* membank_config */ \
                                  (sizeof(tUChar*) * MEMBANK_NUMBER) + /* membank_read */ \
                                  (sizeof(tUChar*) * MEMBANK_NUMBER) + /* membank_write */ \
                                  sizeof(tUShort*) + /* pwTapePulseTable */ \
                                  sizeof(tUShort*) + /* pwTapePulseTableEnd */ \
                                  sizeof(tUShort*) + /* pwTapePulseTablePtr */ \
                                  sizeof(tUChar*) + /* pbTapeBlock */ \
                                  sizeof(tUChar*) + /* pbTapeBlockData */ \
                                  sizeof(tUChar*) + /* pbTapeImage */ \
                                  sizeof(tUChar*) /* pbTapeImageEnd */ )

#define DEBUG_SIZE_NATIVECPC    ( sizeof(tULong*) + /* OffscreenBits */ \
                                  sizeof(tULong*) + /* OnscreenBits */ \
                                  sizeof(tULong) /* debugNbCPCDisplay */ )
#define TRACE_SIZE_NATIVECPC    (0)
// ============================================================================


// ============================================================================
//
// tDiskOperation
//
#define CATALOGUE_FILENAME_SIZE                   8
#define CATALOGUE_FILEEXT_SIZE                    3
#define DISKOPERATION_MAX_CATALOGUE_ENTRIES       256 // ROMDOS D2, ROMDOS D20
#define CATALOGUE_MAX_BLOCKS                      1024

typedef struct
{
	tNativeCPC* NativeCPC;
	tDrive* Drive;
	tDiskFormat* DiskFormatTableP;
	tUChar* DiskContentP;
	tVoid* Param;
	tULong disk_size;
	tUChar FormatType;

  tULong NbCatalogueEntries;
  tULong CatalogueTrack;
  tUChar CatalogueEntries[DISKOPERATION_MAX_CATALOGUE_ENTRIES][CATALOGUE_FILENAME_SIZE + CATALOGUE_FILEEXT_SIZE + 1];
  tULong CatalogueEntrySize[DISKOPERATION_MAX_CATALOGUE_ENTRIES];
  tUChar CatalogueEntryUser[DISKOPERATION_MAX_CATALOGUE_ENTRIES];
} tDiskOperation;

#define DEBUG_SIZE_DISKOPERATION          (0)
#define TRACE_SIZE_DISKOPERATION          (0)
// ============================================================================


// ============================================================================
//
// tSoundCallbackParam
//
typedef struct
{
  //
  // Static pointers first (not affected by restoration)
  //
  tUChar* SoundBufferStartP;
  tUChar* SoundBufferEndP;
  tUChar** CurrentPosPP;
  tULong* CurrentSizeP;
  tUChar* LastPosP;
  tULong BufferRead;

  tULong StreamRef;

  //
  // 32bits, 16 bits then 8 bits variables
  //

  // DEBUG
  tULong DebugFlags;
  tULong DebugCount;
  tULong DebugTotalCount;
  tULong DebugSamples;
  tULong DebugMoveSizeCount;
  tULong DebugFullBufferReadCount;
  tULong DebugFrameCount;

} tSoundCallbackParam;

#define STATIC_SIZE_SOUND_CB_PARAM    ( sizeof(tUChar*) + /* SoundBufferStartP */ \
                                        sizeof(tUChar*) + /* SoundBufferEndP */ \
                                        sizeof(tUChar**) + /* CurrentPosPP */ \
                                        sizeof(tULong*)  + /* CurrentSizeP */ \
                                        sizeof(tUChar*)  + /* LastPosP */ \
                                        sizeof(tULong) + /* BufferRead */ \
                                        sizeof(tULong) /* StreamRef */ )
#define DYNAMIC_SIZE_SOUND_CB_PARAM   (0)
#define DEBUG_SIZE_SOUND_CB_PARAM     (0)
#define TRACE_SIZE_SOUND_CB_PARAM     (0)
// ============================================================================



// ============================================================================
//
// Memory Allocation tables sizes
//
#define MEMMAP_ROM_COUNT                256

#define SIZETAB_RAM                     ((tULong)(sizeof(tUChar) * 128 * 1024))  // 128Ko Max
#define SIZETAB_GPBUFFER                ((tULong)(sizeof(tUChar) * 4 * 1024))    // General Purpose RAM
#define SIZETAB_ROMLO                   ((tULong)(sizeof(tUChar) * 32 * 1024))
#define SIZETAB_MEMMAP_ROM              ((tULong)(sizeof(tUChar*) * MEMMAP_ROM_COUNT))
#define SIZETAB_Z80TABLE                ((tULong)(sizeof(tUChar) * 256))
#define SIZETAB_MODE0                   ((tULong)(sizeof(tUChar) * 512))
#define SIZETAB_MODE1                   ((tULong)(sizeof(tUChar) * 1024))
#define SIZETAB_SND_BUFFER              ((tULong)(sizeof(tUChar) * SND_BUFFER_SIZE))
#define SIZETAB_LEVEL_PP                ((tULong)(sizeof(tLong) * 256))
#define SIZETAB_LEVEL_AR                ((tULong)(sizeof(tLong) * 32))
#define SIZETAB_LEVEL_AL                ((tULong)(sizeof(tLong) * 32))
#define SIZETAB_LEVEL_BR                ((tULong)(sizeof(tLong) * 32))
#define SIZETAB_LEVEL_BL                ((tULong)(sizeof(tLong) * 32))
#define SIZETAB_LEVEL_CR                ((tULong)(sizeof(tLong) * 32))
#define SIZETAB_LEVEL_CL                ((tULong)(sizeof(tLong) * 32))
#define SIZETAB_FDCCOMMAND              ((tULong)(sizeof(fdc_cmd_table_def) * MAX_CMD_COUNT))
#define SIZETAB_VERSION                 ((tULong)(sizeof(tChar) * 20))

#define DEBUG_SIZE_SIZETAB              (0)
#define TRACE_SIZE_SIZETAB              (0)
// ============================================================================



// ============================================================================
//
// tContextHeader
//
#define CONTEXT_FLAG_PROFILE            0x00000001
#define CONTEXT_FLAG_DEBUG              0x00000002
#define CONTEXT_FLAG_TESTU              0x00000004
#define CONTEXT_FLAG_TRACE              0x00000008

typedef struct
{
  tLong Flags;
  tChar version[SIZETAB_VERSION];
  tUChar CPCModel;
  tUChar ScreenType;
  tUChar UseParados;
  tUChar Use64kMemoryExtension;
  tUChar EmuKeyFxState;
  tUChar EmuKeyJoystickState;
  tUChar EmuKeyAutoToggleState;
  tUChar Mode2AntiAliasing;
} tContextHeader;

#define DEBUG_SIZE_HEADER                 (0)
#define TRACE_SIZE_HEADER                 (0)
// ============================================================================


// ============================================================================
//
// tContextResources
//
typedef struct
{
  tVoid* BmpOffScreenBits; // WinCreateOffscreenWindow
  colours_rgb_entry* colours_rgb;   // (colours_rgb_entry*)MemHandleLock(colours_rgbH)
  colours_rgb_entry* colours_green; // (colours_rgb_entry*)MemHandleLock(colours_greenH)
  tUShort* DAATable; // (tUShort*)MemHandleLock(DAATableH)
  fdc_cmd_table_def* FDCCommandTableP;

  // Memory handles
  tVoid* colours_rgbH;
  tVoid* colours_greenH;
  tVoid* DAATableH;

  tUShort BmpOffScreenBytesRow; // BmpGetDimensions
} tContextResources;

#define DEBUG_SIZE_RESOURCES              (0)
#define TRACE_SIZE_RESOURCES              (0)
// ============================================================================


// ============================================================================
//
// Context sections definition
//
#define CONTEXT_OFFSET_HEADER             (0L)
#define CONTEXT_SIZE_HEADER               (DWORD_UPPER_ALIGN(sizeof(tContextHeader)))
#define CONTEXT_DEBUG_HEADER              (0L)
#define CONTEXT_TRACE_HEADER              (0L)
//
#define CONTEXT_OFFSET_RESOURCES          (CONTEXT_OFFSET_HEADER + CONTEXT_SIZE_HEADER)
#define CONTEXT_SIZE_RESOURCES            (DWORD_UPPER_ALIGN(sizeof(tContextResources)))
#define CONTEXT_DEBUG_RESOURCES           (CONTEXT_DEBUG_HEADER + DEBUG_SIZE_HEADER)
#define CONTEXT_TRACE_RESOURCES           (CONTEXT_TRACE_HEADER + TRACE_SIZE_HEADER)
//
#define CONTEXT_OFFSET_DISKOPERATION      (CONTEXT_OFFSET_RESOURCES + CONTEXT_SIZE_RESOURCES)
#define CONTEXT_SIZE_DISKOPERATION        (DWORD_UPPER_ALIGN(sizeof(tDiskOperation)))
#define CONTEXT_DEBUG_DISKOPERATION       (CONTEXT_DEBUG_RESOURCES + DEBUG_SIZE_RESOURCES)
#define CONTEXT_TRACE_DISKOPERATION       (CONTEXT_TRACE_RESOURCES + TRACE_SIZE_RESOURCES)
//
#define CONTEXT_OFFSET_NATIVECPC          (CONTEXT_OFFSET_DISKOPERATION + CONTEXT_SIZE_DISKOPERATION)
#define CONTEXT_SIZE_NATIVECPC            (DWORD_UPPER_ALIGN(sizeof(tNativeCPC)))
#define CONTEXT_DEBUG_NATIVECPC           (CONTEXT_DEBUG_DISKOPERATION + DEBUG_SIZE_DISKOPERATION)
#define CONTEXT_TRACE_NATIVECPC           (CONTEXT_TRACE_DISKOPERATION + TRACE_SIZE_DISKOPERATION)
//
#define CONTEXT_OFFSET_Z80                (CONTEXT_OFFSET_NATIVECPC + CONTEXT_SIZE_NATIVECPC)
#define CONTEXT_SIZE_Z80                  (DWORD_UPPER_ALIGN(sizeof(tZ80)))
#define CONTEXT_DEBUG_Z80                 (CONTEXT_DEBUG_NATIVECPC + DEBUG_SIZE_NATIVECPC)
#define CONTEXT_TRACE_Z80                 (CONTEXT_TRACE_NATIVECPC + TRACE_SIZE_NATIVECPC)
//
#define CONTEXT_OFFSET_PSG                (CONTEXT_OFFSET_Z80 + CONTEXT_SIZE_Z80)
#define CONTEXT_SIZE_PSG                  (DWORD_UPPER_ALIGN(sizeof(tPSG)))
#define CONTEXT_DEBUG_PSG                 (CONTEXT_DEBUG_Z80 + DEBUG_SIZE_Z80)
#define CONTEXT_TRACE_PSG                 (CONTEXT_TRACE_Z80 + TRACE_SIZE_Z80)
//
#define CONTEXT_OFFSET_FDC                (CONTEXT_OFFSET_PSG + CONTEXT_SIZE_PSG)
#define CONTEXT_SIZE_FDC                  (DWORD_UPPER_ALIGN(sizeof(tFDC)))
#define CONTEXT_DEBUG_FDC                 (CONTEXT_DEBUG_PSG + DEBUG_SIZE_PSG)
#define CONTEXT_TRACE_FDC                 (CONTEXT_TRACE_PSG + TRACE_SIZE_PSG)
//
#define CONTEXT_OFFSET_VDU                (CONTEXT_OFFSET_FDC + CONTEXT_SIZE_FDC)
#define CONTEXT_SIZE_VDU                  (DWORD_UPPER_ALIGN(sizeof(tVDU)))
#define CONTEXT_DEBUG_VDU                 (CONTEXT_DEBUG_FDC + DEBUG_SIZE_FDC)
#define CONTEXT_TRACE_VDU                 (CONTEXT_TRACE_FDC + TRACE_SIZE_FDC)
//
#define CONTEXT_OFFSET_CRTC               (CONTEXT_OFFSET_VDU + CONTEXT_SIZE_VDU)
#define CONTEXT_SIZE_CRTC                 (DWORD_UPPER_ALIGN(sizeof(tCRTC)))
#define CONTEXT_DEBUG_CRTC                (CONTEXT_DEBUG_VDU + DEBUG_SIZE_VDU)
#define CONTEXT_TRACE_CRTC                (CONTEXT_TRACE_VDU + TRACE_SIZE_VDU)
//
#define CONTEXT_OFFSET_Z80_SZ             (CONTEXT_OFFSET_CRTC + CONTEXT_SIZE_CRTC)
#define CONTEXT_SIZE_Z80_SZ               (DWORD_UPPER_ALIGN(SIZETAB_Z80TABLE))
#define CONTEXT_DEBUG_Z80_SZ              (CONTEXT_DEBUG_CRTC + DEBUG_SIZE_CRTC)
#define CONTEXT_TRACE_Z80_SZ              (CONTEXT_TRACE_CRTC + TRACE_SIZE_CRTC)
//
#define CONTEXT_OFFSET_Z80_SZ_BIT         (CONTEXT_OFFSET_Z80_SZ + CONTEXT_SIZE_Z80_SZ)
#define CONTEXT_SIZE_Z80_SZ_BIT           (DWORD_UPPER_ALIGN(SIZETAB_Z80TABLE))
#define CONTEXT_DEBUG_Z80_SZ_BIT          (CONTEXT_DEBUG_Z80_SZ + DEBUG_SIZE_SIZETAB)
#define CONTEXT_TRACE_Z80_SZ_BIT          (CONTEXT_TRACE_Z80_SZ + TRACE_SIZE_SIZETAB)
//
#define CONTEXT_OFFSET_Z80_SZP            (CONTEXT_OFFSET_Z80_SZ_BIT + CONTEXT_SIZE_Z80_SZ_BIT)
#define CONTEXT_SIZE_Z80_SZP              (DWORD_UPPER_ALIGN(SIZETAB_Z80TABLE))
#define CONTEXT_DEBUG_Z80_SZP             (CONTEXT_DEBUG_Z80_SZ_BIT + DEBUG_SIZE_SIZETAB)
#define CONTEXT_TRACE_Z80_SZP             (CONTEXT_TRACE_Z80_SZ_BIT + TRACE_SIZE_SIZETAB)
//
#define CONTEXT_OFFSET_Z80_SZHV_inc       (CONTEXT_OFFSET_Z80_SZP + CONTEXT_SIZE_Z80_SZP)
#define CONTEXT_SIZE_Z80_SZHV_inc         (DWORD_UPPER_ALIGN(SIZETAB_Z80TABLE))
#define CONTEXT_DEBUG_Z80_SZHV_inc        (CONTEXT_DEBUG_Z80_SZP + DEBUG_SIZE_SIZETAB)
#define CONTEXT_TRACE_Z80_SZHV_inc        (CONTEXT_TRACE_Z80_SZP + TRACE_SIZE_SIZETAB)
//
#define CONTEXT_OFFSET_Z80_SZHV_dec       (CONTEXT_OFFSET_Z80_SZHV_inc + CONTEXT_SIZE_Z80_SZHV_inc)
#define CONTEXT_SIZE_Z80_SZHV_dec         (DWORD_UPPER_ALIGN(SIZETAB_Z80TABLE))
#define CONTEXT_DEBUG_Z80_SZHV_dec        (CONTEXT_DEBUG_Z80_SZHV_inc + DEBUG_SIZE_SIZETAB)
#define CONTEXT_TRACE_Z80_SZHV_dec        (CONTEXT_TRACE_Z80_SZHV_inc + TRACE_SIZE_SIZETAB)
//
#define CONTEXT_OFFSET_MODE0_TABLE        (CONTEXT_OFFSET_Z80_SZHV_dec + CONTEXT_SIZE_Z80_SZHV_dec)
#define CONTEXT_SIZE_MODE0_TABLE          (DWORD_UPPER_ALIGN(SIZETAB_MODE0))
#define CONTEXT_DEBUG_MODE0_TABLE         (CONTEXT_DEBUG_Z80_SZHV_dec + DEBUG_SIZE_SIZETAB)
#define CONTEXT_TRACE_MODE0_TABLE         (CONTEXT_TRACE_Z80_SZHV_dec + TRACE_SIZE_SIZETAB)
//
#define CONTEXT_OFFSET_MODE1_TABLE        (CONTEXT_OFFSET_MODE0_TABLE + CONTEXT_SIZE_MODE0_TABLE)
#define CONTEXT_SIZE_MODE1_TABLE          (DWORD_UPPER_ALIGN(SIZETAB_MODE1))
#define CONTEXT_DEBUG_MODE1_TABLE         (CONTEXT_DEBUG_MODE0_TABLE + DEBUG_SIZE_SIZETAB)
#define CONTEXT_TRACE_MODE1_TABLE         (CONTEXT_TRACE_MODE0_TABLE + TRACE_SIZE_SIZETAB)
//
#define CONTEXT_OFFSET_GATEARRAY          (CONTEXT_OFFSET_MODE1_TABLE + CONTEXT_SIZE_MODE1_TABLE)
#define CONTEXT_SIZE_GATEARRAY            (DWORD_UPPER_ALIGN(sizeof(tGateArray)))
#define CONTEXT_DEBUG_GATEARRAY           (CONTEXT_DEBUG_MODE1_TABLE + DEBUG_SIZE_SIZETAB)
#define CONTEXT_TRACE_GATEARRAY           (CONTEXT_TRACE_MODE1_TABLE + TRACE_SIZE_SIZETAB)
//
#define CONTEXT_OFFSET_PPI                (CONTEXT_OFFSET_GATEARRAY + CONTEXT_SIZE_GATEARRAY)
#define CONTEXT_SIZE_PPI                  (DWORD_UPPER_ALIGN(sizeof(tPPI)))
#define CONTEXT_DEBUG_PPI                 (CONTEXT_DEBUG_GATEARRAY + DEBUG_SIZE_GATEARRAY)
#define CONTEXT_TRACE_PPI                 (CONTEXT_TRACE_GATEARRAY + TRACE_SIZE_GATEARRAY)
//
#define CONTEXT_OFFSET_PSG_LEVEL_PP       (CONTEXT_OFFSET_PPI + CONTEXT_SIZE_PPI)
#define CONTEXT_SIZE_PSG_LEVEL_PP         (DWORD_UPPER_ALIGN(SIZETAB_LEVEL_PP))
#define CONTEXT_DEBUG_PSG_LEVEL_PP        (CONTEXT_DEBUG_PPI + DEBUG_SIZE_PPI)
#define CONTEXT_TRACE_PSG_LEVEL_PP        (CONTEXT_TRACE_PPI + TRACE_SIZE_PPI)
//
#define CONTEXT_OFFSET_PSG_LEVEL_AR       (CONTEXT_OFFSET_PSG_LEVEL_PP + CONTEXT_SIZE_PSG_LEVEL_PP)
#define CONTEXT_SIZE_PSG_LEVEL_AR         (DWORD_UPPER_ALIGN(SIZETAB_LEVEL_AR))
#define CONTEXT_DEBUG_PSG_LEVEL_AR        (CONTEXT_DEBUG_PSG_LEVEL_PP + DEBUG_SIZE_SIZETAB)
#define CONTEXT_TRACE_PSG_LEVEL_AR        (CONTEXT_TRACE_PSG_LEVEL_PP + TRACE_SIZE_SIZETAB)
//
#define CONTEXT_OFFSET_PSG_LEVEL_AL       (CONTEXT_OFFSET_PSG_LEVEL_AR + CONTEXT_SIZE_PSG_LEVEL_AR)
#define CONTEXT_SIZE_PSG_LEVEL_AL         (DWORD_UPPER_ALIGN(SIZETAB_LEVEL_AL))
#define CONTEXT_DEBUG_PSG_LEVEL_AL        (CONTEXT_DEBUG_PSG_LEVEL_AR + DEBUG_SIZE_SIZETAB)
#define CONTEXT_TRACE_PSG_LEVEL_AL        (CONTEXT_TRACE_PSG_LEVEL_AR + TRACE_SIZE_SIZETAB)
//
#define CONTEXT_OFFSET_PSG_LEVEL_BR       (CONTEXT_OFFSET_PSG_LEVEL_AL + CONTEXT_SIZE_PSG_LEVEL_AL)
#define CONTEXT_SIZE_PSG_LEVEL_BR         (DWORD_UPPER_ALIGN(SIZETAB_LEVEL_BR))
#define CONTEXT_DEBUG_PSG_LEVEL_BR        (CONTEXT_DEBUG_PSG_LEVEL_AL + DEBUG_SIZE_SIZETAB)
#define CONTEXT_TRACE_PSG_LEVEL_BR        (CONTEXT_TRACE_PSG_LEVEL_AL + TRACE_SIZE_SIZETAB)
//
#define CONTEXT_OFFSET_PSG_LEVEL_BL       (CONTEXT_OFFSET_PSG_LEVEL_BR + CONTEXT_SIZE_PSG_LEVEL_BR)
#define CONTEXT_SIZE_PSG_LEVEL_BL         (DWORD_UPPER_ALIGN(SIZETAB_LEVEL_BL))
#define CONTEXT_DEBUG_PSG_LEVEL_BL        (CONTEXT_DEBUG_PSG_LEVEL_BR + DEBUG_SIZE_SIZETAB)
#define CONTEXT_TRACE_PSG_LEVEL_BL        (CONTEXT_TRACE_PSG_LEVEL_BR + TRACE_SIZE_SIZETAB)
//
#define CONTEXT_OFFSET_PSG_LEVEL_CR       (CONTEXT_OFFSET_PSG_LEVEL_BL + CONTEXT_SIZE_PSG_LEVEL_BL)
#define CONTEXT_SIZE_PSG_LEVEL_CR         (DWORD_UPPER_ALIGN(SIZETAB_LEVEL_CR))
#define CONTEXT_DEBUG_PSG_LEVEL_CR        (CONTEXT_DEBUG_PSG_LEVEL_BL + DEBUG_SIZE_SIZETAB)
#define CONTEXT_TRACE_PSG_LEVEL_CR        (CONTEXT_TRACE_PSG_LEVEL_BL + TRACE_SIZE_SIZETAB)
//
#define CONTEXT_OFFSET_PSG_LEVEL_CL       (CONTEXT_OFFSET_PSG_LEVEL_CR + CONTEXT_SIZE_PSG_LEVEL_CR)
#define CONTEXT_SIZE_PSG_LEVEL_CL         (DWORD_UPPER_ALIGN(SIZETAB_LEVEL_CL))
#define CONTEXT_DEBUG_PSG_LEVEL_CL        (CONTEXT_DEBUG_PSG_LEVEL_CR + DEBUG_SIZE_SIZETAB)
#define CONTEXT_TRACE_PSG_LEVEL_CL        (CONTEXT_TRACE_PSG_LEVEL_CR + TRACE_SIZE_SIZETAB)
//
#define CONTEXT_OFFSET_GPBUFFER           (CONTEXT_OFFSET_PSG_LEVEL_CL + CONTEXT_SIZE_PSG_LEVEL_CL)
#define CONTEXT_SIZE_GPBUFFER             (DWORD_UPPER_ALIGN(SIZETAB_GPBUFFER))
#define CONTEXT_DEBUG_GPBUFFER            (CONTEXT_DEBUG_PSG_LEVEL_CL + DEBUG_SIZE_SIZETAB)
#define CONTEXT_TRACE_GPBUFFER            (CONTEXT_TRACE_PSG_LEVEL_CL + TRACE_SIZE_SIZETAB)
//
#define CONTEXT_OFFSET_RAM                (CONTEXT_OFFSET_GPBUFFER + CONTEXT_SIZE_GPBUFFER)
#define CONTEXT_SIZE_RAM                  (DWORD_UPPER_ALIGN(SIZETAB_RAM))
#define CONTEXT_DEBUG_RAM                 (CONTEXT_DEBUG_GPBUFFER + DEBUG_SIZE_SIZETAB)
#define CONTEXT_TRACE_RAM                 (CONTEXT_TRACE_GPBUFFER + TRACE_SIZE_SIZETAB)
//
#define CONTEXT_OFFSET_SND_BUFFER         (CONTEXT_OFFSET_RAM + CONTEXT_SIZE_RAM)
#define CONTEXT_SIZE_SND_BUFFER           (DWORD_UPPER_ALIGN(SIZETAB_SND_BUFFER))
#define CONTEXT_DEBUG_SND_BUFFER          (CONTEXT_DEBUG_RAM + DEBUG_SIZE_SIZETAB)
#define CONTEXT_TRACE_SND_BUFFER          (CONTEXT_TRACE_RAM + TRACE_SIZE_SIZETAB)
//
#define CONTEXT_OFFSET_SESSIONFILENAME    (CONTEXT_OFFSET_SND_BUFFER + CONTEXT_SIZE_SND_BUFFER)
#define CONTEXT_SIZE_SESSIONFILENAME      (DWORD_UPPER_ALIGN(SIZETAB_FILENAME))
#define CONTEXT_DEBUG_SESSIONFILENAME     (CONTEXT_DEBUG_SND_BUFFER + DEBUG_SIZE_SIZETAB)
#define CONTEXT_TRACE_SESSIONFILENAME     (CONTEXT_TRACE_SND_BUFFER + TRACE_SIZE_SIZETAB)
//
#define CONTEXT_OFFSET_SOUND_CB_PARAM     (CONTEXT_OFFSET_SESSIONFILENAME + CONTEXT_SIZE_SESSIONFILENAME)
#define CONTEXT_SIZE_SOUND_CB_PARAM       (DWORD_UPPER_ALIGN(sizeof(tSoundCallbackParam)))
#define CONTEXT_DEBUG_SOUND_CB_PARAM      (CONTEXT_DEBUG_SESSIONFILENAME + DEBUG_SIZE_SIZETAB)
#define CONTEXT_TRACE_SOUND_CB_PARAM      (CONTEXT_TRACE_SESSIONFILENAME + TRACE_SIZE_SIZETAB)
//
#define CONTEXT_OFFSET_DRIVE_A            (CONTEXT_OFFSET_SOUND_CB_PARAM + CONTEXT_SIZE_SOUND_CB_PARAM)
#define CONTEXT_SIZE_DRIVE                (DWORD_UPPER_ALIGN(sizeof(tDrive)))
#define CONTEXT_SIZE_DRIVE_A              CONTEXT_SIZE_DRIVE
#define CONTEXT_DEBUG_DRIVE_A             (CONTEXT_DEBUG_SOUND_CB_PARAM + DEBUG_SIZE_SOUND_CB_PARAM)
#define CONTEXT_TRACE_DRIVE_A             (CONTEXT_TRACE_SOUND_CB_PARAM + TRACE_SIZE_SOUND_CB_PARAM)
//
#define CONTEXT_OFFSET_DRIVE_B            (CONTEXT_OFFSET_DRIVE_A + CONTEXT_SIZE_DRIVE_A)
#define CONTEXT_SIZE_DRIVE_B              CONTEXT_SIZE_DRIVE
#define CONTEXT_DEBUG_DRIVE_B             (CONTEXT_DEBUG_DRIVE_A + DEBUG_SIZE_DRIVE)
#define CONTEXT_TRACE_DRIVE_B             (CONTEXT_TRACE_DRIVE_A + TRACE_SIZE_DRIVE)
//
#define CONTEXT_OFFSET_MEMMAP_ROM         (CONTEXT_OFFSET_DRIVE_B + CONTEXT_SIZE_DRIVE_B)
#define CONTEXT_SIZE_MEMMAP_ROM           (DWORD_UPPER_ALIGN(SIZETAB_MEMMAP_ROM))
#define CONTEXT_DEBUG_MEMMAP_ROM          (CONTEXT_DEBUG_DRIVE_B + DEBUG_SIZE_DRIVE)
#define CONTEXT_TRACE_MEMMAP_ROM          (CONTEXT_TRACE_DRIVE_B + TRACE_SIZE_DRIVE)
//
#define SIZETAB_CONTEXT                   (CONTEXT_OFFSET_MEMMAP_ROM + CONTEXT_SIZE_MEMMAP_ROM)
//
// ============================================================================



//
// Unitary tests
//
// copied from ErrorBase.h
#define appErrorClass                     0x8000  // Application-defined errors

#define testUErrorClass                   appErrorClass


//
// Trace breakpoints
//
enum
{
  /*  0 */ TRACE_BP_read_mem = 0,
  /*  1 */ TRACE_BP_signed_read_mem,
  /*  2 */ TRACE_BP_write_mem,
  /*  3 */ TRACE_BP_PNOMain_registers,
  // Must be the last
  NUMBER_OF_TRACE_BP
};


//
// Profiling
//
#ifdef _PROFILE
#  define PROFILE_ADD_NATIVE(fn) if (NativeCPC) NativeCPC->profileCounter[fn]++;
#  define PROFILE_ADD_68K(fn) \
    if (NativeCPC) \
    { \
      NativeCPC->profileCounter[fn] = EndianSwap32(EndianSwap32(NativeCPC->profileCounter[fn]) + 1); \
    }
#else /* _PROFILE */
#  define PROFILE_ADD_NATIVE(fn)
#  define PROFILE_ADD_68K(fn)
#endif /* _PROFILE */


#endif /* ! NATIVE_CPC_H */

 //
 // Prototypes declarations
 //
extern tULong Engine_CPCExecute(tNativeCPC* NativeCPC);
extern tULong Engine_CPCReset(tNativeCPC* NativeCPC);
extern tULong Engine_CPCSetColor(tNativeCPC* NativeCPC);
extern tULong Engine_CPCStart(tNativeCPC* NativeCPC);
extern tULong Engine_CPCStop(tNativeCPC* NativeCPC);
extern tVoid Engine_DiskReadCatalogue(tDiskOperation* DiskOperation);
extern tULong Engine_DiskFormat(tDiskOperation* DiskOperation);
extern tULong Engine_DiskEject(tDiskOperation* DiskOperation);
extern tULong Engine_DiskLoad(tDiskOperation* DiskOperation);
extern tULong Engine_DiskSave(tDiskOperation* DiskOperation);
extern tULong Engine_DiskAutoStart(tDiskOperation* DiskOperation);
extern tULong Engine_SoundCallback(tSoundCallbackParam* paramP, tVoid* bufferP, tULong* bufferSizeP);
extern tULong Engine_WinCopyScreen(tNativeCPC* NativeCPC);
extern tVoid Sound_Calculate_Level_Tables(tNativeCPC* NativeCPC);

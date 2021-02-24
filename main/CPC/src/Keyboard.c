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



//#include "CaPriCe.h"

#include <stdio.h>
#include <string.h>

#include "types.h"
#include "sections.h"
#include "trace.h"

#include "Keyboard.h"

#include "Files.h"
#include "CPC.h"
#include "Prefs.h"
#include "Routines.h"

#include "event.h"
#include "backlight.h"

//#include "Display.h"
//#include "Resources.h"
//#include "Forms.h"
#include "Sound.h"

#include "gbuf.h"
#include "../rsc/vkb_keyboard.h"
#include "../rsc/vkb_cpcfont.h"

//===================
// PATCH begin
#ifdef _PATCH_ENABLE

#endif /* _PATCH_ENABLE */
// PATCH end
//===================

#define RGB565(r, g, b)  ( (((r>>3) & 0x1F) << 11) | (((g>>2) & 0x3F) << 5) | ((b>>3) & 0x1F) )

#ifdef SIM
#define ESWAP(a) (a)
#else
#define ESWAP(a) ( (a >> 8) | ((a) << 8) )
#endif

// Copied from Core/System/CharLatin.h
#define chrPoundSign            0x00A3

#define errNone                       0x0000  // No error
#define memErrorClass                 0x0100  // Memory Manager
// copied from MemoryMgr.h
#define memErrNotEnoughSpace          (memErrorClass | 2)
#define nativeFormat                  1

#define AUTOTOGGLE_DURATION_INDEX_DEFAULT     2


#define MOD_CPC_SHIFT   (0x01 << 8)
#define MOD_CPC_CTRL    (0x02 << 8)
#define MOD_EMU_KEY     (0x10 << 8)

#define KEY_INVALID               0xff
#define PRESSED_KEY_INVALID       0xffff

#undef KEYBOARD_TRACE_ENABLED
//#define KEYBOARD_TRACE_ENABLED

#ifdef KEYBOARD_TRACE_ENABLED
#  define DBG_PRINT(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s():" fmt, __FILE__, __LINE__, __func__, ##args)
#else
#  define DBG_PRINT(fmt, args...)
#endif /* CPCSTART_TRACE_ENABLED */

#define MemPtrNew(p) malloc(p)
#define MemPtrFree(p) free(p)

// Routines
static tVoid MemMove(tUChar* destP,
                     tUChar* sourceP,
                     tULong numBytes);

static const tUShort cpc_kbd[149] =
{
  // original CPC keyboard
  0x40,                   // CPC_KEY_0
  0x80,                   // CPC_KEY_1
  0x81,                   // CPC_KEY_2
  0x71,                   // CPC_KEY_3
  0x70,                   // CPC_KEY_4
  0x61,                   // CPC_KEY_5
  0x60,                   // CPC_KEY_6
  0x51,                   // CPC_KEY_7
  0x50,                   // CPC_KEY_8
  0x41,                   // CPC_KEY_9
  0x85 | MOD_CPC_SHIFT,   // CPC_KEY_A
  0x66 | MOD_CPC_SHIFT,   // CPC_KEY_B
  0x76 | MOD_CPC_SHIFT,   // CPC_KEY_C
  0x75 | MOD_CPC_SHIFT,   // CPC_KEY_D
  0x72 | MOD_CPC_SHIFT,   // CPC_KEY_E
  0x65 | MOD_CPC_SHIFT,   // CPC_KEY_F
  0x64 | MOD_CPC_SHIFT,   // CPC_KEY_G
  0x54 | MOD_CPC_SHIFT,   // CPC_KEY_H
  0x43 | MOD_CPC_SHIFT,   // CPC_KEY_I
  0x55 | MOD_CPC_SHIFT,   // CPC_KEY_J
  0x45 | MOD_CPC_SHIFT,   // CPC_KEY_K
  0x44 | MOD_CPC_SHIFT,   // CPC_KEY_L
  0x46 | MOD_CPC_SHIFT,   // CPC_KEY_M
  0x56 | MOD_CPC_SHIFT,   // CPC_KEY_N
  0x42 | MOD_CPC_SHIFT,   // CPC_KEY_O
  0x33 | MOD_CPC_SHIFT,   // CPC_KEY_P
  0x83 | MOD_CPC_SHIFT,   // CPC_KEY_Q
  0x62 | MOD_CPC_SHIFT,   // CPC_KEY_R
  0x74 | MOD_CPC_SHIFT,   // CPC_KEY_S
  0x63 | MOD_CPC_SHIFT,   // CPC_KEY_T
  0x52 | MOD_CPC_SHIFT,   // CPC_KEY_U
  0x67 | MOD_CPC_SHIFT,   // CPC_KEY_V
  0x73 | MOD_CPC_SHIFT,   // CPC_KEY_W
  0x77 | MOD_CPC_SHIFT,   // CPC_KEY_X
  0x53 | MOD_CPC_SHIFT,   // CPC_KEY_Y
  0x87 | MOD_CPC_SHIFT,   // CPC_KEY_Z
  0x85,                   // CPC_KEY_a
  0x66,                   // CPC_KEY_b
  0x76,                   // CPC_KEY_c
  0x75,                   // CPC_KEY_d
  0x72,                   // CPC_KEY_e
  0x65,                   // CPC_KEY_f
  0x64,                   // CPC_KEY_g
  0x54,                   // CPC_KEY_h
  0x43,                   // CPC_KEY_i
  0x55,                   // CPC_KEY_j
  0x45,                   // CPC_KEY_k
  0x44,                   // CPC_KEY_l
  0x46,                   // CPC_KEY_m
  0x56,                   // CPC_KEY_n
  0x42,                   // CPC_KEY_o
  0x33,                   // CPC_KEY_p
  0x83,                   // CPC_KEY_q
  0x62,                   // CPC_KEY_r
  0x74,                   // CPC_KEY_s
  0x63,                   // CPC_KEY_t
  0x52,                   // CPC_KEY_u
  0x67,                   // CPC_KEY_v
  0x73,                   // CPC_KEY_w
  0x77,                   // CPC_KEY_x
  0x53,                   // CPC_KEY_y
  0x87,                   // CPC_KEY_z
  0x60 | MOD_CPC_SHIFT,   // CPC_KEY_AMPERSAND
  0x35 | MOD_CPC_SHIFT,   // CPC_KEY_ASTERISK
  0x32,                   // CPC_KEY_AT
  0x26 | MOD_CPC_SHIFT,   // CPC_KEY_BACKQUOTE
  0x26,                   // CPC_KEY_BACKSLASH
  0x86,                   // CPC_KEY_CAPSLOCK
  0x20,                   // CPC_KEY_CLR
  0x35,                   // CPC_KEY_COLON
  0x47,                   // CPC_KEY_COMMA
  0x27,                   // CPC_KEY_CONTROL
  0x11,                   // CPC_KEY_COPY
  0x02 | MOD_CPC_SHIFT,   // CPC_KEY_CPY_DOWN
  0x10 | MOD_CPC_SHIFT,   // CPC_KEY_CPY_LEFT
  0x01 | MOD_CPC_SHIFT,   // CPC_KEY_CPY_RIGHT
  0x00 | MOD_CPC_SHIFT,   // CPC_KEY_CPY_UP
  0x02,                   // CPC_KEY_CUR_DOWN
  0x10,                   // CPC_KEY_CUR_LEFT
  0x01,                   // CPC_KEY_CUR_RIGHT
  0x00,                   // CPC_KEY_CUR_UP
  0x02 | MOD_CPC_CTRL,    // CPC_KEY_CUR_ENDBL
  0x10 | MOD_CPC_CTRL,    // CPC_KEY_CUR_HOMELN
  0x01 | MOD_CPC_CTRL,    // CPC_KEY_CUR_ENDLN
  0x00 | MOD_CPC_CTRL,    // CPC_KEY_CUR_HOMEBL
  0x81 | MOD_CPC_SHIFT,   // CPC_KEY_DBLQUOTE
  0x97,                   // CPC_KEY_DEL
  0x70 | MOD_CPC_SHIFT,   // CPC_KEY_DOLLAR
  0x06,                   // CPC_KEY_ENTER
  0x31 | MOD_CPC_SHIFT,   // CPC_KEY_EQUAL
  0x82,                   // CPC_KEY_ESC
  0x80 | MOD_CPC_SHIFT,   // CPC_KEY_EXCLAMATN
  0x17,                   // CPC_KEY_F0
  0x15,                   // CPC_KEY_F1
  0x16,                   // CPC_KEY_F2
  0x05,                   // CPC_KEY_F3
  0x24,                   // CPC_KEY_F4
  0x14,                   // CPC_KEY_F5
  0x04,                   // CPC_KEY_F6
  0x12,                   // CPC_KEY_F7
  0x13,                   // CPC_KEY_F8
  0x03,                   // CPC_KEY_F9
  0x07,                   // CPC_KEY_FPERIOD
  0x37 | MOD_CPC_SHIFT,   // CPC_KEY_GREATER
  0x71 | MOD_CPC_SHIFT,   // CPC_KEY_HASH
  0x21,                   // CPC_KEY_LBRACKET
  0x21 | MOD_CPC_SHIFT,   // CPC_KEY_LCBRACE
  0x50 | MOD_CPC_SHIFT,   // CPC_KEY_LEFTPAREN
  0x47 | MOD_CPC_SHIFT,   // CPC_KEY_LESS
  0x25,                   // CPC_KEY_LSHIFT
  0x31,                   // CPC_KEY_MINUS
  0x61 | MOD_CPC_SHIFT,   // CPC_KEY_PERCENT
  0x37,                   // CPC_KEY_PERIOD
  0x32 | MOD_CPC_SHIFT,   // CPC_KEY_PIPE
  0x34 | MOD_CPC_SHIFT,   // CPC_KEY_PLUS
  0x30 | MOD_CPC_SHIFT,   // CPC_KEY_POUND
  0x30,                   // CPC_KEY_POWER
  0x36 | MOD_CPC_SHIFT,   // CPC_KEY_QUESTION
  0x51 | MOD_CPC_SHIFT,   // CPC_KEY_QUOTE
  0x23,                   // CPC_KEY_RBRACKET
  0x23 | MOD_CPC_SHIFT,   // CPC_KEY_RCBRACE
  0x22,                   // CPC_KEY_RETURN
  0x41 | MOD_CPC_SHIFT,   // CPC_KEY_RIGHTPAREN
  0x25,                   // CPC_KEY_RSHIFT
  0x34,                   // CPC_KEY_SEMICOLON
  0x36,                   // CPC_KEY_SLASH
  0x57,                   // CPC_KEY_SPACE
  0x84,                   // CPC_KEY_TAB
  0x40 | MOD_CPC_SHIFT,   // CPC_KEY_UNDERSCORE
  0x90,                   // CPC_KEY_J0_UP
  0x91,                   // CPC_KEY_J0_DOWN
  0x92,                   // CPC_KEY_J0_LEFT
  0x93,                   // CPC_KEY_J0_RIGHT
  0x94,                   // CPC_KEY_J0_FIRE1
  0x95,                   // CPC_KEY_J0_FIRE2
  0x60,                   // CPC_KEY_J1_UP
  0x61,                   // CPC_KEY_J1_DOWN
  0x62,                   // CPC_KEY_J1_LEFT
  0x63,                   // CPC_KEY_J1_RIGHT
  0x64,                   // CPC_KEY_J1_FIRE1
  0x65,                   // CPC_KEY_J1_FIRE2
  KEY_INVALID,            // CPC_KEY_ES_NTILDE
  KEY_INVALID,            // CPC_KEY_ES_nTILDE
  KEY_INVALID,            // CPC_KEY_ES_PESETA
  KEY_INVALID,            // CPC_KEY_FR_eACUTE
  KEY_INVALID,            // CPC_KEY_FR_eGRAVE
  KEY_INVALID,            // CPC_KEY_FR_cCEDIL
  KEY_INVALID,            // CPC_KEY_FR_aGRAVE
  KEY_INVALID,            // CPC_KEY_FR_uGRAVE
};

//
// Copied from Core/System/Chars.h
//
static const tUChar palm_kbd[128] =
{
  KEY_INVALID,            // 0x0000 chrNull
  KEY_INVALID,            // 0x0001 chrStartOfHeading
  KEY_INVALID,            // 0x0002 chrStartOfText
  KEY_INVALID,            // 0x0003 chrEndOfText
  KEY_INVALID,            // 0x0004 chrEndOfTransmission
  KEY_INVALID,            // 0x0005 chrEnquiry
  KEY_INVALID,            // 0x0006 chrAcknowledge
  KEY_INVALID,            // 0x0007 chrBell
  CPC_KEY_DEL,            // 0x0008 chrBackspace
  CPC_KEY_TAB,            // 0x0009 chrHorizontalTabulation
  CPC_KEY_RETURN,         // 0x000A chrLineFeed
  KEY_INVALID,            // 0x000B chrVerticalTabulation
  KEY_INVALID,            // 0x000C chrFormFeed
  CPC_KEY_RETURN,         // 0x000D chrCarriageReturn
  KEY_INVALID,            // 0x000E chrShiftOut
  KEY_INVALID,            // 0x000F chrShiftIn
  KEY_INVALID,            // 0x0010 chrDataLinkEscape
  KEY_INVALID,            // 0x0011 chrDeviceControlOne
  KEY_INVALID,            // 0x0012 chrDeviceControlTwo
  KEY_INVALID,            // 0x0013 chrDeviceControlThree
  KEY_INVALID,            // 0x0014 chrDeviceControlFour
  KEY_INVALID,            // 0x0015 chrNegativeAcknowledge
  KEY_INVALID,            // 0x0016 chrSynchronousIdle
  KEY_INVALID,            // 0x0017 chrEndOfTransmissionBlock
  KEY_INVALID,            // 0x0018 chrCancel
  KEY_INVALID,            // 0x0019 chrEndOfMedium
  KEY_INVALID,            // 0x001A chrSubstitute
  CPC_KEY_ESC,            // 0x001B chrEscape
  KEY_INVALID,            // 0x001C chrFileSeparator
  KEY_INVALID,            // 0x001D chrGroupSeparator
  KEY_INVALID,            // 0x001E chrRecordSeparator
  KEY_INVALID,            // 0x001F chrUnitSeparator
  CPC_KEY_SPACE,          // 0x0020 chrSpace
  CPC_KEY_EXCLAMATN,      // 0x0021 chrExclamationMark
  CPC_KEY_DBLQUOTE,       // 0x0022 chrQuotationMark
  CPC_KEY_HASH,           // 0x0023 chrNumberSign
  CPC_KEY_DOLLAR,         // 0x0024 chrDollarSign
  CPC_KEY_PERCENT,        // 0x0025 chrPercentSign
  CPC_KEY_AMPERSAND,      // 0x0026 chrAmpersand
  CPC_KEY_QUOTE,          // 0x0027 chrApostrophe
  CPC_KEY_LEFTPAREN,      // 0x0028 chrLeftParenthesis
  CPC_KEY_RIGHTPAREN,     // 0x0029 chrRightParenthesis
  CPC_KEY_ASTERISK,       // 0x002A chrAsterisk
  CPC_KEY_PLUS,           // 0x002B chrPlusSign
  CPC_KEY_COMMA,          // 0x002C chrComma/
  CPC_KEY_MINUS,          // 0x002D chrHyphenMinus
  CPC_KEY_PERIOD,         // 0x002E chrFullStop/Period
  CPC_KEY_SLASH,          // 0x002F chrSolidus/Slash
  CPC_KEY_0,              // 0x0030 chrDigitZero
  CPC_KEY_1,              // 0x0031 chrDigitOne
  CPC_KEY_2,              // 0x0032 chrDigitTwo
  CPC_KEY_3,              // 0x0033 chrDigitThree
  CPC_KEY_4,              // 0x0034 chrDigitFour
  CPC_KEY_5,              // 0x0035 chrDigitFive
  CPC_KEY_6,              // 0x0036 chrDigitSix
  CPC_KEY_7,              // 0x0037 chrDigitSeven
  CPC_KEY_8,              // 0x0038 chrDigitEight
  CPC_KEY_9,              // 0x0039 chrDigitNine
  CPC_KEY_COLON,          // 0x003A chrColon
  CPC_KEY_SEMICOLON,      // 0x003B chrSemicolon
  CPC_KEY_LESS,           // 0x003C chrLessThanSign
  CPC_KEY_EQUAL,          // 0x003D chrEqualsSign
  CPC_KEY_GREATER,        // 0x003E chrGreaterThanSign
  CPC_KEY_QUESTION,       // 0x003F chrQuestionMark
  CPC_KEY_AT,             // 0x0040 chrCommercialAt
  CPC_KEY_A,              // 0x0041 chrCapital_A
  CPC_KEY_B,              // 0x0042 chrCapital_B
  CPC_KEY_C,              // 0x0043 chrCapital_C
  CPC_KEY_D,              // 0x0044 chrCapital_D
  CPC_KEY_E,              // 0x0045 chrCapital_E
  CPC_KEY_F,              // 0x0046 chrCapital_F
  CPC_KEY_G,              // 0x0047 chrCapital_G
  CPC_KEY_H,              // 0x0048 chrCapital_H
  CPC_KEY_I,              // 0x0049 chrCapital_I
  CPC_KEY_J,              // 0x004A chrCapital_J
  CPC_KEY_K,              // 0x004B chrCapital_K
  CPC_KEY_L,              // 0x004C chrCapital_L
  CPC_KEY_M,              // 0x004D chrCapital_M
  CPC_KEY_N,              // 0x004E chrCapital_N
  CPC_KEY_O,              // 0x004F chrCapital_O
  CPC_KEY_P,              // 0x0050 chrCapital_P
  CPC_KEY_Q,              // 0x0051 chrCapital_Q
  CPC_KEY_R,              // 0x0052 chrCapital_R
  CPC_KEY_S,              // 0x0053 chrCapital_S
  CPC_KEY_T,              // 0x0054 chrCapital_T
  CPC_KEY_U,              // 0x0055 chrCapital_U
  CPC_KEY_V,              // 0x0056 chrCapital_V
  CPC_KEY_W,              // 0x0057 chrCapital_W
  CPC_KEY_X,              // 0x0058 chrCapital_X
  CPC_KEY_Y,              // 0x0059 chrCapital_Y
  CPC_KEY_Z,              // 0x005A chrCapital_Z
  CPC_KEY_LBRACKET,       // 0x005B chrLeftSquareBracket
  CPC_KEY_BACKSLASH,      // 0x005C chrReverseSolidus
  CPC_KEY_RBRACKET,       // 0x005D chrRightSquareBracket
  KEY_INVALID,            // 0x005E chrCircumflexAccent
  CPC_KEY_UNDERSCORE,     // 0x005F chrLowLine
  KEY_INVALID,            // 0x0060 chrGraveAccent
  CPC_KEY_a,              // 0x0061 chrSmall_A
  CPC_KEY_b,              // 0x0062 chrSmall_B
  CPC_KEY_c,              // 0x0063 chrSmall_C
  CPC_KEY_d,              // 0x0064 chrSmall_D
  CPC_KEY_e,              // 0x0065 chrSmall_E
  CPC_KEY_f,              // 0x0066 chrSmall_F
  CPC_KEY_g,              // 0x0067 chrSmall_G
  CPC_KEY_h,              // 0x0068 chrSmall_H
  CPC_KEY_i,              // 0x0069 chrSmall_I
  CPC_KEY_j,              // 0x006A chrSmall_J
  CPC_KEY_k,              // 0x006B chrSmall_K
  CPC_KEY_l,              // 0x006C chrSmall_L
  CPC_KEY_m,              // 0x006D chrSmall_M
  CPC_KEY_n,              // 0x006E chrSmall_N
  CPC_KEY_o,              // 0x006F chrSmall_O
  CPC_KEY_p,              // 0x0070 chrSmall_P
  CPC_KEY_q,              // 0x0071 chrSmall_Q
  CPC_KEY_r,              // 0x0072 chrSmall_R
  CPC_KEY_s,              // 0x0073 chrSmall_S
  CPC_KEY_t,              // 0x0074 chrSmall_T
  CPC_KEY_u,              // 0x0075 chrSmall_U
  CPC_KEY_v,              // 0x0076 chrSmall_V
  CPC_KEY_w,              // 0x0077 chrSmall_W
  CPC_KEY_x,              // 0x0078 chrSmall_X
  CPC_KEY_y,              // 0x0079 chrSmall_Y
  CPC_KEY_z,              // 0x007A chrSmall_Z
  CPC_KEY_LCBRACE,        // 0x007B chrLeftCurlyBracket
  CPC_KEY_PIPE,           // 0x007C chrVerticalLine
  CPC_KEY_RCBRACE,        // 0x007D chrRightCurlyBracket
  KEY_INVALID,            // 0x007E chrTilde
  CPC_KEY_CLR,            // 0x007F chrDelete
};


static Boolean DetectPressedKey(tKey* keyP,
                                tUShort* keySaveP);
static Boolean ReleaseLastPressedKey(void);
static Boolean KeyDownHandleKeyboardEvent(tKeyboard* keyboardP,
                                          event_t* Event);


static void EnableFonctionKeys(void) SECTION_KEYBOARD;
static void DisableFonctionKeys(void) SECTION_KEYBOARD;
static void SHIFTKeyPressed(void) SECTION_KEYBOARD;
static void CTRLKeyPressed(void)SECTION_KEYBOARD;

static void InitKeysPanel(tKey* keyP,
                          UInt8 nbKeys,
                          const tKeySetting* settingsP,
                          tUChar DefaultHighlight) SECTION_KEYBOARD;

static Err StartKeyboard(tKeyboard* keyboardP);
static tResult PrepareOffscreenKey(tKey* keyP, tKeyStatus KeyStatus);
static void PrepareChars(const tUChar* String, tUShort Length, tUShort Left, tUShort Top);

static tUShort usCurrentPressedKey = PRESSED_KEY_INVALID;
static tUShort usSaveShiftCtrl = 0;
static UInt32 debounceTimeoutTicks = 0;
static tUShort usKeyState = 0;
static UInt32 oldScreenKey = 0;
static tKey* lastPressedKeyP = NULL;
static tUShort* lastPressedKeyStateP = NULL;
static UInt32 u32DebounceInTicks = 0;

gbuf_t* KeyBoardOffScreenBuffer = NULL;
RGBColorType* MiniKeyboardColorMapP = NULL;
UInt16* MiniKeyboardColorMap565P = NULL;

tResult PrepareKeyboard(tKeyboard* keyboardP);

//
// Auto toggle
//
UInt8 AutoToggleActive = 0;
UInt8 AutoToggleDurationInTicks = 0;
UInt8 AutoToggleDurationIndex = 0;
static UInt32 u32AutoToggleKeyState = keyBitRockerLeft;
static UInt32 u32OldRockerStateWhileAutoToggle = keyBitRockerLeft;
tRockerAttach NewRockerAttach = RockerAsJoyOrCursors;
tRockerAttach OldRockerAttach = RockerAsJoyOrCursors;

static const UInt8 AutoToggleDurationA[] =
{ 0  /*End*/,
	30,
	16 /*Default*/,
	12,
	9,
	6,
	0  /*End*/ };


//
// Keys
//
#define FIRST_ICON_X            5
#define ICONS_Y                 10
#define ICONS_SUB_Y             1
#define BACKGROUND_Y            9
#define BACKGROUND_SUB_Y        0

#define ARROW_X                 287
#define ARROW_Y                 1
#define ARROW_WIDTH             29
#define ARROW_HEIGHT            38

#define KEY_ICON_WIDTH          32
#define KEY_ICON_HEIGHT         32

#define EMU_BACKGROUND_WIDTH    54
#define CPC_BACKGROUND_1_WIDTH  8
#define CPC_BACKGROUND_2_WIDTH  180
#define BACKGROUND_HEIGHT       34

#define ESC_KEY_WIDTH           32
#define COPY_KEY_WIDTH          32
#define SHIFT_KEY_WIDTH         40
#define CTRL_KEY_WIDTH          32
#define CAPSLOCK_KEY_WIDTH      40
#define TAB_KEY_WIDTH           32
#define DEL_KEY_WIDTH           32
#define ENTER_KEY_WIDTH         40

#define RESET_KEY_X             FIRST_ICON_X
#define PAUSE_KEY_X             RESET_KEY_X + KEY_ICON_WIDTH
#define TRUESPEED_KEY_X         PAUSE_KEY_X + KEY_ICON_WIDTH
#define SOUND_KEY_X             TRUESPEED_KEY_X + KEY_ICON_WIDTH
#define SOUND_VOLUME_KEY_X      SOUND_KEY_X + KEY_ICON_WIDTH
#define JOYSTICK_KEY_X          SOUND_VOLUME_KEY_X + KEY_ICON_WIDTH
#define AUTOTOGGLE_KEY_X        JOYSTICK_KEY_X + KEY_ICON_WIDTH
#define EMU_BACKGROUND_X        AUTOTOGGLE_KEY_X + KEY_ICON_WIDTH
#define SUBPANEL_KEY_X          EMU_BACKGROUND_X + EMU_BACKGROUND_WIDTH
//
#define ESC_KEY_X               FIRST_ICON_X
#define COPY_KEY_X              ESC_KEY_X + ESC_KEY_WIDTH
#define SHIFT_KEY_X             COPY_KEY_X + COPY_KEY_WIDTH
#define CTRL_KEY_X              SHIFT_KEY_X + SHIFT_KEY_WIDTH
#define CAPSLOCK_KEY_X          CTRL_KEY_X + CTRL_KEY_WIDTH
#define TAB_KEY_X               CAPSLOCK_KEY_X + CAPSLOCK_KEY_WIDTH
#define DEL_KEY_X               TAB_KEY_X + TAB_KEY_WIDTH
#define ENTER_KEY_X             DEL_KEY_X + DEL_KEY_WIDTH
//
#define CLR_KEY_X               FIRST_ICON_X
#define CPC_BACKGROUND_1_X      CLR_KEY_X + KEY_ICON_WIDTH
#define FX_KEY_X                CPC_BACKGROUND_1_X + CPC_BACKGROUND_1_WIDTH
#define EMUSPEED_KEY_X          FX_KEY_X + KEY_ICON_WIDTH
#define CPC_BACKGROUND_2_X      EMUSPEED_KEY_X + KEY_ICON_WIDTH


static const tKeySetting DriveKeysSettings[] =
{
	// tKeySetting
	// Left, Top, Width, Height,
	// resKeyUp, resKeyDown,
	// KeyMode, CPCKey, charCode_Normal, charCode_Shift,
	// OnPushedP, OnReleasedP, KeyHighlight

  // Drive Swap Key
  // { 286, 4, 32, 32,
  //   DriveKey_Swap_Released, DriveKey_Swap_Pushed,
  //   KeyImpulse, CPC_KEY_NONE, 0, 0,
  //   CPCSwapDrive, NULL},
  // // Disk Eject Key
  // { 2, 17, 26, 20,
  //   DriveKey_Eject_Released, DriveKey_Eject_Pushed,
  //   KeyImpulse, CPC_KEY_NONE, 0, 0,
  //   CPCEjectBoth, NULL},
  // // Drive A Key
  // { 30, 1, 254, 18,
  //   NULL, NULL,
  //   KeyImpulse, CPC_KEY_NONE, 0, 0,
  //   ChangeDriveAContent, NULL},
  // // Drive B Key
  // { 30, 20, 254, 18,
  //   NULL, NULL,
  //   KeyImpulse, CPC_KEY_NONE, 0, 0,
  //   ChangeDriveBContent, NULL},
};


static const tKeySetting EmulatorKeysSettings[] =
{
	// tKeySetting
	// Left, Top, Width, Height,
	// resKeyUp, resKeyDown,
	// KeyMode, CPCKey, charCode_Normal, charCode_Shift,
	// OnPushedP, OnReleasedP

  // Reset Key
  // { RESET_KEY_X, ICONS_Y, KEY_ICON_WIDTH, KEY_ICON_HEIGHT,
  //   EmuKey_Reset_Released, EmuKey_Reset_Pushed,
  //   KeyImpulse, CPC_KEY_NONE, 0, 0,
  //   NULL, CPCResetWithConfirmation },
  // // Pause Key
  // { PAUSE_KEY_X, ICONS_Y, KEY_ICON_WIDTH, KEY_ICON_HEIGHT,
  //   EmuKey_Pause_Released, EmuKey_Pause_Pushed,
  //   KeyToggle, CPC_KEY_NONE, 0, 0,
  //   EmulatorFreeze, EmulatorUnfreeze },
  // // True Speed Key
  // { TRUESPEED_KEY_X, ICONS_Y, KEY_ICON_WIDTH, KEY_ICON_HEIGHT,
  //   EmuKey_TrueSpeed_Released, EmuKey_TrueSpeed_Pushed,
  //   KeyToggle, CPC_KEY_NONE, 0, 0,
  //   EnableTrueSpeed, DisableTrueSpeed },
  // // Sound Key
  // { SOUND_KEY_X, ICONS_Y, KEY_ICON_WIDTH, KEY_ICON_HEIGHT,
  //   EmuKey_Sound_Released, EmuKey_Sound_Pushed,
  //   KeyToggle, CPC_KEY_NONE, 0, 0,
  //   EnableSound, DisableSound },
  // // Sound Volume Key
  // { SOUND_VOLUME_KEY_X, ICONS_Y, KEY_ICON_WIDTH, KEY_ICON_HEIGHT,
  //   EmuKey_Sound_Volume_Released, EmuKey_Sound_Volume_Pushed,
  //   KeyToggle, CPC_KEY_NONE, 0, 0,
  //   AdjustSoundVolumeKeyPressed, AdjustSoundVolumeKeyReleased },
  // // Joystick Key
  // { JOYSTICK_KEY_X, ICONS_Y, KEY_ICON_WIDTH, KEY_ICON_HEIGHT,
  //   EmuKey_Joystick_Released, EmuKey_Joystick_Pushed,
  //   KeyToggle, CPC_KEY_NONE, 0, 0,
  //   EnableJoystick, DisableJoystick },
  // // AutoToggle Key
  // { AUTOTOGGLE_KEY_X, ICONS_Y, KEY_ICON_WIDTH, KEY_ICON_HEIGHT,
  //   CPCKey_AutoToggle_Released, CPCKey_AutoToggle_Pushed,
  //   KeyToggle, CPC_KEY_NONE, 0, 0,
  //   AutoToggleKeyPressed, AutoToggleKeyReleased },
  // // Background
  // { EMU_BACKGROUND_X, BACKGROUND_Y, EMU_BACKGROUND_WIDTH, BACKGROUND_HEIGHT,
  //   EmuKey_Background, EmuKey_Background,
  //   KeyStatic, CPC_KEY_NONE, 0, 0,
  //   NULL, NULL },
};


static const tKeySetting CPCColouredKeysSettings[] =
{
	// tKeySetting
  /*
  tUShort Left;
  tUShort Top;
  tUShort Width;
  tUShort Height;
  DmResID resKeyUp;
  DmResID resKeyDown;
  tKeyMode KeyMode;
  tUChar CPCKey;
  tUChar charCode_Normal;
  tUChar charCode_Shift;
  void (*OnPushedP)(void);
  void (*OnReleasedP)(void);
  tUChar NextLeft;
  tUChar NextRight;
  tUChar NextUp;
  tUChar NextDown;
  tUChar yBase;
  */


  // ESC Key
  { ESC_KEY_X, ICONS_SUB_Y, ESC_KEY_WIDTH, KEY_ICON_HEIGHT,
    CPCKey_ESC_Released, CPCKey_ESC_Pushed,
    KeyImpulse, CPC_KEY_ESC, 0, 0,
    NULL, NULL,0,0,0,0,0},
  // COPY Key
  { COPY_KEY_X, ICONS_SUB_Y, COPY_KEY_WIDTH, KEY_ICON_HEIGHT,
    CPCKey_COPY_Released, CPCKey_COPY_Pushed,
    KeyImpulse, CPC_KEY_COPY, 0, 0,
    NULL, NULL,0,0,0,0,0 },
  // SHIFT Key
  { SHIFT_KEY_X, ICONS_SUB_Y, SHIFT_KEY_WIDTH, KEY_ICON_HEIGHT,
    CPCKey_SHIFT_Released, CPCKey_SHIFT_Pushed,
    KeyToggle, CPC_KEY_NONE, 0, 0,
    SHIFTKeyPressed, SHIFTKeyPressed,0,0,0,0,0 },
  // CTRL Key
  { CTRL_KEY_X, ICONS_SUB_Y, CTRL_KEY_WIDTH, KEY_ICON_HEIGHT,
    CPCKey_CTRL_Released, CPCKey_CTRL_Pushed,
    KeyToggle, CPC_KEY_NONE, 0, 0,
    CTRLKeyPressed, CTRLKeyPressed,0,0,0,0,0 },
  // CAPSLOCK Key
  { CAPSLOCK_KEY_X, ICONS_SUB_Y, CAPSLOCK_KEY_WIDTH, KEY_ICON_HEIGHT,
    CPCKey_CAPSLOCK_Released, CPCKey_CAPSLOCK_Pushed,
    KeyImpulse, CPC_KEY_CAPSLOCK, 0, 0,
    NULL, NULL,0,0,0,0,0 },
  // TAB Key
  { TAB_KEY_X, ICONS_SUB_Y, TAB_KEY_WIDTH, KEY_ICON_HEIGHT,
    CPCKey_TAB_Released, CPCKey_TAB_Pushed,
    KeyImpulse, CPC_KEY_TAB, 0, 0,
    NULL, NULL,0,0,0,0,0 },
  // DEL Key
  { DEL_KEY_X, ICONS_SUB_Y, DEL_KEY_WIDTH, KEY_ICON_HEIGHT,
    CPCKey_DEL_Released, CPCKey_DEL_Pushed,
    KeyImpulse, CPC_KEY_DEL, 0, 0,
    NULL, NULL,0,0,0,0,0 },
  // ENTER Key
  { ENTER_KEY_X, ICONS_SUB_Y, ENTER_KEY_WIDTH, KEY_ICON_HEIGHT,
    CPCKey_ENTER_Released, CPCKey_ENTER_Pushed,
    KeyImpulse, CPC_KEY_ENTER, 0, 0,
    NULL, NULL,0,0,0,0,0 },
  // Top Arrow
  { ARROW_X, ARROW_Y, ARROW_WIDTH, ARROW_HEIGHT,
    0, 0,
    KeyImpulse, CPC_KEY_NONE, 0, 0,
    NULL, NULL,0,0,0,0,0 },
};

/*
static const tKeySetting CPCSpecialKeysSettings[] =
{
	// tKeySetting
	// Left, Top, Width, Height,
	// resKeyUp, resKeyDown,
	// KeyMode, CPCKey, charCode_Normal, charCode_Shift,
	// OnPushedP, OnReleasedP

  // CLR Key
  { CLR_KEY_X, ICONS_SUB_Y, KEY_ICON_WIDTH, KEY_ICON_HEIGHT,
    CPCKey_CLR_Released, CPCKey_CLR_Pushed,
    KeyImpulse, CPC_KEY_CLR, 0, 0,
    NULL, NULL },
  // Background 1
  { CPC_BACKGROUND_1_X, BACKGROUND_SUB_Y, CPC_BACKGROUND_1_WIDTH, BACKGROUND_HEIGHT,
    CPCKey_Background_1, CPCKey_Background_1,
    KeyStatic, CPC_KEY_NONE, 0, 0,
    NULL, NULL },
  // Fx Key
  { FX_KEY_X, ICONS_SUB_Y, KEY_ICON_WIDTH, KEY_ICON_HEIGHT,
    EmuKey_Fx_Released, EmuKey_Fx_Pushed,
    KeyToggle, CPC_KEY_NONE, 0, 0,
    EnableFonctionKeys, DisableFonctionKeys },
  // Display Emulator Speed Key
  // { EMUSPEED_KEY_X, ICONS_SUB_Y, KEY_ICON_WIDTH, KEY_ICON_HEIGHT,
  //   EmuKey_EmuSpeed_Released, EmuKey_EmuSpeed_Pushed,
  //   KeyToggle, CPC_KEY_NONE, 0, 0,
  //   EnableDisplayEmuSpeed, DisableDisplayEmuSpeed },
  // Background 2
  { CPC_BACKGROUND_2_X, BACKGROUND_SUB_Y, CPC_BACKGROUND_2_WIDTH, BACKGROUND_HEIGHT,
    CPCKey_Background_2, CPCKey_Background_2,
    KeyStatic, CPC_KEY_NONE, 0, 0,
    NULL, NULL },
  // Arrow
  // { ARROW_X, ARROW_Y, ARROW_WIDTH, ARROW_HEIGHT,
  //   NULL, NULL,
  //   KeyImpulse, CPC_KEY_NONE, 0, 0,
  //   DisplayCPCColouredKeyboard, NULL },
};

*/


tKey DriveKeys[NB_DRIVE_KEYS];
tKey EmulatorKeys[NB_EMULATORS_KEYS];
tKey CPCColouredKeys[NB_CPC_COLOURED_KEYS];
tKey CPCSpecialKeys[NB_CPC_SPECIAL_KEYS];

tKey* EmulatorKeysPanelP = CPCColouredKeys;
tULong NbEmulatorKeys = NB_CPC_COLOURED_KEYS;

tEmulatorKeysStatus EmulatorKeysStatus;

static tUChar cpcKeyUp;
static tUChar cpcKeyDown;
static tUChar cpcKeyRight;
static tUChar cpcKeyLeft;
static tUChar cpcKeyCenter;      // A
static tUChar cpcKeyCenterEx;    // B
static tUChar cpcKeyStart;       // Start
static tUChar cpcKeySelect;      // Select

//
// CPC Keyboards
//
tKeyboard* MiniKeyboardP = NULL;
tKeyboard* ActiveKeyboardP = NULL;

typedef struct
{
  UInt32* KeycodeMaskP;
  UInt32  HardKeyIndex;
} tUpdateCPCKeycode;

void logkeys(void)
{
  int key1,key2;
  for (int i=0;i<CPC_KB_KEYS;i++)
  {
    key1=MiniKeyboardP->keysP[i].CPCKey;
    key2=MiniKeyboardP->keysP[i].settingP->CPCKey;
    printf("Keycode: %2d %3d",i,key1);
    printf(" %3d %3d\n",key2,key1-key2);
  }
}

tUShort GetKeySHIFTandCTRLState(tVoid)
/***********************************************************************
 *
 *  GetKeySHIFTandCTRLState
 *
 ***********************************************************************/
{
tUShort usKeyStatus = 0;

  // Save SHIFT state
  if (IS_KEY_PRESSED(CPC_KBD_SHIFT))
  {
    usKeyStatus |= MOD_CPC_SHIFT;
  }

  // Save CTRL state
  if (IS_KEY_PRESSED(CPC_KBD_CTRL))
  {
    usKeyStatus |= MOD_CPC_CTRL;
  }

  return usKeyStatus;
}
/*----------------------------------------------------------------------------*/


void SetKeySHIFTandCTRLState(tUShort usKeyState)
/***********************************************************************
 *
 *  SetKeySHIFTandCTRLState
 *
 ***********************************************************************/
{
  if (usKeyState & MOD_CPC_SHIFT) // CPC SHIFT key required?
  {
    PRESS_KEY(CPC_KBD_SHIFT); // key needs to be SHIFTed
  }
  else
  {
    RELEASE_KEY(CPC_KBD_SHIFT); // make sure key is unSHIFTed
  }

  if (usKeyState & MOD_CPC_CTRL) // CPC CONTROL key required?
  {
    PRESS_KEY(CPC_KBD_CTRL); // CONTROL key is held down
  }
  else
  {
    RELEASE_KEY(CPC_KBD_CTRL); // // make sure CONTROL key is released
  }
}
/*----------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------*/


Boolean KeyDownHandleEvent(event_t* Event)
/***********************************************************************
 *
 *  KeyDownHandleEvent
 *
 ***********************************************************************/
{

// Main virtual keyboard event loop.
// Still need to detect inf keypad is now liked to the virtual keyboard
if (1)
{
    //
    // Mini Keyboard
    //
      KeyDownHandleKeyboardEvent(MiniKeyboardP, Event);
      return true;
  }

  return false;
}
/*----------------------------------------------------------------------------*/


static Boolean KeyDownHandleKeyboardEvent(tKeyboard* keyboardP,
                                          event_t* Event)
/***********************************************************************
 *
 *  KeyDownHandleKeyboardEvent
 *
 ***********************************************************************/
{
Boolean Handled = false;
tKey* lastkeyP = keyboardP->keyCurrentP;
const tKeySetting* lastSettingP = lastkeyP->settingP;

if (Event->keypad.pressed & KEYPAD_A)
  Handled = DetectPressedKey(lastkeyP, &usSaveShiftCtrl);
else {
  if (Event->keypad.pressed & KEYPAD_RIGHT)
    keyboardP->keyCurrentP = &keyboardP->keysP[lastSettingP->NextRight];

  if (Event->keypad.pressed & KEYPAD_LEFT)
      keyboardP->keyCurrentP = &keyboardP->keysP[lastSettingP->NextLeft];

  if (Event->keypad.pressed & KEYPAD_UP)
    keyboardP->keyCurrentP = &keyboardP->keysP[lastSettingP->NextUp];

  if (Event->keypad.pressed & KEYPAD_DOWN)
      keyboardP->keyCurrentP = &keyboardP->keysP[lastSettingP->NextDown];
  }

  // update current key position on the keyboard by highlight red
  if (lastkeyP != keyboardP->keyCurrentP) {
    PrepareOffscreenKey(keyboardP->keyCurrentP, KeyPressed);

  // do not cancle special key status when moved over it
  if (!(((lastkeyP->CPCKey == CPC_KEY_LSHIFT) && (lastkeyP->KeyStatus == KeyPressed)) ||
    ((lastkeyP->CPCKey == CPC_KEY_RSHIFT) && (lastkeyP->KeyStatus == KeyPressed)) ||
    ((lastkeyP->CPCKey == CPC_KEY_CONTROL) && (lastkeyP->KeyStatus == KeyPressed)))) {
    PrepareOffscreenKey(lastkeyP, KeyReleased);
    }
  }
  return Handled;
}
/*----------------------------------------------------------------------------*/
UInt8 GetCurrentKeyBaseCoordY()
/***********************************************************************
 *
 *  GetCurrentKeyBaseCoordY
 *
 ***********************************************************************/
 {
   if (MiniKeyboardP != NULL)
    return MiniKeyboardP->keyCurrentP->settingP->yBase;
   else
    return 0;
 }


/*----------------------------------------------------------------------------*/
Boolean KeyUpHandleEvent(event_t* Event)
/***********************************************************************
 *
 *  KeyUpHandleEvent
 *
 ***********************************************************************/
{
Boolean Handled = false;

  if (Event->keypad.released & KEYPAD_A)
  {
    printf("ReleaseLastPressedKey entered\n");
    Handled = ReleaseLastPressedKey();
  }

  // Screen keys
  if (oldScreenKey != 0)
  {
    SetCursorAndJoystickState(0,&oldScreenKey);
  }

  return Handled;
}
/*----------------------------------------------------------------------------*/



static Boolean DetectPressedKey(tKey* keyP,
                                tUShort* keySaveP)
/***********************************************************************
 *
 *  DetectPressedKey
 *
 ***********************************************************************/
{
const tKeySetting* settingP = keyP->settingP;
tUChar keyCode;
printf("DetectPressedKey entered %p\n",keyP);

#ifndef __RELEASE__
  if (keyP == NULL)
    DBG_PRINT("keyP == NULL");
#endif /* __RELEASE__ */

  if (settingP->KeyMode == KeyStatic)
    return false;

  if (lastPressedKeyP)
    return false;

  keyCode = (tUChar)cpc_kbd[keyP->CPCKey];

  // Pen down on that key
  if ( (settingP->KeyMode == KeyImpulse) ||
       ((settingP->KeyMode == KeyToggle) && (keyP->KeyStatus == KeyReleased)) )
  {
    keyP->KeyStatus = KeyPressed;

    if (settingP->KeyMode == KeyImpulse)
    {
      lastPressedKeyP = keyP;
      lastPressedKeyStateP = keySaveP;
    }

    // Execute associated routine
    if (keyP->OnPushedP != NULL)
    {
      keyP->OnPushedP();
    }

    // Simulate associated CPC Key press
    if (keyP->CPCKey != CPC_KEY_NONE)
    {
      if (keySaveP)
      {
        // Save pressed key, SHIFT and CTRL state
        *keySaveP = GetKeySHIFTandCTRLState();
      }

      // Update hardware matrix
      printf("Emulate! %d\n",keyP->settingP->CPCKey);
      //for (int i=0;i<NATIVECPC_KEYBOARD_MATRIX_NBELEMENT;i++)
      //  printf("%d ",NativeCPC->keyboard_matrix[i]);
      //printf("\n");
      PRESS_KEY(keyCode);
      debounceTimeoutTicks = TimGetTicks() + u32DebounceInTicks;
    }
  }
  else if ((settingP->KeyMode == KeyToggle) && (keyP->KeyStatus == KeyPressed))
  {
    keyP->KeyStatus = KeyReleased;

    // Execute associated routine
    if (keyP->OnReleasedP != NULL)
    {
      keyP->OnReleasedP();
    }

    // Simulate associated CPC Key release
    if (settingP->CPCKey != CPC_KEY_NONE)
    {
      // Update hardware keyboard matrix
      RELEASE_KEY(keyCode);
    }
  }
  else
  {
  	return true;
  }

  // todo? On pressing do another visual effect?
  if ( settingP->resKeyUp && settingP->resKeyDown )
  {
    //PrepareOffscreenKey(keyP);
  }
  else
  {
    // Invert key display
    //INVERT_KEY_DISPLAY(keyP)
  }

  return true;
}
/*----------------------------------------------------------------------------*/



static Boolean ReleaseLastPressedKey(void)
/***********************************************************************
 *
 *  ReleaseLastPressedKey
 *
 ***********************************************************************/
{
const tKeySetting* settingP;

  if (lastPressedKeyP == NULL)
    return false;

  settingP = lastPressedKeyP->settingP;
  lastPressedKeyP->KeyStatus = KeyReleased;

  // Execute associated routine
  if (lastPressedKeyP->OnReleasedP != NULL)
  {
    lastPressedKeyP->OnReleasedP();
  }

  // Simulate associated CPC Key release
  if (lastPressedKeyP->CPCKey != CPC_KEY_NONE)
  {
    printf("Release key: %p, %d\n",lastPressedKeyP,lastPressedKeyP->CPCKey);
    printf("Release key settings: %p, %d\n",lastPressedKeyP,lastPressedKeyP->settingP->CPCKey);


    RELEASE_KEY(cpc_kbd[settingP->CPCKey]);

    if (lastPressedKeyStateP)
    {
      // Restore SHIFT and CTRL state
      SetKeySHIFTandCTRLState(*lastPressedKeyStateP);
    }
  }


  if ( settingP->resKeyUp && settingP->resKeyDown )
  {
    //PrepareOffscreenKey(lastPressedKeyP);
  }
  else
  {
    // Invert key display
    //INVERT_KEY_DISPLAY(lastPressedKeyP)
  }

  lastPressedKeyP = NULL;
  lastPressedKeyStateP = NULL;

  return true;
}
/*----------------------------------------------------------------------------*/

void SetSessionKeyMapping(tNativeCPC* NativeCPC)
{
  NativeCPC->SessionCPCKeyUp = cpcKeyUp;
  NativeCPC->SessionCPCKeyDown = cpcKeyDown;
  NativeCPC->SessionCPCKeyRight = cpcKeyRight;
  NativeCPC->SessionCPCKeyLeft = cpcKeyLeft;
  NativeCPC->SessionCPCKeyCenter = cpcKeyCenter;
  NativeCPC->SessionCPCKeyCenterEx = cpcKeyCenterEx;
  NativeCPC->SessionCPCKeyStart = cpcKeyStart;
  NativeCPC->SessionCPCKeySelect = cpcKeySelect;
}

void GetSessionKeyMapping(tNativeCPC* NativeCPC)
{
  cpcKeyUp = NativeCPC->SessionCPCKeyUp;
  cpcKeyDown = NativeCPC->SessionCPCKeyDown;
  cpcKeyRight = NativeCPC->SessionCPCKeyRight;
  cpcKeyLeft = NativeCPC->SessionCPCKeyLeft;
  cpcKeyCenter = NativeCPC->SessionCPCKeyCenter;
  cpcKeyCenterEx = NativeCPC->SessionCPCKeyCenterEx;
  cpcKeyStart = NativeCPC->SessionCPCKeyStart;
  cpcKeySelect = NativeCPC->SessionCPCKeySelect;
}

tUShort SetKeyMapping(char* mappingString)
{
    if (!strcmp(mappingString, "JST_UP")) return(CPC_KEY_J0_UP);
    if (!strcmp(mappingString, "JST_RIGHT")) return(CPC_KEY_J0_RIGHT);
    if (!strcmp(mappingString, "JST_DOWN")) return(CPC_KEY_J0_DOWN);
    if (!strcmp(mappingString, "JST_LEFT")) return(CPC_KEY_J0_LEFT);
    if (!strcmp(mappingString, "JST_FIREA")) return(CPC_KEY_J0_FIRE1);
    if (!strcmp(mappingString, "JST_FIREB")) return(CPC_KEY_J0_FIRE2);

    if (!strcmp(mappingString, "JST2_UP")) return(CPC_KEY_J1_UP);
    if (!strcmp(mappingString, "JST2_RIGHT")) return(CPC_KEY_J1_RIGHT);
    if (!strcmp(mappingString, "JST2_DOWN")) return(CPC_KEY_J1_DOWN);
    if (!strcmp(mappingString, "JST2_LEFT")) return(CPC_KEY_J1_LEFT);
    if (!strcmp(mappingString, "JST2_FIREA")) return(CPC_KEY_J1_FIRE1);
    if (!strcmp(mappingString, "JST2_FIREB")) return(CPC_KEY_J1_FIRE2);

    if (!strcmp(mappingString, "KBD_SPACE")) return(CPC_KEY_SPACE);

    if (!strcmp(mappingString, "KBD_F0")) return(CPC_KEY_F0);
    if (!strcmp(mappingString, "KBD_F1")) return(CPC_KEY_F1);
    if (!strcmp(mappingString, "KBD_F2")) return(CPC_KEY_F2);
    if (!strcmp(mappingString, "KBD_F3")) return(CPC_KEY_F3);
    if (!strcmp(mappingString, "KBD_F4")) return(CPC_KEY_F4);
    if (!strcmp(mappingString, "KBD_F5")) return(CPC_KEY_F5);
    if (!strcmp(mappingString, "KBD_F6")) return(CPC_KEY_F6);
    if (!strcmp(mappingString, "KBD_F7")) return(CPC_KEY_F7);
    if (!strcmp(mappingString, "KBD_F8")) return(CPC_KEY_F8);
    if (!strcmp(mappingString, "KBD_F9")) return(CPC_KEY_F9);

    if (!strcmp(mappingString, "KBD_COPY")) return(CPC_KEY_COPY);
    if (!strcmp(mappingString, "KBD_LEFT")) return(CPC_KEY_CUR_LEFT);
    if (!strcmp(mappingString, "KBD_UP")) return(CPC_KEY_CUR_UP);
    if (!strcmp(mappingString, "KBD_DOWN")) return(CPC_KEY_CUR_DOWN);
    if (!strcmp(mappingString, "KBD_RIGHT")) return(CPC_KEY_CUR_RIGHT);
    if (!strcmp(mappingString, "KBD_LSHIFT")) return(CPC_KEY_LSHIFT);
    if (!strcmp(mappingString, "KBD_RSHIFT")) return(CPC_KEY_RSHIFT);
    if (!strcmp(mappingString, "KBD_CONTROL")) return(CPC_KEY_CONTROL);
    if (!strcmp(mappingString, "KBD_BS")) return(CPC_KEY_BACKSLASH);
    if (!strcmp(mappingString, "KBD_TAB")) return(CPC_KEY_TAB);
    if (!strcmp(mappingString, "KBD_CAPSLOCK")) return(CPC_KEY_CAPSLOCK);
    if (!strcmp(mappingString, "KBD_HOME")) return(CPC_KEY_CUR_HOMELN);
    if (!strcmp(mappingString, "KBD_ENTER")) return(CPC_KEY_ENTER);
    if (!strcmp(mappingString, "KBD_RETURN")) return(CPC_KEY_RETURN);

    if (!strcmp(mappingString, "KBD_ESCAPE")) return(CPC_KEY_ESC);

    // handle single Chars
    return(palm_kbd[(mappingString[0] & 0x7F)]);
}

void EnableSpecialKeymapping(void)
/***********************************************************************
 *
 *  EnableSpecialKeymapping
 *
 ***********************************************************************/
 {
   Err Result;
   char* Settings = NULL;
   const char* KeyMap[8] = {"UP","RIGHT","DOWN","LEFT","SELECT","START","A","B"};

   for (int i=0;i<8;i++)
   {
     Result = CPCLoadKeymapFromConfigFile(KeyMap[i], &Settings);
     if (Result == errNone)
     {
       //printf("KeyMap for %s = %s\n",KeyMap[i], Settings);
       if (i==0) {cpcKeyUp = cpc_kbd[SetKeyMapping(Settings)]; continue;}
       if (i==1) {cpcKeyRight = cpc_kbd[SetKeyMapping(Settings)]; continue;}
       if (i==2) {cpcKeyDown = cpc_kbd[SetKeyMapping(Settings)]; continue;}
       if (i==3) {cpcKeyLeft = cpc_kbd[SetKeyMapping(Settings)]; continue;}
       if (i==4) {cpcKeySelect = cpc_kbd[SetKeyMapping(Settings)]; continue;}
       if (i==5) {cpcKeyStart = cpc_kbd[SetKeyMapping(Settings)]; continue;}
       if (i==6) {cpcKeyCenter = cpc_kbd[SetKeyMapping(Settings)]; continue;}
       if (i==7) {cpcKeyCenterEx = cpc_kbd[SetKeyMapping(Settings)]; continue;}
     }
   }
   SetSessionKeyMapping(NativeCPC);
 }

void EnableJoystick(void)
/***********************************************************************
 *
 *  EnableJoystick
 *
 ***********************************************************************/
{
UInt32 OldKeyState = ~0;

  // Release potential pressed keys
  SetCursorAndJoystickState(0,
                            &OldKeyState);

  EmulatorKeysStatus.JoystickKeyStatus = KeyPressed;

  // Joystick
  cpcKeyUp = cpc_kbd[CPC_KEY_J0_UP];
  cpcKeyDown = cpc_kbd[CPC_KEY_J0_DOWN];
  cpcKeyRight = cpc_kbd[CPC_KEY_J0_RIGHT];
  cpcKeyLeft = cpc_kbd[CPC_KEY_J0_LEFT];
  cpcKeyCenter = cpc_kbd[CPC_KEY_J0_FIRE1];
  SetSessionKeyMapping(NativeCPC);

}
/*----------------------------------------------------------------------------*/
void DisableJoystick(void)
/***********************************************************************
 *
 *  DisableJoystick
 *
 ***********************************************************************/
{
UInt32 OldKeyState = ~0;

const tUChar CenterKeys[] =
{
  CPC_KEY_ESC,
  CPC_KEY_RETURN,
  CPC_KEY_COPY,
  CPC_KEY_CONTROL,
  CPC_KEY_RSHIFT,
  CPC_KEY_SPACE,
  CPC_KEY_TAB,
  CPC_KEY_CAPSLOCK
};

  // Release potential pressed keys
  SetCursorAndJoystickState(0,&OldKeyState);

  EmulatorKeysStatus.JoystickKeyStatus = KeyReleased;

  // Cursor
  cpcKeyUp = cpc_kbd[CPC_KEY_CUR_UP];
  cpcKeyDown = cpc_kbd[CPC_KEY_CUR_DOWN];
  cpcKeyRight = cpc_kbd[CPC_KEY_CUR_RIGHT];
  cpcKeyLeft = cpc_kbd[CPC_KEY_CUR_LEFT];
  cpcKeyCenter = cpc_kbd[CenterKeys[prefP->RockerCenterKeyIndex]];
  SetSessionKeyMapping(NativeCPC);

}
/*----------------------------------------------------------------------------*/



static void EnableFonctionKeys(void)
/***********************************************************************
 *
 *  EnableFonctionKeys
 *
 ***********************************************************************/
{
  EmulatorKeysStatus.FxKeyStatus = KeyPressed;
}
/*----------------------------------------------------------------------------*/
static void DisableFonctionKeys(void)
/***********************************************************************
 *
 *  DisableFonctionKeys
 *
 ***********************************************************************/
{
  EmulatorKeysStatus.FxKeyStatus = KeyReleased;
}
/*----------------------------------------------------------------------------*/


static void SHIFTKeyPressed(void)
/***********************************************************************
 *
 *  SHIFTKeyPressed
 *
 ***********************************************************************/
{
  if ( //(CPCColouredKeys[KEYINDEX_SHIFT].KeyStatus == KeyPressed) ||
       (ActiveKeyboardP->keyRShiftP->KeyStatus == KeyPressed) ||
       (ActiveKeyboardP->keyLShiftP->KeyStatus == KeyPressed) )
  {
    // SHIFT is pressed
    PRESS_KEY(CPC_KBD_SHIFT);
  }
  else
  {
    // SHIFT is released
    RELEASE_KEY(CPC_KBD_SHIFT);
  }

	// Todo Update Display
  CPCPushEvent(CapriceEventKeyboardRedraw);
  printf("Push CapriceEventKeyboardRedraw\n");

}
/*----------------------------------------------------------------------------*/


static void CTRLKeyPressed(void)
/***********************************************************************
 *
 *  CTRLKeyPressed
 *
 ***********************************************************************/
{
  if (ActiveKeyboardP->keyCTRLP->KeyStatus == KeyPressed)
  {
    // CTRL is pressed
    PRESS_KEY(CPC_KBD_CTRL);
  }
  else
  {
    // CTRL is released
    RELEASE_KEY(CPC_KBD_CTRL);
  }

	// Update form display
  CPCPushEvent(CapriceEventKeyboardRedraw);
  printf("Push CapriceEventKeyboardRedraw\n");
}
/*----------------------------------------------------------------------------*/


void AutoToggleKeyPressed(void)
/***********************************************************************
 *
 *  AutoToggleKeyPressed
 *
 ***********************************************************************/
{
  AutoToggleActive = 1;
  AutoToggleDurationIndex = AUTOTOGGLE_DURATION_INDEX_DEFAULT;
  AutoToggleDurationInTicks = AutoToggleDurationA[AutoToggleDurationIndex];
}
/*----------------------------------------------------------------------------*/
void AutoToggleKeyReleased(void)
/***********************************************************************
 *
 *  AutoToggleKeyReleased
 *
 ***********************************************************************/
{
  AutoToggleActive = 0;
}
/*----------------------------------------------------------------------------*/


void SetCursorAndJoystickState(UInt32 keyState,
                               UInt32* oldkeystateP)
/***********************************************************************
 *
 *  SetCursorAndJoystickState
 *
 ***********************************************************************/
{
UInt32 keyDiff;

  //
  // Auto toggle Left-Right
  //
  if (AutoToggleActive)
  {
    keyDiff = keyState ^ u32OldRockerStateWhileAutoToggle;

    // Auto toggle duration update
    if (keyDiff)
    {
      u32OldRockerStateWhileAutoToggle = keyState;

      // Rocker RIGHT pushed = duration divided by 2
      if (keyDiff & keyBitRockerRight)
      {
        if (keyState & keyBitRockerRight)
        {
          if (AutoToggleDurationA[AutoToggleDurationIndex+1])
          {
            AutoToggleDurationInTicks = AutoToggleDurationA[++AutoToggleDurationIndex];
          }
        }
      }
      // Rocker LEFT pushed = duration multiply by 2
      else if (keyDiff & keyBitRockerLeft)
      {
        if (keyState & keyBitRockerLeft)
        {
          if (AutoToggleDurationA[AutoToggleDurationIndex-1])
          {
            AutoToggleDurationInTicks = AutoToggleDurationA[--AutoToggleDurationIndex];
          }
        }
      }
    }

    // Add only left and right simulated rocker status
    keyState &= ~(keyBitRockerRight | keyBitRockerLeft);
    keyState |= u32AutoToggleKeyState;
  }


  //
  // Check if rocker status changed
  //
  keyDiff = keyState ^ *oldkeystateP;
  if (!keyDiff)
    return;


  // Save key states
  *oldkeystateP = keyState;

  //
  // Adjust sound volume
  //
  if (keyState & (keyBitRockerVolume))
  {
  	if (!AutoToggleActive)
  	{
      if (keyDiff & (keyBitRockerRight))
      {
      	// Rocker RIGHT pushed = Volume raised
        if (keyState & (keyBitRockerRight) )
        {
          SoundIncreaseVolume(1);
          return;
        }
      }
      else if (keyDiff & (keyBitRockerUp))
      {
        // Rocker UP pushed = Volums fast raised
        if (keyState & (keyBitRockerUp))
        {
          SoundIncreaseVolume(10);
          return;
        }
      }
      else if (keyDiff & (keyBitRockerLeft))
      {
        // Rocker LEFT pushed = Volums lowered
        if (keyState & (keyBitRockerLeft))
        {
          SoundDecreaseVolume(1);
          return;
        }
      }
      else if (keyDiff & (keyBitRockerDown))
      {
        // Rocker DOWN pushed = Volums lowered
        if (keyState & (keyBitRockerDown))
        {
          SoundDecreaseVolume(10);
          return;
        }
      }
  	}

    if (prefP->SoundRenderer == 0)
      CPCPushEvent(CapriceEventVolumeOkEvent);
    else
      CPCPushEvent(CapriceEventVolumeFailEvent);
  }

  //
  // Brightness adjust
  //

  if (keyState & (keyBitRockerMenuOnly))
  {
    if (!AutoToggleActive)
    {
      if (keyDiff & (keyBitRockerRight))
      {
      	// Rocker RIGHT pushed = Brightness raised
        if (keyState & (keyBitRockerRight) )
        {
          backlight_percentage_increase(1);
          CPCPushEvent(CapriceEventBrightnessOkEvent);
          return;
        }
      }
      else if (keyDiff & (keyBitRockerUp))
      {
        // Rocker UP pushed = Brightness fast raised
        if (keyState & (keyBitRockerUp))
        {
          backlight_percentage_increase(10);
          CPCPushEvent(CapriceEventBrightnessOkEvent);
          return;
        }
      }
      else if (keyDiff & (keyBitRockerLeft))
      {
        // Rocker LEFT pushed = Brightness lowered
        if (keyState & (keyBitRockerLeft))
        {
          backlight_percentage_decrease(1);
          CPCPushEvent(CapriceEventBrightnessOkEvent);
          return;
        }
      }
      else if (keyDiff & (keyBitRockerDown))
      {
        // Rocker DOWN pushed = Brightness lowered
        if (keyState & (keyBitRockerDown))
        {
          backlight_percentage_decrease(10);
          CPCPushEvent(CapriceEventBrightnessOkEvent);
          return;
        }
      }
  	}
  }

  //
  // Rocker attached to?
  //
  if (NewRockerAttach == RockerAsJoyOrCursors)
  {
    // Not MENU button pressed?
    if ((keyState & keyBitRockerMenuOnly) == 0)
    {
      // Rocker UP...
      if (keyDiff & (keyBitRockerUp))
      {
        if (keyState & (keyBitRockerUp))
        {
          PRESS_KEY(cpcKeyUp);
        }
        else
        {
          RELEASE_KEY(cpcKeyUp);
        }
      }

      // Rocker DOWN.
      if (keyDiff & (keyBitRockerDown))
      {
        if (keyState & (keyBitRockerDown))
        {
          PRESS_KEY(cpcKeyDown);
        }
        else
        {
          RELEASE_KEY(cpcKeyDown);
        }
      }

      // Rocker RIGHT...
      if (keyDiff & keyBitRockerRight)
      {
        if (keyState & keyBitRockerRight)
        {
          PRESS_KEY(cpcKeyRight);
        }
        else
        {
          RELEASE_KEY(cpcKeyRight);
        }
      }

      // Rocker LEFT.
      if (keyDiff & keyBitRockerLeft)
      {
        if (keyState & keyBitRockerLeft)
        {
          PRESS_KEY(cpcKeyLeft);
        }
        else
        {
          RELEASE_KEY(cpcKeyLeft);
        }
      }

      // Rocker Center A
      if (keyDiff & keyBitRockerButtonA)
      {
        if (keyState & keyBitRockerButtonA)
        {
          PRESS_KEY(cpcKeyCenter);
        }
        else
        {
          RELEASE_KEY(cpcKeyCenter);
        }
      }

      // Rocker Center B
      if (keyDiff & (keyBitRockerButtonB))
      {
        if (keyState & (keyBitRockerButtonB))
        {
          PRESS_KEY(cpcKeyCenterEx);
        }
        else
        {
          RELEASE_KEY(cpcKeyCenterEx);
        }
      }

      // Rocker Select
      if (keyDiff & (keyBitRockerSelect))
      {
        if (keyState & (keyBitRockerSelect))
        {
          PRESS_KEY(cpcKeySelect);
        }
        else
        {
          RELEASE_KEY(cpcKeySelect);
        }
      }

      // Rocker Start
      if (keyDiff & (keyBitRockerStart))
      {
        if (keyState & (keyBitRockerStart))
        {
          PRESS_KEY(cpcKeyStart);
        }
        else
        {
          RELEASE_KEY(cpcKeyStart);
        }
      }

    } // Keypad check
  } // (NewRockerAttach == RockerAsJoyOrCursors)
}
/*----------------------------------------------------------------------------*/


void ReleasePressedKey(void)
/***********************************************************************
 *
 *  ReleasePressedKey
 *
 ***********************************************************************/
{
Boolean KeyDown = true;

  if (usCurrentPressedKey == 0xffff)
    return;

  if (TimGetTicks() < debounceTimeoutTicks)
    return;

  if (KeyDown == false)
  {
    RELEASE_KEY(usCurrentPressedKey); // key has been released

    // Restore SHIFT and CTRL state
    SetKeySHIFTandCTRLState(usCurrentPressedKey);

    usCurrentPressedKey = 0xffff;
  }
}
/*----------------------------------------------------------------------------*/

void RockerAttachManager(UInt32 keyState,
                         UInt32* oldkeystateP)
/***********************************************************************
 *
 *  RockerAttachMenuConfig
 *
 ***********************************************************************/
{
  UInt32 keyDiff;
  //
  // Check if rocker status changed
  //
  keyDiff = keyState ^ *oldkeystateP;
  if (!keyDiff)
    return;

  // Save key states
  *oldkeystateP = keyState;

  // Menu Navigation
  if ((keyState & (keyBitRockerMenu)) ==  (keyBitRockerMenu))
  {
    printf("Start Menu Config\n");
    if ( (NewRockerAttach == RockerAsJoyOrCursors) || NewRockerAttach == RockerAsVirtualKeyboard )
    {
      //Enable File and Config Menu
      OldRockerAttach = NewRockerAttach;
      NewRockerAttach = RockerAsMenuControl;
      EmulatorFreeze();
      CPCPushEvent(CapriceEventMenuRedraw);
      return;
    }
    if ( (NewRockerAttach == RockerAsMenuControl) && (OldRockerAttach == RockerAsJoyOrCursors) )
    {
      printf("EMU + Joystick/Cursor mode \n");
      // Go back to the normal EMU + Joystick/Cursor mode
      OldRockerAttach = NewRockerAttach;
      NewRockerAttach = RockerAsJoyOrCursors;
      EmulatorUnfreeze();
      CPCPushEvent(CapriceEventRestartAudioPipe);
      return;
    }

    if ( (NewRockerAttach == RockerAsMenuControl) && (OldRockerAttach == RockerAsVirtualKeyboard) )
    {
      printf("EMU + virtual Keyboard mode \n");
      // Go back to the normal EMU + Keyboard mode
      OldRockerAttach = NewRockerAttach;
      NewRockerAttach = RockerAsVirtualKeyboard;
      EmulatorUnfreeze();
      CPCPushEvent(CapriceEventRestartAudioPipe);
      return;
    }
  }
  // Virtual keyboard navigation
  if ((keyState & (keyBitRockerViKeyboard )) == (keyBitRockerViKeyboard ))
  {
    if (NewRockerAttach == RockerAsJoyOrCursors)
    {
        printf("EMU + virtual Keyboard mode \n");
        OldRockerAttach = NewRockerAttach;
        NewRockerAttach = RockerAsVirtualKeyboard;
        CPCPushEvent(CapriceEventKeyboardRedraw);
        return;
    }

    if (NewRockerAttach == RockerAsVirtualKeyboard)
    {
      printf("EMU + Joystick/Cursor mode \n");
      OldRockerAttach = NewRockerAttach;
      NewRockerAttach = RockerAsJoyOrCursors;
    }
  }
}
/*----------------------------------------------------------------------------*/

void EnableSound(void)
/***********************************************************************
 *
 *  EnableSound
 *
 ***********************************************************************/
{

  prefP->SoundEnabled = 1;
  prefP->PreferencesChanged = 1;

	// Should be done after prefP update
  SoundPlay(NativeCPC);

}
/*----------------------------------------------------------------------------*/
void DisableSound(void)
/***********************************************************************
 *
 *  DisableSound
 *
 ***********************************************************************/
{

  SoundPause(NativeCPC);

  prefP->SoundEnabled = 0;
  prefP->PreferencesChanged = 1;

}
/*----------------------------------------------------------------------------*/

void InitKeys(void)
/***********************************************************************
 *
 *  InitKeys
 *
 ***********************************************************************/
{
  // Drive panel
  InitKeysPanel(DriveKeys,
                NB_DRIVE_KEYS,
                DriveKeysSettings,
                0);

  // Emulator panel
  InitKeysPanel(EmulatorKeys,
                NB_EMULATORS_KEYS,
                EmulatorKeysSettings,
                0);

  // Sub panel
  InitKeysPanel(CPCColouredKeys,
                NB_CPC_COLOURED_KEYS,
                CPCColouredKeysSettings,
                0);
  //InitKeysPanel(CPCSpecialKeys,
  //              NB_CPC_SPECIAL_KEYS,
  //              CPCSpecialKeysSettings,
  //              0);

  EmulatorKeysStatus.SoundKeyStatus = KeyReleased;
  EmulatorKeysStatus.FxKeyStatus = KeyReleased;
  EmulatorKeysStatus.JoystickKeyStatus = KeyReleased;
  EmulatorKeysStatus.AutoToggleKeyStatus = KeyReleased;
}
/*----------------------------------------------------------------------------*/


static void InitKeysPanel(tKey* keyP,
                          UInt8 nbKeys,
                          const tKeySetting* settingsP,
                          tUChar DefaultHighlight)
/***********************************************************************
 *
 *  InitKeysPanel
 *
 ***********************************************************************/
{
UInt8 loop;

  for (loop = 0; loop < nbKeys; loop++)
  {
  	UInt8 Keycode;

    keyP[loop].settingP = &settingsP[loop];
    keyP[loop].KeyStatus = KeyReleased;
    keyP[loop].CPCKey = settingsP[loop].CPCKey;
    keyP[loop].OnPushedP = settingsP[loop].OnPushedP;
    keyP[loop].OnReleasedP = settingsP[loop].OnReleasedP;
    keyP[loop].KeyHighlight = DefaultHighlight;

    // CPC Keycode
    Keycode = (tUChar)cpc_kbd[keyP[loop].CPCKey];
    keyP[loop].CPCKeycode = ((Keycode >> 4) << 3) | (Keycode & 0xf);
  }
}
/*----------------------------------------------------------------------------*/

Err StartMiniKeyboard(void)
/***********************************************************************
 *
 *  StartMiniKeyboard
 *
 ***********************************************************************/
{
Err Result;
  DBG_PRINT("Starting Minikeyboard\n");
  u32DebounceInTicks = (SysTicksPerSecond() * DEBOUNCETIME_MS / 1000) + 1; // +1 = upper round
  MiniKeyboardP = (tKeyboard*)MemPtrNew(sizeof(tKeyboard));

  if (MiniKeyboardP == NULL)
  {
#ifndef __RELEASE__
    DBG_PRINT("MiniKeyboardP == NULL\n");
#endif /*__RELEASE__*/

    return memErrNotEnoughSpace;
  }

  ActiveKeyboardP = MiniKeyboardP;
  Result = StartKeyboard(MiniKeyboardP);
  DBG_PRINT("Init Minikeyboard Done\n");
  // Start the background buffering of the keyboard
  if (Result==errNone)
    Result = PrepareKeyboard(MiniKeyboardP);

  DBG_PRINT("Display Minikeyboard Done\n");
  return Result;
}
/*----------------------------------------------------------------------------*/


static Err StartKeyboard(tKeyboard* keyboardP)
/***********************************************************************
 *
 *  StartMiniKeyboard
 *
 ***********************************************************************/
{
UInt8 loop;

  keyboardP->sectionsP = (tKeySection*)((tUChar*)&cpc_ks[0]);
  keyboardP->keysP = (tKey*)MemPtrNew(CPC_KB_KEYS * sizeof(tKey));
  if (keyboardP->keysP == NULL)
  {
#ifndef __RELEASE__
    DBG_PRINT("keyboardP->keysP == NULL");
#endif /*__RELEASE__*/

    return memErrNotEnoughSpace;
  }

  InitKeysPanel(keyboardP->keysP,
                CPC_KB_KEYS,
                (tKeySetting*)((tUChar*)&cpc_kb[0]),
                1);


  // Special keys settings
  for (loop = 0; loop < CPC_KB_KEYS; loop++)
  {
    switch (keyboardP->keysP[loop].settingP->CPCKey)
    {
      case CPC_KEY_RSHIFT:
      {
        keyboardP->keyRShiftP = &keyboardP->keysP[loop];
        keyboardP->keyRShiftP->CPCKey = CPC_KEY_NONE;
        keyboardP->keyRShiftP->OnPushedP = keyboardP->keyRShiftP->OnReleasedP = SHIFTKeyPressed;
        keyboardP->keyRShiftP->KeyHighlight = 0;
      }
      break;

      case CPC_KEY_LSHIFT:
      {
        keyboardP->keyLShiftP = &keyboardP->keysP[loop];
        keyboardP->keyLShiftP->CPCKey = CPC_KEY_NONE;
        keyboardP->keyLShiftP->OnPushedP = keyboardP->keyLShiftP->OnReleasedP = SHIFTKeyPressed;
        keyboardP->keyLShiftP->KeyHighlight = 0;
      }
      break;

      case CPC_KEY_CONTROL:
      {
        keyboardP->keyCTRLP = &keyboardP->keysP[loop];
        keyboardP->keyCTRLP->CPCKey = CPC_KEY_NONE;
        keyboardP->keyCTRLP->OnPushedP = keyboardP->keyCTRLP->OnReleasedP = CTRLKeyPressed;
        keyboardP->keyCTRLP->KeyHighlight = 0;
      }
      break;
    }
  }

  keyboardP->keyCurrentP = &keyboardP->keysP[39];
  return errNone;
}
/*----------------------------------------------------------------------------*/


void StopKeyboard(tKeyboard** keyboardPP)
/***********************************************************************
 *
 *  StopKeyboard
 *
 ***********************************************************************/
{
  if (*keyboardPP != NULL)
  {

    if ((*keyboardPP)->keysP != NULL)
    {
      MemPtrFree((*keyboardPP)->keysP);
    }

    MemPtrFree(*keyboardPP);
    *keyboardPP = NULL;
  }
}
/*----------------------------------------------------------------------------*/

static tResult PrepareOffscreenKey(tKey* keyP, tKeyStatus KeyStatus)
/***********************************************************************
 *
 *  PrepareOffscreenKey
 *
 ***********************************************************************/
{

if (KeyBoardOffScreenBuffer  == NULL)
{
  DBG_PRINT("PrepareOffscreenKey() == NULL");
  return memErrNotEnoughSpace;
}

uint8_t  *inp = NULL;
uint8_t  *outp = NULL;

// draw pressed keyboard status
for (int k=0;k<keyP->settingP->Height; k++)
{
  //inp = (uint8_t*)&CPC_KEYBOARD_IMG_DATA[CPC_KEYBOARD_IMG_WIDTH*(keyP->settingP->Top+k)+keyP->settingP->Left];
  outp = (uint8_t*)(KeyBoardOffScreenBuffer->data[0]) + CPC_KEYBOARD_IMG_WIDTH*(keyP->settingP->Top+k) + keyP->settingP->Left;
  inp = outp;
  for (int l=0;l<keyP->settingP->Width; l++)
    (*outp++) = (KeyStatus == KeyReleased) ? (*inp++) & 0x0F : (*inp++) | 0x10;
}

  return ResultSucceed;
}
/*----------------------------------------------------------------------------*/

static void PrepareChars(const tUChar* String, tUShort Length, tUShort Left, tUShort Top)
/***********************************************************************
 *
 *  PrepareChars
 *
 ***********************************************************************/
{
  uint8_t* sc_buffer = (uint8_t*)KeyBoardOffScreenBuffer->data[0];
  uint8_t c_string = 0;
  for(int j=0; j<Length; j++) {
    c_string = (uint8_t)String[j];
    if (c_string<CPC_KEYBOARD_FONT_FIRST_CHAR)
      return;

    for(int i=0;i<CPC_KEYBOARD_FONT_WIDTH; i++) {
      char charColumn = CPC_KEYBOARD_FONT[c_string-CPC_KEYBOARD_FONT_FIRST_CHAR][i];
      for (int pos=0;pos<8;pos++) {
        if ((1<<pos) & charColumn)
          *(sc_buffer + (pos+Top)*CPC_KEYBOARD_IMG_WIDTH + i + j*CPC_KEYBOARD_FONT_WIDTH + Left) = CPC_KEYBOARD_TXT_COL_INDEX;
        else
          *(sc_buffer + (pos+Top)*CPC_KEYBOARD_IMG_WIDTH + i + j*CPC_KEYBOARD_FONT_WIDTH + Left) = CPC_KEYBOARD_BCK_COL_INDEX;
      }
    }
  }
}



tResult PrepareKeyboard(tKeyboard* keyboardP)
/***********************************************************************
 *
 *  PrepareKeyboard
 *
 ***********************************************************************/
{

UInt8 Loop;
tUChar String[3];
Boolean ShiftPressed;

  // Offscreen window to prepare bitmap of panel
  // do it only once
  if (KeyBoardOffScreenBuffer  == NULL)
  {
    KeyBoardOffScreenBuffer = WinCreateOffscreenWindow(CPC_KEYBOARD_IMG_WIDTH,CPC_KEYBOARD_IMG_HEIGHT,nativeFormat, 1);
    if (KeyBoardOffScreenBuffer  == NULL)
      return memErrNotEnoughSpace;

    //
    // Draw keyboard background to offscreen buffer
    //
    MemMove((tUChar*)KeyBoardOffScreenBuffer->data[0], (tUChar*)&CPC_KEYBOARD_IMG_DATA[0], CPC_KEYBOARD_IMG_WIDTH*CPC_KEYBOARD_IMG_HEIGHT);
    //
    // Prepare the CPC_KEYBOARD_COLORMAP
    //
    MiniKeyboardColorMapP = (RGBColorType*)MemPtrNew(sizeof(RGBColorType) * CPC_KEYBOARD_NB_COLORS);
    if (MiniKeyboardColorMapP == NULL)
    {
#ifndef __RELEASE__
      DBG_PRINT("MiniKeyboardColorMapP == NULL");
#endif /*__RELEASE__*/

      return memErrNotEnoughSpace;
    }
    MemMove((tUChar*)MiniKeyboardColorMapP,(tUChar*)&CPC_KEYBOARD_COLORMAP[0],sizeof(RGBColorType) * CPC_KEYBOARD_NB_COLORS);
    //for (int i=0;i<CPC_KEYBOARD_NB_COLORS;i++)
    //  printf("i:%d, r:%d, g:%d, b:%d\n",i,MiniKeyboardColorMapP[i].r,MiniKeyboardColorMapP[i].g,MiniKeyboardColorMapP[i].b);

    // Prepare the 565 color codes
    MiniKeyboardColorMap565P = (UInt16*)MemPtrNew(sizeof(UInt16) * CPC_KEYBOARD_NB_COLORS);
    if (MiniKeyboardColorMap565P == NULL)
    {
#ifndef __RELEASE__
      DBG_PRINT("MiniKeyboardColorMap565P == NULL");
#endif /*__RELEASE__*/

      return memErrNotEnoughSpace;
    }
    for (int i=0;i<CPC_KEYBOARD_NB_COLORS;i++)
    {
        MiniKeyboardColorMap565P[i]=ESWAP(RGB565(MiniKeyboardColorMapP[i].r,MiniKeyboardColorMapP[i].g,MiniKeyboardColorMapP[i].b));
    }
  }

  //
  // Draw pushed keys bitmap
  //
#ifndef __RELEASE__
  if (keyboardP->keyRShiftP == NULL)
    {DBG_PRINT("keyboardP->keyRShiftP == NULL\n");}
#endif /* __RELEASE__ */
  PrepareOffscreenKey(keyboardP->keyRShiftP,keyboardP->keyRShiftP->KeyStatus);
#ifndef __RELEASE__
  if (keyboardP->keyLShiftP == NULL)
    {DBG_PRINT("keyboardP->keyLShiftP == NULL\n");}
#endif /* __RELEASE__ */
  PrepareOffscreenKey(keyboardP->keyLShiftP,keyboardP->keyRShiftP->KeyStatus);
#ifndef __RELEASE__
  if (keyboardP->keyCTRLP == NULL)
    {DBG_PRINT("keyboardP->keyCTRLP == NULL\n");}
#endif /* __RELEASE__ */
  PrepareOffscreenKey(keyboardP->keyCTRLP,keyboardP->keyRShiftP->KeyStatus);


  //
  // Normal keyboard display
  //
  ShiftPressed = IsShiftPressed() == cFalse ? false : true;
  if (1)
  {
    String[1] = 0;
    for (Loop = 0; Loop < CPC_KB_KEYS; Loop++)
    {
      const tKeySetting* settingP = keyboardP->keysP[Loop].settingP;

      String[0] = (ShiftPressed == false) ? settingP->charCode_Normal : settingP->charCode_Shift;
      if (String[0])
      {
        // Display key char
        PrepareChars(String,
                      1,
                      settingP->Left + ((settingP->Width - CPC_KEYBOARD_FONT_WIDTH) / 2),
                      settingP->Top + (settingP->Height - CPC_KEYBOARD_FONT_HEIGHT) / 2);
      }
    }
  }
  else
  //
  // CPC Keycodes display
  //
  {

    String[2] = 0;
    for (Loop = 0; Loop < CPC_KB_KEYS; Loop++)
    {
      const tKeySetting* settingP = keyboardP->keysP[Loop].settingP;

      String[0] = '0' + keyboardP->keysP[Loop].CPCKeycode / 10;
      String[1] = '0' + keyboardP->keysP[Loop].CPCKeycode % 10;

      if (String[0] == '0')
      {
        // Display key char
        PrepareChars(&String[1],
                      1,
                      settingP->Left + ((settingP->Width - CPC_KEYBOARD_FONT_WIDTH) / 2),
                      settingP->Top + (settingP->Height - CPC_KEYBOARD_FONT_HEIGHT) / 2);
      }
      else
      {
        // Display key char
        PrepareChars(String,
                      2,
                      settingP->Left + ((settingP->Width - (CPC_KEYBOARD_FONT_WIDTH * 2)) / 2),
                      settingP->Top + (settingP->Height - CPC_KEYBOARD_FONT_HEIGHT) / 2);
      }
    }
  }

  PrepareOffscreenKey(keyboardP->keyCurrentP, KeyPressed);
  return ResultSucceed;
}
/*----------------------------------------------------------------------------*/


static void UpdatePanelPosition(UInt16 y_base,
                                UInt8 nbKeys,
                                tKey* keyP)
/***********************************************************************
 *
 *  UpdateKeysPosition
 *
 ***********************************************************************/
{
UInt8 loop;

#ifndef __RELEASE__
  if (keyP == NULL)
    DBG_PRINT("keyP == NULL");
#endif /* NOT __RELEASE__ */

  for (loop=nbKeys; loop; loop--, keyP++)
  {
    keyP->Left = keyP->settingP->Left;
    keyP->Top = keyP->settingP->Top + y_base;
    keyP->Right = keyP->Left + keyP->settingP->Width;
    keyP->Bottom = keyP->Top + keyP->settingP->Height;
  }
}

void ToggleAutoToggle(void)
/***********************************************************************
 *
 *  ToggleAutoToggle
 *
 ***********************************************************************/
{
  u32AutoToggleKeyState = (u32AutoToggleKeyState & keyBitRockerLeft) ? keyBitRockerRight : keyBitRockerLeft;
}
/*----------------------------------------------------------------------------*/

tBool IsShiftPressed(void)
/***********************************************************************
 *
 *  IsShiftPressed
 *
 ***********************************************************************/
{
  //if (CPCColouredKeys[KEYINDEX_SHIFT].KeyStatus == KeyPressed)
  //  return cTrue;
  if (ActiveKeyboardP->keyRShiftP->KeyStatus == KeyPressed)
    return cTrue;
  if (ActiveKeyboardP->keyLShiftP->KeyStatus == KeyPressed)
    return cTrue;
  return cFalse;
}
/*----------------------------------------------------------------------------*/


#if defined(_DEBUG) || defined(_SHOW_KEYS)

tUShort GetCurrentPressedKeys(void)
/***********************************************************************
 *
 *  GetCurrentPressedKeys
 *
 ***********************************************************************/
{
	return usCurrentPressedKey;
}

#endif /* _DEBUG || _SHOW_KEYS */


void KeyboardSetAsciiKeyDown(char key)
/***********************************************************************
 *
 *  KeyboardSetAsciiKeyDown
 *
 ***********************************************************************/
{
tUChar cpc_index;
tUShort cpc_key = KEY_INVALID;

  cpc_index = palm_kbd[(tUChar)key];
  if (cpc_index != KEY_INVALID)
  {
    cpc_key = cpc_kbd[cpc_index];
  }

  if (cpc_key != KEY_INVALID)
  {
    PRESS_KEY(cpc_key);

    // Restore SHIFT and CTRL state
    SetKeySHIFTandCTRLState(cpc_key);
  }
}
/*----------------------------------------------------------------------------*/


void KeyboardSetAsciiKeyUp(char key)
/***********************************************************************
 *
 *  KeyboardSetAsciiKeyUp
 *
 ***********************************************************************/
{
tUChar cpc_index;
tUShort cpc_key = KEY_INVALID;

  cpc_index = palm_kbd[(tUChar)key];
  if (cpc_index != KEY_INVALID)
  {
    cpc_key = cpc_kbd[cpc_index];
  }

  if (cpc_key != KEY_INVALID)
  {
    RELEASE_KEY(cpc_key);

    if (cpc_key & MOD_CPC_SHIFT) // CPC SHIFT key required?
    {
      RELEASE_KEY(CPC_KBD_SHIFT); // make sure key is unSHIFTed
    }

    if (cpc_key & MOD_CPC_CTRL) // CPC CONTROL key required?
    {
      RELEASE_KEY(CPC_KBD_CTRL); // make sure CONTROL key is released
    }
  }
}
/*----------------------------------------------------------------------------*/

static tVoid MemMove(tUChar* destP,
                     tUChar* sourceP,
                     tULong numBytes)
/***********************************************************************
 *
 *  MemMove
 *
 ***********************************************************************/
{
  while (numBytes--)
  {
    *(destP++) = *(sourceP++);
  }
}

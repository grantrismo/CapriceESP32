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

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"
#include "sections.h"
#include "vkb_layout.h"
#include "gbuf.h"
#include "event.h"
#include "keypad.h"


//
// Drive panel
//
#define KEYINDEX_SWAPDISK           0
#define KEYINDEX_EJECTDISK          1
#define KEYINDEX_DRIVE_A            2
#define KEYINDEX_DRIVE_B            3
#define NB_DRIVE_KEYS               1/* First index */ + KEYINDEX_DRIVE_B


//
// Emulator panel
//
enum
{
  KEYINDEX_RESET = 0,
  KEYINDEX_PAUSE,
  KEYINDEX_TRUESPEED,
  KEYINDEX_SOUND,
  KEYINDEX_SOUND_VOLUME,
  KEYINDEX_JOYSTICK,
  KEYINDEX_AUTOTOGGLE,
  KEYINDEX_EMU_BG,
  KEYINDEX_SUBPANEL,
  // do not change
  NB_EMULATORS_KEYS
};

enum
{
  KEYINDEX_ESC = 0,
  KEYINDEX_COPY,
  KEYINDEX_SHIFT,
  KEYINDEX_CTRL,
  KEYINDEX_CAPSLOCK,
  KEYINDEX_TAB,
  KEYINDEX_DEL,
  KEYINDEX_ENTER,
  //
  CPC_COLOURED_ARROW,
  NB_CPC_COLOURED_KEYS
};

enum
{
  KEYINDEX_FX = 0,
  KEYINDEX_CLR,
  KEYINDEX_CPC_BG_1,
  KEYINDEX_SHOWSPEED,
  KEYINDEX_CPC_BG_2,
  //
  CPC_SPECIAL_ARROW,
  NB_CPC_SPECIAL_KEYS
};

//
#define keyBitHardKey1              (0x00000008)
#define keyBitHardKey2              (0x00000010)
#define keyBitHardKey3              (0x00000020)
#define keyBitHardKey4              (0x00000040)


typedef enum
{
   CPC_KEY_0,
   CPC_KEY_1,
   CPC_KEY_2,
   CPC_KEY_3,
   CPC_KEY_4,
   CPC_KEY_5,
   CPC_KEY_6,
   CPC_KEY_7,
   CPC_KEY_8,
   CPC_KEY_9,
   CPC_KEY_A,
   CPC_KEY_B,
   CPC_KEY_C,
   CPC_KEY_D,
   CPC_KEY_E,
   CPC_KEY_F,
   CPC_KEY_G,
   CPC_KEY_H,
   CPC_KEY_I,
   CPC_KEY_J,
   CPC_KEY_K,
   CPC_KEY_L,
   CPC_KEY_M,
   CPC_KEY_N,
   CPC_KEY_O,
   CPC_KEY_P,
   CPC_KEY_Q,
   CPC_KEY_R,
   CPC_KEY_S,
   CPC_KEY_T,
   CPC_KEY_U,
   CPC_KEY_V,
   CPC_KEY_W,
   CPC_KEY_X,
   CPC_KEY_Y,
   CPC_KEY_Z,
   CPC_KEY_a,
   CPC_KEY_b,
   CPC_KEY_c,
   CPC_KEY_d,
   CPC_KEY_e,
   CPC_KEY_f,
   CPC_KEY_g,
   CPC_KEY_h,
   CPC_KEY_i,
   CPC_KEY_j,
   CPC_KEY_k,
   CPC_KEY_l,
   CPC_KEY_m,
   CPC_KEY_n,
   CPC_KEY_o,
   CPC_KEY_p,
   CPC_KEY_q,
   CPC_KEY_r,
   CPC_KEY_s,
   CPC_KEY_t,
   CPC_KEY_u,
   CPC_KEY_v,
   CPC_KEY_w,
   CPC_KEY_x,
   CPC_KEY_y,
   CPC_KEY_z,
   CPC_KEY_AMPERSAND,
   CPC_KEY_ASTERISK,
   CPC_KEY_AT,
   CPC_KEY_BACKQUOTE,
   CPC_KEY_BACKSLASH,
   CPC_KEY_CAPSLOCK,
   CPC_KEY_CLR,
   CPC_KEY_COLON,
   CPC_KEY_COMMA,
   CPC_KEY_CONTROL,
   CPC_KEY_COPY,
   CPC_KEY_CPY_DOWN,
   CPC_KEY_CPY_LEFT,
   CPC_KEY_CPY_RIGHT,
   CPC_KEY_CPY_UP,
   CPC_KEY_CUR_DOWN,
   CPC_KEY_CUR_LEFT,
   CPC_KEY_CUR_RIGHT,
   CPC_KEY_CUR_UP,
   CPC_KEY_CUR_ENDBL,
   CPC_KEY_CUR_HOMELN,
   CPC_KEY_CUR_ENDLN,
   CPC_KEY_CUR_HOMEBL,
   CPC_KEY_DBLQUOTE,
   CPC_KEY_DEL,
   CPC_KEY_DOLLAR,
   CPC_KEY_ENTER,
   CPC_KEY_EQUAL,
   CPC_KEY_ESC,
   CPC_KEY_EXCLAMATN,
   CPC_KEY_F0,
   CPC_KEY_F1,
   CPC_KEY_F2,
   CPC_KEY_F3,
   CPC_KEY_F4,
   CPC_KEY_F5,
   CPC_KEY_F6,
   CPC_KEY_F7,
   CPC_KEY_F8,
   CPC_KEY_F9,
   CPC_KEY_FPERIOD,
   CPC_KEY_GREATER,
   CPC_KEY_HASH,
   CPC_KEY_LBRACKET,
   CPC_KEY_LCBRACE,
   CPC_KEY_LEFTPAREN,
   CPC_KEY_LESS,
   CPC_KEY_LSHIFT,
   CPC_KEY_MINUS,
   CPC_KEY_PERCENT,
   CPC_KEY_PERIOD,
   CPC_KEY_PIPE,
   CPC_KEY_PLUS,
   CPC_KEY_POUND,
   CPC_KEY_POWER,
   CPC_KEY_QUESTION,
   CPC_KEY_QUOTE,
   CPC_KEY_RBRACKET,
   CPC_KEY_RCBRACE,
   CPC_KEY_RETURN,
   CPC_KEY_RIGHTPAREN,
   CPC_KEY_RSHIFT,
   CPC_KEY_SEMICOLON,
   CPC_KEY_SLASH,
   CPC_KEY_SPACE,
   CPC_KEY_TAB,
   CPC_KEY_UNDERSCORE,
   CPC_KEY_J0_UP,
   CPC_KEY_J0_DOWN,
   CPC_KEY_J0_LEFT,
   CPC_KEY_J0_RIGHT,
   CPC_KEY_J0_FIRE1,
   CPC_KEY_J0_FIRE2,
   CPC_KEY_J1_UP,
   CPC_KEY_J1_DOWN,
   CPC_KEY_J1_LEFT,
   CPC_KEY_J1_RIGHT,
   CPC_KEY_J1_FIRE1,
   CPC_KEY_J1_FIRE2,
   CPC_KEY_ES_NTILDE,
   CPC_KEY_ES_nTILDE,
   CPC_KEY_ES_PESETA,
   CPC_KEY_FR_eACUTE,
   CPC_KEY_FR_eGRAVE,
   CPC_KEY_FR_cCEDIL,
   CPC_KEY_FR_aGRAVE,
   CPC_KEY_FR_uGRAVE,
   CPC_KEY_NONE,
} CPC_KEYS;


typedef enum
{
  KeyReleased,
  KeyPressed,
} tKeyStatus;

typedef enum
{
  RockerAsJoyOrCursors,
  RockerAsVirtualKeyboard,
  RockerAsMenuControl
} tRockerAttach;

// typedef enum
// {
//   KeyStatic,
//   KeyImpulse,
//   KeyToggle,
// } tKeyMode;

// typedef struct
// {
//   tUShort Left;
//   tUShort Top;
//   tUShort Width;
//   tUShort Height;
//   DmResID resKeyUp;
//   DmResID resKeyDown;
//   tKeyMode KeyMode;
//   tUChar CPCKey;
//   tUChar charCode_Normal;
//   tUChar charCode_Shift;
//   void (*OnPushedP)(void);
//   void (*OnReleasedP)(void);
// } tKeySetting;

typedef struct
{
  const tKeySetting* settingP;
  void (*OnPushedP)(void);
  void (*OnReleasedP)(void);
  tUShort Left;
  tUShort Top;
  tUShort Right;
  tUShort Bottom;
  tKeyStatus KeyStatus;
  tUChar CPCKey;
  tUChar CPCKeycode;
  tUChar KeyHighlight;
} tKey;

// typedef struct
// {
//   tUShort Left;
//   tUShort Top;
//   tUShort Right;
//   tUShort Bottom;
//   tUChar nb_keys;
// } tKeySection;

typedef struct
{
  tKeyStatus SoundKeyStatus;
  tKeyStatus FxKeyStatus;
  tKeyStatus JoystickKeyStatus;
  tKeyStatus AutoToggleKeyStatus;
} tEmulatorKeysStatus;

typedef struct
{
  tUShort HeaderSize;
  tUShort SectionsSize;
  tUShort SettingsSize;
  tUShort NbTotalSections;
  tUShort NbTotalKeys;
} tKeyboardHeader;

typedef struct
{
  MemHandle headerH;
  tKeyboardHeader* headerP;
  const tKeySection* sectionsP;
  tKey* keysP;
  tKey* keyRShiftP;
  tKey* keyLShiftP;
  tKey* keyCTRLP;
  tKey* keyCurrentP;
} tKeyboard;

typedef struct
{
  uint8_t           index;       // index of color or best match to cur CLUT or unused.
  uint8_t           r;           // amount of red, 0->255
  uint8_t           g;           // amount of green, 0->255
  uint8_t           b;           // amount of blue, 0->255
} RGBColorType;


#define IS_KEY_PRESSED(key) ((NativeCPC->keyboard_matrix[((tUChar)key) >> 4] & (1 << (((tUChar)key) & 7))) == 0)
#define PRESS_KEY(key)      (NativeCPC->keyboard_matrix[((tUChar)key) >> 4] &= ~(1 << (((tUChar)key) & 7)))
#define RELEASE_KEY(key)    (NativeCPC->keyboard_matrix[((tUChar)key) >> 4] |= 1 << (((tUChar)key) & 7))

#define LINE_FROM_KEYCODE(Key) ((tUChar)(Key >> 4))
#define ROW_FROM_KEYCODE(Key)  ((tUChar)(Key & 7))

#define CPC_KBD_SHIFT             0x25
#define CPC_KBD_CTRL              0x27
#define CPC_KBD_J0_UP             0x90
#define CPC_KBD_J0_DOWN           0x91
#define CPC_KBD_J0_FIRE1          0x94
#define CPC_KBD_J0_FIRE2          0x95

#define WEST_PHASER_TRIG_KEY      CPC_KBD_J0_FIRE2
#define WEST_PHASER_TRIG_TICKS    10 // Best found = 10
#define WEST_PHASER_BEAM_KEY      CPC_KBD_J0_UP
#define WEST_PHASER_SENSITIVITY   255 // Best found = 64

#define GUNSTICK_TRIG_KEY         CPC_KBD_J0_FIRE1
#define GUNSTICK_BEAM_KEY         CPC_KBD_J0_DOWN
#define GUNSTICK_SENSITIVITY      32 // Best found = 32
#define DEBOUNCETIME_MS           50     // Key down duration following to key down event (ms)


#ifndef __WIN32__

extern UInt8 AutoToggleActive;
extern UInt8 AutoToggleDurationInTicks;
extern UInt8 AutoToggleDurationIndex;

extern UInt8 AdjustSoundVolumeActive;

extern tKey* EmulatorKeysPanelP;
extern tULong NbEmulatorKeys;
extern tKey DriveKeys[];
extern tKey EmulatorKeys[];
extern tKey CPCColouredKeys[];
extern tKey CPCSpecialKeys[];

extern tKeyboard* ActiveKeyboardP;
extern tKeyboard* DIAKeyboardP;
extern tKeyboard* MiniKeyboardP;
extern gbuf_t*    KeyBoardOffScreenBuffer;
extern RGBColorType* MiniKeyboardColorMapP;
extern UInt16* MiniKeyboardColorMap565P;
extern tRockerAttach NewRockerAttach;
extern tRockerAttach OldRockerAttach;

extern tEmulatorKeysStatus EmulatorKeysStatus;

extern void logkeys(void);

extern Boolean KeyDownHandleEvent(event_t* Event) SECTION_KEYBOARD;
extern Boolean KeyUpHandleEvent(event_t* Event) SECTION_KEYBOARD;
extern tResult PrepareKeyboard(tKeyboard* keyboardP) SECTION_KEYBOARD;
extern UInt8 GetCurrentKeyBaseCoordY() SECTION_KEYBOARD;

extern void SetCursorAndJoystickState(UInt32 keyState,
                                      UInt32* oldkeystateP) SECTION_KEYBOARD;

extern void RockerAttachManager(UInt32 keyState,
                         UInt32* oldkeystateP) SECTION_KEYBOARD;

extern void ReleasePressedKey(void) SECTION_KEYBOARD;

extern void SelectCPCColouredKeyboard(void) SECTION_KEYBOARD;
extern void SelectCPCSpecialKeyboard(void) SECTION_KEYBOARD;

extern void EnableJoystick(void) SECTION_KEYBOARD;
extern void DisableJoystick(void) SECTION_KEYBOARD;
extern void EnableSound(void) SECTION_KEYBOARD;
extern void DisableSound(void) SECTION_KEYBOARD;
extern void AutoToggleKeyPressed(void) SECTION_KEYBOARD;
extern void AutoToggleKeyReleased(void) SECTION_KEYBOARD;
extern void AdjustSoundVolumeKeyPressed(void) SECTION_KEYBOARD;
extern void AdjustSoundVolumeKeyReleased(void) SECTION_KEYBOARD;

extern void InitKeys(void) SECTION_KEYBOARD;
extern Err StartDIAKeyboard(void) SECTION_KEYBOARD;
extern Err StartMiniKeyboard(void) SECTION_KEYBOARD;
extern void StopKeyboard(tKeyboard** keyboardPP) SECTION_KEYBOARD;
extern void UpdateKeysPosition(void) SECTION_KEYBOARD;
extern void ToggleAutoToggle(void) SECTION_KEYBOARD;

extern tBool IsShiftPressed(void) SECTION_KEYBOARD;

extern void RestoreEmulatorKeysStatus(tEmulatorKeysStatus* keyStatusP) SECTION_KEYBOARD;

extern void SwapKeyboardState(tUChar Display320x480) SECTION_KEYBOARD;

extern void UpdateRockerCenterKeyMask(void) SECTION_KEYBOARD;
extern void UpdateHardCPCKeyCodeMask(void) SECTION_KEYBOARD;

extern tUShort GetKeySHIFTandCTRLState(tVoid);
extern void SetKeySHIFTandCTRLState(tUShort usKeyState);

extern void KeyboardSetAsciiKeyDown(char key);
extern void KeyboardSetAsciiKeyUp(char key);

#if defined(_DEBUG) || defined(_SHOW_KEYS)
extern tUShort GetCurrentPressedKeys(void);
#endif /* _DEBUG || _SHOW_KEYS */

#endif /* __WIN32__ */

#endif /* ! KEYBOARD_H */

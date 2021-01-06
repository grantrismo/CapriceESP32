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
#include "Prefs.h"
//#include "Keyboard.h"
//#include "Display.h"

// Default values

#define DEFAULT_PREF_CPCMODEL             CPC_MODEL_6128
#define DEFAULT_PREF_CPCJUMPERS           CPC_DEFAULT_JUMPERS
#define DEFAULT_PREF_SCREENTYPE           0 // Color
#define DEFAULT_PREF_CPCTRUESPEED         1 // Full speed
#define DEFAULT_PREF_SCREENINTENSITY      10
#define DEFAULT_PREF_SOUNDENABLED         1
#define DEFAULT_PREF_SOUNDRENDERER        0   // 0 = internal Speaker, 1 = Bluetooth A2DP
#define DEFAULT_PREF_MEDIASTATES          0   // Holds the actual A2DP Meadia States
#define DEFAULT_PREF_SOUNDVOLUME          20  // in percetage of full scale
#define DEFAULT_PREF_SOUNDSYSTEMVOLUME    1   // 0 = Own volume, 1 = System games volume (default)
#define DEFAULT_PREF_DISPLAY320X480       0   // 0 = 320x320 (default), 1 = 320x480
#define DEFAULT_PREF_AUTOCONTEXTSAVE      0   // 0 = Manual save (default), 1 = Automatic save
#define DEFAULT_PREF_CUSTOMCOLORUSE       0   // 0 = Do not use Custom colors (default), 1 = Use custom colors
#define DEFAULT_PREF_CUSTOMCOLORINDEX     0
#define DEFAULT_PREF_MODE2ANTIALIAS       0   // 0 = Mode 2 Aliased (default), 1 = Mode 2 Anti-aliased (not implemented)
#define DEFAULT_PREF_OS_DISPLAY           1   // 0 = OSD Inactive, 1 = OSD Ative (default)
#define DEFAULT_PREF_OS_ROCKER            0   // 0 = OSR Inactive (default), 1 = OSR Active
#define DEFAULT_PREF_CENTERKEY_INDEX      0
#define DEFAULT_PREF_HARDKEY_FIRE         0
#define DEFAULT_PREF_CHANGED              1   // Preferences is changed by default
#define DEFAULT_PREF_AUTOOFF_DISABLE      1   // 0 = AutoOff Enable, 1 = AutoOff Disable (default)
#define DEFAULT_PREF_DISPLAY_OFFSET_X     OFFSCREEN_OFFSET_X
#define DEFAULT_PREF_DISPLAY_OFFSET_Y     OFFSCREEN_OFFSET_Y
#define DEFAULT_PREF_HARDKEY_1_INDEX      HARDKEY_NOT_USED_INDEX
#define DEFAULT_PREF_HARDKEY_2_INDEX      HARDKEY_NOT_USED_INDEX
#define DEFAULT_PREF_HARDKEY_3_INDEX      HARDKEY_NOT_USED_INDEX
#define DEFAULT_PREF_HARDKEY_4_INDEX      HARDKEY_NOT_USED_INDEX
#define DEFAULT_PREF_USE_PARADOS          0
#define DEFAULT_PREF_USE_64K_MEM_EXT      0
#define DEFAULT_PREF_NIGHTMODE_ACTIVE     0
#define DEFAULT_PREF_CPC_KEYCODE_A        76  // Key code for Joystick 0 Fire 2
#define DEFAULT_PREF_RENDERING            0   // Fastest
#define DEFAULT_PREF_CPC_KEYCODE_B        21  // Key code for Shift
#define DEFAULT_PREF_CPC_KEYCODE_C        23  // Key code for Control
#define DEFAULT_PREF_CPC_KEYCODE_D        47  // Key code for Space Bar
#define DEFAULT_AUTOSTART_ENABLE          1   // Disable
#define DEFAULT_PREF_OS_LIGHTPEN          0
#define DEFAULT_PREF_OS_MAGNUMGUN         0
#define DEFAULT_PREF_OS_GUNSTICK          0
#define DEFAULT_PREF_OS_WESTPHASER        0

#define MemPtrNew(p) malloc(p)
#define errNone                       0x0000  // No error

static tVoid StrCopy(tChar* destP,
                     tChar* srcP);

static tVoid MemSet(tUChar* destP,
                   tULong numBytes,
                   tUChar value);


Err LirePreferences(tPreferences** prefPP)
/***********************************************************************
 *
 *  LirePreferences
 *
 ***********************************************************************/
{
tPreferences* prefP;

prefP = (tPreferences*)MemPtrNew(sizeof(tPreferences));

if (prefP == NULL)
  return memErrNotEnoughSpace;

*prefPP = prefP;

// defaults
prefP->CPCModel = DEFAULT_PREF_CPCMODEL;
prefP->CPCJumpers = DEFAULT_PREF_CPCJUMPERS;
prefP->ScreenType = DEFAULT_PREF_SCREENTYPE;
prefP->CPCTrueSpeed = DEFAULT_PREF_CPCTRUESPEED;
StrCopy(prefP->FilesPathname, (tChar*)DEFAULT_DISK_PATH);
prefP->ScreenIntensity = DEFAULT_PREF_SCREENINTENSITY;
prefP->SoundEnabled = DEFAULT_PREF_SOUNDENABLED;
prefP->SoundVolume = DEFAULT_PREF_SOUNDVOLUME;
prefP->SoundSystemVolume = DEFAULT_PREF_SOUNDSYSTEMVOLUME;
prefP->SoundRenderer = DEFAULT_PREF_SOUNDRENDERER;
prefP->A2dpMediaStates = DEFAULT_PREF_MEDIASTATES; 
prefP->Display320x480 = DEFAULT_PREF_DISPLAY320X480;
prefP->AutomaticContextSave = DEFAULT_PREF_AUTOCONTEXTSAVE;
prefP->CustomColorUse = DEFAULT_PREF_CUSTOMCOLORUSE;
MemSet(prefP->CustomColorIndexes, 32, DEFAULT_PREF_CUSTOMCOLORINDEX);
prefP->Mode2AntiAliasing = DEFAULT_PREF_MODE2ANTIALIAS;
prefP->OnScreenDisplayActive = DEFAULT_PREF_OS_DISPLAY;
prefP->OnScreenRockerActive = DEFAULT_PREF_OS_ROCKER;
prefP->RockerCenterKeyIndex = DEFAULT_PREF_CENTERKEY_INDEX;
prefP->JoystickFireHardKey = DEFAULT_PREF_HARDKEY_FIRE;
prefP->PreferencesChanged = DEFAULT_PREF_CHANGED;
prefP->AutoOffDisable = DEFAULT_PREF_AUTOOFF_DISABLE;
prefP->ScreenDisplayOffsetX = 0;
prefP->ScreenDisplayOffsetY = 0;
prefP->HardKey1Index = DEFAULT_PREF_HARDKEY_1_INDEX;
prefP->HardKey2Index = DEFAULT_PREF_HARDKEY_2_INDEX;
prefP->HardKey3Index = DEFAULT_PREF_HARDKEY_3_INDEX;
prefP->HardKey4Index = DEFAULT_PREF_HARDKEY_4_INDEX;
prefP->UseParados = DEFAULT_PREF_USE_PARADOS;
prefP->Use64kMemoryExtension = DEFAULT_PREF_USE_64K_MEM_EXT;
prefP->NightModeActive = DEFAULT_PREF_NIGHTMODE_ACTIVE;
prefP->CPCKeycodeA = DEFAULT_PREF_CPC_KEYCODE_A;
prefP->Rendering = DEFAULT_PREF_RENDERING;
prefP->CPCKeycodeB = DEFAULT_PREF_CPC_KEYCODE_B;
prefP->CPCKeycodeC = DEFAULT_PREF_CPC_KEYCODE_C;
prefP->CPCKeycodeD = DEFAULT_PREF_CPC_KEYCODE_D;
prefP->AutoStartEnable = DEFAULT_AUTOSTART_ENABLE;
prefP->OnScreenLightPenActive = DEFAULT_PREF_OS_LIGHTPEN;
prefP->OnScreenMagnumGunActive = DEFAULT_PREF_OS_MAGNUMGUN;
prefP->OnScreenGunstickActive = DEFAULT_PREF_OS_GUNSTICK;
prefP->OnScreenWestPhaserActive = DEFAULT_PREF_OS_WESTPHASER;
prefP->NightModeActive = DEFAULT_PREF_NIGHTMODE_ACTIVE; // Night mode always inactive at start

#ifdef _ROCKER_ACTIVE
  prefP->OnScreenRockerActive = 1;
  prefP->OnScreenLightPenActive = 0;
  prefP->OnScreenMagnumGunActive = 0;
  prefP->OnScreenGunstickActive = 0;
  prefP->OnScreenWestPhaserActive = 0;
#endif /* _ROCKER_ACTIVE */
#ifdef _LIGHTPEN_ACTIVE
  prefP->OnScreenRockerActive = 0;
  prefP->OnScreenLightPenActive = 1;
  prefP->OnScreenMagnumGunActive = 0;
  prefP->OnScreenGunstickActive = 0;
  prefP->OnScreenWestPhaserActive = 0;
#endif /* _LIGHTPEN_ACTIVE */
#ifdef _MAGNUMGUN_ACTIVE
  prefP->OnScreenRockerActive = 0;
  prefP->OnScreenLightPenActive = 0;
  prefP->OnScreenMagnumGunActive = 1;
  prefP->OnScreenGunstickActive = 0;
  prefP->OnScreenWestPhaserActive = 0;
#endif /* _MAGNUMGUN_ACTIVE */
#ifdef _GUNSTICK_ACTIVE
  prefP->OnScreenRockerActive = 0;
  prefP->OnScreenLightPenActive = 0;
  prefP->OnScreenMagnumGunActive = 0;
  prefP->OnScreenGunstickActive = 1;
  prefP->OnScreenWestPhaserActive = 0;
#endif /* _GUNSTICK_ACTIVE */
#ifdef _WESTPHASER_ACTIVE
  prefP->OnScreenRockerActive = 0;
  prefP->OnScreenLightPenActive = 0;
  prefP->OnScreenMagnumGunActive = 0;
  prefP->OnScreenGunstickActive = 0;
  prefP->OnScreenWestPhaserActive = 1;
#endif /* _WESTPHASER_ACTIVE */

  //TODO load overrides from database

  return errNone;
}
/*----------------------------------------------------------------------------*/


void EcrirePreferences(tPreferences* prefP)
/***********************************************************************
 *
 *  EcrirePreferences
 *
 ***********************************************************************/
{
  printf("Save Prefs not implemented\n");
}
/*----------------------------------------------------------------------------*/


void SetHardKeysMask(const tPreferences* prefP,
                     UInt32 originKeyMask)
/***********************************************************************
 *
 *  SetHardKeysMask
 *
 ***********************************************************************/
{
  // Do not generate events for rocker
  /*
  emulatorHardKeyMask = originKeyMask & ~(keyBitRockerUp | keyBitRockerDown | keyBitRockerLeft | keyBitRockerRight | keyBitRockerCenter);

  // Do not generate Hard Key events for simulating Joystick Fire.
  if (prefP->HardKey1Index == HARDKEY_ROCKER_CENTER_INDEX)
  {
    emulatorHardKeyMask &= ~keyBitHardKey1;
  }
  if (prefP->HardKey2Index == HARDKEY_ROCKER_CENTER_INDEX)
  {
    emulatorHardKeyMask &= ~keyBitHardKey2;
  }
  if (prefP->HardKey3Index == HARDKEY_ROCKER_CENTER_INDEX)
  {
    emulatorHardKeyMask &= ~keyBitHardKey3;
  }
  if (prefP->HardKey4Index == HARDKEY_ROCKER_CENTER_INDEX)
  {
    emulatorHardKeyMask &= ~keyBitHardKey4;
  }
*/
	//UpdateRockerCenterKeyMask();
	//UpdateHardCPCKeyCodeMask();

  // Set new key mask.
  //KeySetMask(emulatorHardKeyMask);
}
/*----------------------------------------------------------------------------*/

static tVoid StrCopy(tChar* destP,
                     tChar* srcP)
/***********************************************************************
 *
 *  MemSet
 *
 ***********************************************************************/
{
  while (*srcP != 0)
  {
    *(destP++) = *(srcP++);
  }
}
/*----------------------------------------------------------------------------*/

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

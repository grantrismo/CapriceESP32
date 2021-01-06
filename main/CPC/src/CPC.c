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
#include <PalmOS.h>
#include <ByteOrderUtils.h>
#endif

#include <stdio.h>
#include <stdlib.h>

//#include "CaPriCe.h"
#include "Prefs.h"
#include "Routines.h"
#include "Resources.h"
#include "Keyboard.h"
#include "CPC.h"
#include "Sound.h"
#include "trace.h"
#include "Files.h"
//#include "Display.h"

#include "ttgui.h"
#include "gbuf.h"
#include "colours_rgb.h"
#include "colours_green.h"
#include "DAATable.h"

#ifdef SIM
#include "FDCCommandTableX64.h"
#else
#include "FDCCommandTable.h"
#endif

//===================
// PATCH begin
#ifdef _PATCH_ENABLE

#endif /* _PATCH_ENABLE */
// PATCH end
//===================

// Basic DEBUG SWITCH
#undef CPC_DEBUG_ENABLED
#define CPC_DEBUG_ENABLED

#ifdef CPC_DEBUG_ENABLED
#  define DBG_PRINT(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s():" fmt, __FILE__, __LINE__, __func__, ##args)
#else /* CPC_DEBUG_ENABLED */
#  define DBG_PRINT(fmt, args...)
#endif /* CPC_DEBUG_ENABLED */

// Large PNO Native_CPCExecute
#define PNO_CPCEXECUTE_SIZE           ((tULong)117076)
#define PNO_CPCEXECUTE_CRC32          ((tULong)0xc795025c)
#define PNO_CPCEXECUTE_FIRSTRESID     5000

#define errNone                       0x0000  // No error
#define memErrorClass                 0x0100  // Memory Manager
// copied from MemoryMgr.h
#define memErrNotEnoughSpace          (memErrorClass | 2)
#define nativeFormat                  1

#define	sysErrorClass			           	0x0500	// System Manager
#define	sysErrTimeout							    (sysErrorClass | 1)
#define	sysErrParamErr							  (sysErrorClass | 2)
#define	sysErrNoFreeResource					(sysErrorClass | 3)
#define	sysErrNoFreeRAM						    (sysErrorClass | 4)
#define	sysErrNotAllowed						  (sysErrorClass | 5)

#define MemPtrNew(p) malloc(p)
#define MemPtrFree(p) free(p)

gbuf_t* OffScreenBuffer = NULL;
tUChar* contextP = NULL;
tNativeCPC* NativeCPC = NULL;
colours_rgb_entry* WinPalettePtr = NULL;
UInt16 *RGB565PalettePtr = NULL;

// might Move
char* sessionFilenameP = NULL; // from caprice.c
CmdToGoLaunchParamHeader* CmdToGoParamsP = NULL; // from caprice.c
char AutoStartCommand[20] = {0}; // from caprice.c
UInt8 CPCStarted = 0; // from caprice.c
UInt8 SystemHalt = 1;
UInt8 BufferToWrite = 0;
UInt8 VideoFrameInitialDelay = 2; // 0 not alowed!!
tPreferences* prefP = NULL;
tEmulatorKeysStatus EmulatorKeysStatus;
UInt8 ScreenshotRequested = 0;
UInt8 DisplayEmuSpeed = 0;
tDriveEnum DriveSelected = DriveA;
UInt8 IsRendering = 0;

//FC!! will be placed into resources area
static MemPtr FDCCommandTableP = NULL;

//
// Routines
//
static tUShort MemCmp(tUChar* s1,
                      tUChar* s2,
                      tULong numBytes);
static tVoid MemMove(tUChar* destP,
                     tUChar* sourceP,
                     tULong numBytes);
static tVoid MemSet(tUChar* destP,
                    tULong numBytes,
                    tUChar value);
static tVoid StrCopy(tChar* destP,
                     tChar* srcP);


/***********************************************************************
 *
 *  Internal Functions
 *
 ***********************************************************************/
static void ShowTraceAlert(tULong Number)
/***********************************************************************
 *
 *  ShowTraceAlert
 *
 ***********************************************************************/
{
#ifdef _TRACE
  DisplayNativeTrace(Number);
#endif /* _TRACE */
}
/*----------------------------------------------------------------------------*/


static void* PalmOS_MemPtrNewLarge(tULong size)
/***********************************************************************
 *
 *  PalmOS_MemPtrNewLarge
 *
 ***********************************************************************/
{
void* ptrP;

  if (MemPtrNewLarge(size, &ptrP) != errNone)
    return NULL;

  return ptrP;
}
/*----------------------------------------------------------------------------*/

Err GetCPCFrameBuffers(void)
/***********************************************************************
 *
 *  GetCPCMenuBuffer
 *
 ***********************************************************************/
{

  if (OffScreenBuffer != NULL)
  {
    // give back the CPC double buffers
    WinDeleteWindow(OffScreenBuffer);
  }

    // get double buffer system for CPC engine
  printf("Create Frame Buffers\n");
  OffScreenBuffer = WinCreateOffscreenWindow(ON_SCR_WIDTH, ON_SCR_HEIGHT, nativeFormat, 1);

  if (OffScreenBuffer == NULL)
    return (memErrNotEnoughSpace);

  return (errNone);
}

void FlipAndCommitFrameBuffer(void)
/***********************************************************************
 *
 *  FlipAndCommitFrameBuffer
 *
 ***********************************************************************/
{
  tContextResources* resourcesP;

  while (IsRendering==1) {};

  //update resources
  if (NativeCPC != NULL && contextP != NULL)
  {
    //resourcesP = (tContextResources*)(contextP + CONTEXT_OFFSET_RESOURCES);
    //resourcesP->BmpOffScreenBits = (BufferToWrite == 0) ? OffScreenBuffer->data[1] : OffScreenBuffer->data[0];
    //resourcesP->BmpOffScreenBytesRow = OffScreenBuffer->width;
    //NativeCPC->BmpOffScreenBits = resourcesP->BmpOffScreenBits;
    //NativeCPC->BmpOffScreenBytesRow = resourcesP->BmpOffScreenBytesRow;

    NativeCPC->BmpOffScreenBits = (BufferToWrite == 0) ? OffScreenBuffer->data[1] : OffScreenBuffer->data[0];
    NativeCPC->BmpOffScreenBytesRow = OffScreenBuffer->width;
    NativeCPC->VDU->scr_base = (tULong*)NativeCPC->BmpOffScreenBits;
    NativeCPC->VDU->scr_root = (tULong*)NativeCPC->BmpOffScreenBits;

    BufferToWrite = (BufferToWrite == 0) ? 1 : 0;
  }
}

Err CPCSwitchAudio(UInt8 SinkId)
/***********************************************************************
 *
 *  CPCSwitchAudio, 0 Intern, 1 BT audio
 *
 ***********************************************************************/
{
    if (NativeCPC->PSG->snd_enabled == 1)
    {
      printf("Can't switch Audio, EMU still running");
      return(sysErrNotAllowed);
    }

    NativeCPC->prefP->SoundRenderer = SinkId;
    return(errNone);

}

Int32 SoundRender(UInt8 *data, Int32 len)
/***********************************************************************
 *
 *  SoundRender
 *
 ***********************************************************************/
{
  // this function handles all sound Rendering optins
  if (prefP->SoundRenderer == 1)
  {
    // system needs to use callback mode
    return (SoundVariableCallback(NativeCPC, data, len));
  }
  else
  {
    // system uses default sound queue mode
    return(SoundPush(NativeCPC));

  }
}

/*----------------------------------------------------------------------------*/

Err CPCFirstStart(void)
/***********************************************************************
 *
 *  CPCFirstStart
 *
 ***********************************************************************/
#undef CPCFIRSTSTART_TRACE_ENABLED
//#define CPCFIRSTSTART_TRACE_ENABLED

#ifdef CPCFIRSTSTART_TRACE_ENABLED
#  define CPCFIRSTSTART_TRACE_SHOW_INT(value) TRACE_SHOW_INT("CPCFirstStart", value)
#else /* CPCFIRSTSTART_TRACE_ENABLED */
#  define CPCFIRSTSTART_TRACE_SHOW_INT(value)
#endif /* CPCFIRSTSTART_TRACE_ENABLED */
{
tContextResources* resourcesP;
tContextHeader* ContextHeaderP;
Err ReturnCode = errNone;
Err Error;
UInt32 Size;

  CPCFIRSTSTART_TRACE_SHOW_INT(0);

  // Loading preferences
  Error = LirePreferences(&prefP);
  if (Error != errNone)
  {
    DBG_PRINT("Issues loading Preferences\n");
    return Error;
  }

  CPCFIRSTSTART_TRACE_SHOW_INT(1);

  do
  {
    //
    // Allocate context memory
    //
    MemPtrNewLarge(SIZETAB_CONTEXT,
                   (tVoid**)&contextP);
    if (contextP == NULL)
    {
      DBG_PRINT("NotEnoughMemoryAlert\n");
      ReturnCode = memErrNotEnoughSpace;
      continue;
    }

    MemSet(contextP,
           SIZETAB_CONTEXT,
           0);

    CPCFIRSTSTART_TRACE_SHOW_INT(2);

    // Prepare context header
    ContextHeaderP = (tContextHeader*)(contextP + CONTEXT_OFFSET_HEADER);




    // Pointer affectation into context memory
    resourcesP = (tContextResources*)(contextP + CONTEXT_OFFSET_RESOURCES);

    // assing the actual offscreen buffer
    resourcesP->BmpOffScreenBits = OffScreenBuffer->data[0];
    resourcesP->BmpOffScreenBytesRow = OffScreenBuffer->width;

    CPCFIRSTSTART_TRACE_SHOW_INT(3);

    // colours_rgb table
    resourcesP->colours_rgb = (void*)&colours_rgbH;
    resourcesP->colours_green = (void*)&colours_greenH;
    WinPalettePtr = (colours_rgb_entry*)MemPtrNew(NATIVECPC_COLOURS_NBELEMENT*sizeof(colours_rgb_entry));
    RGB565PalettePtr = (UInt16*)MemPtrNew(NATIVECPC_COLOURS_NBELEMENT*sizeof(UInt16));

    CPCFIRSTSTART_TRACE_SHOW_INT(8);

    // FDCCommandTable table
    Size = sizeof(FDCCommandTableH);
    FDCCommandTableP = (fdc_cmd_table_def*)MemPtrNew(Size);
    if (FDCCommandTableP == NULL)
    {
      DBG_PRINT("NotEnoughMemoryAlert\n");
      ReturnCode = memErrNotEnoughSpace;
      continue;
    }
    MemMove((tUChar*)FDCCommandTableP,
            (tUChar*)&FDCCommandTableH[0],
            Size);

    resourcesP->FDCCommandTableP = FDCCommandTableP;

    CPCFIRSTSTART_TRACE_SHOW_INT(4);

    // DAATable table
    resourcesP->DAATableH = (tVoid*)&DAATableH;
    resourcesP->DAATable = (tUShort*)&DAATableH;

    CPCFIRSTSTART_TRACE_SHOW_INT(5);
  }
  while (0);

  return ReturnCode;
}
/*----------------------------------------------------------------------------*/


static Err CPCStart(void)
/***********************************************************************
 *
 *  CPCStart
 *
 ***********************************************************************/
#undef CPCSTART_TRACE_ENABLED
//#define CPCSTART_TRACE_ENABLED

#ifdef CPCSTART_TRACE_ENABLED
#  define CPCSTART_TRACE_SHOW_INT(value) TRACE_SHOW_INT("CPCStart", value)
#else /* CPCSTART_TRACE_ENABLED */
#  define CPCSTART_TRACE_SHOW_INT(value)
#endif /* CPCSTART_TRACE_ENABLED */
{
UInt32 Result;
Err ReturnCode = errNone;
tContextResources* resourcesP;

  CPCSTART_TRACE_SHOW_INT(1);

  // Pointer affectation into context memory
  #ifdef __NEWMEMLAYOUT__
  NativeCPC = (tNativeCPC*)calloc(1,sizeof(tNativeCPC));
  #else
  NativeCPC = (tNativeCPC*)(contextP + CONTEXT_OFFSET_NATIVECPC);
  #endif
  sessionFilenameP = (char*)(contextP + CONTEXT_OFFSET_SESSIONFILENAME);
  resourcesP = (tContextResources*)(contextP + CONTEXT_OFFSET_RESOURCES);

  CPCSTART_TRACE_SHOW_INT(2);

  //
  // Initialiser la structure
  //
  NativeCPC->prefP = prefP;
  NativeCPC->contextP = contextP;
  NativeCPC->BmpOffScreenBits = resourcesP->BmpOffScreenBits;
  NativeCPC->BmpOffScreenBytesRow = resourcesP->BmpOffScreenBytesRow;


  NativeCPC->TraceAlertPtr = (tULong)ShowTraceAlert;
  NativeCPC->MemPtrNewPtr = (tMemPtrNewFct)PalmOS_MemPtrNewLarge; //??
  NativeCPC->MemPtrDeletePtr = (tMemPtrDeleteFct)free;


  NativeCPC->WinPalettePtr = WinPalettePtr;
  NativeCPC->RGB565PalettePtr = RGB565PalettePtr;
  NativeCPC->SoundCalculateLevelTablesPtr = (tULong)Sound_Calculate_Level_Tables;
  NativeCPC->HardKeyCPCKeyCodeA = prefP->CPCKeycodeA;
  NativeCPC->HardKeyCPCKeyCodeB = prefP->CPCKeycodeB;
  NativeCPC->HardKeyCPCKeyCodeC = prefP->CPCKeycodeC;
  NativeCPC->HardKeyCPCKeyCodeD = prefP->CPCKeycodeD;

  CPCSTART_TRACE_SHOW_INT(3);

  // Lire les tables de constantes
  NativeCPC->colours_rgb = resourcesP->colours_rgb;
  NativeCPC->colours_green = resourcesP->colours_green;
  NativeCPC->FDCCommandTable = resourcesP->FDCCommandTableP;
  NativeCPC->DAATable = resourcesP->DAATable;

  CPCSTART_TRACE_SHOW_INT(4);

  do
  {
    //
    // Init the Emulator
    //
    Result = Engine_CPCStart(NativeCPC);

    if (Result != errNone)
    {
      DBG_PRINT("NotEnoughMemoryAlert\n");
      ReturnCode = memErrNotEnoughSpace;
      continue;
    }

    CPCSTART_TRACE_SHOW_INT(5);

    CPCSetColor();

    CPCSTART_TRACE_SHOW_INT(6);

    CPCReset();

    CPCSTART_TRACE_SHOW_INT(7);

    //
    // CmdToGo Image Disk Loading
    //
    /*
    if (CmdToGoParamsP)
    {
    	tUChar* dataP;

      SetDriveFilename(NativeCPC->DriveA,
                       (tUChar*)CmdToGoParamsP + CmdToGoParamsP->descriptionOffset);

      // Allocate memory for whole disk data
      MemPtrNewLarge(CmdToGoParamsP->fileSize,
                     (void**)&dataP);
      if (dataP == NULL)
      {
        DBG_PRINT("NotEnoughMemoryAlert\n");
      }
      else
      {
        // Copy Disk Image Content
        MemMove(dataP,
                (tUChar*)CmdToGoParamsP + CmdToGoParamsP->fileOffset,
                CmdToGoParamsP->fileSize);

    	  LoadDiskImageFromMemory(NativeCPC->DriveA,
    	                          NativeCPC,
    	                          EndianSwap32(CmdToGoParamsP->fileSize),
    	                          (tUChar*)EndianSwap32(dataP));

        // Autostart enable
        if (prefP->AutoStartEnable)
        {
        	// Get command to execute
        	DiskAutostart(NativeCPC,
        		            NativeCPC->DriveA,
        		            AutoStartCommand);
        }
      }

      // Free CmdToGo Params
      MemPtrFree((MemPtr)CmdToGoParamsP);
      CmdToGoParamsP = NULL;
    } */

    CPCSTART_TRACE_SHOW_INT(8);
  }
  while (0);

  CPCSTART_TRACE_SHOW_INT(9);

  return ReturnCode;
}
/*----------------------------------------------------------------------------*/


void CPCStop(void)
/***********************************************************************
 *
 *  CPCStop
 *
 ***********************************************************************/
#undef CPCSTOP_TRACE_ENABLED
//#define CPCSTOP_TRACE_ENABLED

#ifdef CPCSTOP_TRACE_ENABLED
#  define CPCSTOP_TRACE_SHOW_INT(value) TRACE_SHOW_INT("CPCStop", value)
#else /* CPCSTOP_TRACE_ENABLED */
#  define CPCSTOP_TRACE_SHOW_INT(value)
#endif /* CPCSTOP_TRACE_ENABLED */
{
	CPCSTOP_TRACE_SHOW_INT(1);

  if (NativeCPC != NULL)
  {
	  CPCSTOP_TRACE_SHOW_INT(2);

    // Arr�ter l'�mulation.
    Engine_CPCStop(NativeCPC);
    NativeCPC = NULL;
  }

	CPCSTOP_TRACE_SHOW_INT(3);
}
/*----------------------------------------------------------------------------*/


void CPCLastStop(void)
/***********************************************************************
 *
 *  CPCLastStop
 *
 ***********************************************************************/
#undef CPCLASTSTOP_TRACE_ENABLED
//#define CPCLASTSTOP_TRACE_ENABLED

#ifdef CPCLASTSTOP_TRACE_ENABLED
#  define CPCLASTSTOP_TRACE_SHOW_INT(value) TRACE_SHOW_INT("CPCLastStop", value)
#else /* CPCLASTSTOP_TRACE_ENABLED */
#  define CPCLASTSTOP_TRACE_SHOW_INT(value)
#endif /* CPCLASTSTOP_TRACE_ENABLED */
{
tContextResources* resourcesP;

  CPCLASTSTOP_TRACE_SHOW_INT(1);

  // Pointer affectation into context memory
  resourcesP = (tContextResources*)(contextP + CONTEXT_OFFSET_RESOURCES);

  CPCLASTSTOP_TRACE_SHOW_INT(3);

  if (OffScreenBuffer != NULL)
  {
    WinDeleteWindow(OffScreenBuffer);
    OffScreenBuffer = NULL;
  }

  CPCLASTSTOP_TRACE_SHOW_INT(7);

  if (FDCCommandTableP != NULL)
  {
    MemPtrFree(FDCCommandTableP);
    FDCCommandTableP = NULL;
  }

  CPCLASTSTOP_TRACE_SHOW_INT(8);

  if (contextP != NULL)
  {
    MemPtrFree(contextP);
    contextP = NULL;
  }

  CPCLASTSTOP_TRACE_SHOW_INT(9);
}
/*----------------------------------------------------------------------------*/


void CPCReset(void)
/***********************************************************************
 *
 *  CPCReset
 *
 ***********************************************************************/
{
Err Result;

  // EMU Reset
  Result = Engine_CPCReset(NativeCPC);

#ifdef _TESTU
  if (Result >= appErrorClass)
  {
    TRACE_SHOW_HEX("TestU Native_CPCReset", Result);
  }
#endif /* _TESTU */

#ifdef _TRACE
  {
    tZ80* Z80 = (tZ80*)EndianSwap32((tULong)NativeCPC->Z80);
    Z80->Regs.breakpoint = NativeCPC->TraceInstruction ? EndianSwap32(TRACE_BREAKPOINT) : 0xffffffff;
  }
#endif /* _TRACE */

  //SetRenderingParameters(prefP->Rendering);
  //SetLightGunParameters(prefP,NativeCPC);
}
/*----------------------------------------------------------------------------*/


void CPCResetWithConfirmation(void)
/***********************************************************************
 *
 *  CPCResetWithConfirmation
 *
 ***********************************************************************/
{
    CPCReset();
}
/*----------------------------------------------------------------------------*/


Err CPCColdReset(const char* pathP,
                 const char* filenameP,
                 tAutomaticLoadType Load)
/***********************************************************************
 *
 *  CPCColdReset
 *
 ***********************************************************************/
#undef CPCCOLDRESET_TRACE_ENABLED
//#define CPCCOLDRESET_TRACE_ENABLED

#ifdef CPCCOLDRESET_TRACE_ENABLED
#  define CPCCOLDRESET_TRACE_SHOW_INT(value) TRACE_SHOW_INT("CPCColdReset", value)
#else /* CPCCOLDRESET_TRACE_ENABLED */
#  define CPCCOLDRESET_TRACE_SHOW_INT(value)
#endif /* CPCCOLDRESETTRACE_ENABLED */
{
Err Result;
tUChar RestoreOK = 0;

  CPCCOLDRESET_TRACE_SHOW_INT(1);

  if (CPCStarted)
  {
    SoundStop(NativeCPC); // !! Should be done before CPCStop !!
  }

  CPCCOLDRESET_TRACE_SHOW_INT(2);

  while (!RestoreOK)
  {
    RestoreOK = 1;

    if (CPCStarted)
    {
      CPCCOLDRESET_TRACE_SHOW_INT(3);

      CPCStop();
      CPCStarted = 0;
    }

    CPCCOLDRESET_TRACE_SHOW_INT(4);

    Result = CPCStart();

    if (Result != errNone)
    {
      DBG_PRINT("Fatal Error: CPC not started\n");
      return(Result);
    }
    else
    {
      DBG_PRINT("Success: CPC started\n");

      CPCStarted = 1;

      CPCCOLDRESET_TRACE_SHOW_INT(5);

      //StopKeyboard(&DIAKeyboardP);

      CPCCOLDRESET_TRACE_SHOW_INT(6);

      //StartDIAKeyboard();

      CPCCOLDRESET_TRACE_SHOW_INT(7);

      if (IsFileExist(pathP, filenameP) == cTrue)
      {
        if ( (Load == AutomaticLoad) ||
             ( (Load == LoadWithConfirmation) ) )
        {
          tCPCRestoreReturnCode RestoreReturnCode = Restore_OK;
          tPreferences RestorePref;

          CPCCOLDRESET_TRACE_SHOW_INT(8);

          // Restore previously saved CPC context
          // RestoreReturnCode = RestoreCPC(pathP,
          //                                filenameP,
          //                                &EmulatorKeysStatus,
          //                                &RestorePref,
          //                                contextP);

          //
          // Restore_OK
          //
          if (RestoreReturnCode == Restore_OK)
          {
            CPCCOLDRESET_TRACE_SHOW_INT(9);

            // Save current session filename
            StrCopy((tChar*)sessionFilenameP, (tChar*)filenameP);

            CPCCOLDRESET_TRACE_SHOW_INT(10);

            // Update colors due to NativeCPC->colours replacement.
            CPCSetColor();

            CPCCOLDRESET_TRACE_SHOW_INT(11);

            if (prefP->CPCKeycodeA != NativeCPC->HardKeyCPCKeyCodeA)
            {
            	prefP->CPCKeycodeA = NativeCPC->HardKeyCPCKeyCodeA;
              prefP->PreferencesChanged = 1;
            }
            if (prefP->CPCKeycodeB != NativeCPC->HardKeyCPCKeyCodeB)
            {
            	prefP->CPCKeycodeB = NativeCPC->HardKeyCPCKeyCodeB;
              prefP->PreferencesChanged = 1;
            }
            if (prefP->CPCKeycodeC != NativeCPC->HardKeyCPCKeyCodeC)
            {
            	prefP->CPCKeycodeC = NativeCPC->HardKeyCPCKeyCodeC;
              prefP->PreferencesChanged = 1;
            }
            if (prefP->CPCKeycodeD != NativeCPC->HardKeyCPCKeyCodeD)
            {
            	prefP->CPCKeycodeD = NativeCPC->HardKeyCPCKeyCodeD;
              prefP->PreferencesChanged = 1;
            }
            prefP->ScreenType = RestorePref.ScreenType;
            prefP->UseParados = RestorePref.UseParados;
            prefP->Use64kMemoryExtension = RestorePref.Use64kMemoryExtension;
            prefP->Mode2AntiAliasing = RestorePref.Mode2AntiAliasing;
            prefP->PreferencesChanged = 1;
          }
          //
          // Wrong_CPC_Model
          //
          else if (RestoreReturnCode == Wrong_CPC_Model)
          {
            CPCCOLDRESET_TRACE_SHOW_INT(12);

            prefP->CPCModel = RestorePref.CPCModel;

            RestoreOK = 0;  // Request start with new CPC Model
          }
          //
          // Error
          //
          else
          {
            //DisplayRestoreError(RestoreReturnCode,
            //                    &RestorePref);
          }
        }
      }
    }
  } /* while (!RestoreOK) */

  CPCCOLDRESET_TRACE_SHOW_INT(13);

  if (CPCStarted)
  {
    EmulatorKeysStatus.SoundKeyStatus = KeyReleased;

    CPCCOLDRESET_TRACE_SHOW_INT(14);

    Result = SoundStart(NativeCPC);

    if (Result == errNone)
    {
      if (prefP->SoundEnabled == true)
      {
        EmulatorKeysStatus.SoundKeyStatus = KeyPressed;
      }
    }

    //RestoreEmulatorKeysStatus(&EmulatorKeysStatus);
  }

  CPCCOLDRESET_TRACE_SHOW_INT(15);

  return (Result);
}
/*----------------------------------------------------------------------------*/

UInt32 CPCExecute(void)
/***********************************************************************
 *
 *  CPCExecute
 *
 ***********************************************************************/
{
  return(Engine_CPCExecute(NativeCPC));
}
/*----------------------------------------------------------------------------*/

UInt32 CPCSetColor(void)
/***********************************************************************
 *
 *  CPCSetColor
 *
 ***********************************************************************/
{

  NativeCPC->night_mode = prefP->NightModeActive;
  return (Engine_CPCSetColor(NativeCPC));
}
/*----------------------------------------------------------------------------*/


void EmulatorFreeze(void)
/***********************************************************************
 *
 *  EmulatorFreeze
 *
 ***********************************************************************/
{
  SystemHalt = 1;

  SoundPause(NativeCPC);

  // Restaurer le mask des touches mat�rielles
  //emulatorHardKeyMask = oldHardKeyMask;
  //KeySetMask(emulatorHardKeyMask);
}
/*----------------------------------------------------------------------------*/


void EmulatorUnfreeze(void)
/***********************************************************************
 *
 *  EmulatorUnfreeze
 *
 ***********************************************************************/
{
  SystemHalt = 0;

  SoundPlay(NativeCPC);

  // Appliquer le masque des touches mat�rielles pour l'�mulation
  //SetHardKeysMask(prefP,
  //                oldHardKeyMask);
}
/*----------------------------------------------------------------------------*/

Err CPCLoadDiskImage(char* PathnameP, char* FilenameP)
/***********************************************************************
 *
 *  CCPCLoadDiskImage
 *
 ***********************************************************************/
{
  tDrive* driveP = DriveSelected == DriveA ? NativeCPC->DriveA : NativeCPC->DriveB;

  // Charger le nouveau disque
  SetDriveFilename(driveP, FilenameP);
  if (LoadDiskImage(PathnameP,
                    driveP,
                    NativeCPC) != errNone)
  {
    // Problèmes pendant le chargement
    EjectDisk(driveP, NativeCPC);
    return (sysErrParamErr);
  }
  else
    return(errNone);
}


UInt16 CPCLoadDiskAndGetCatalog(char* PathnameP, char* FilenameP, char** Cat)
/***********************************************************************
 *
 *  CPCLoadDiskAndGetCatalog
 *
 ***********************************************************************/
{
  tDrive* driveP = DriveSelected == DriveA ? NativeCPC->DriveA : NativeCPC->DriveB;

  // Charger le nouveau disque
  SetDriveFilename(driveP, FilenameP);
  if (LoadDiskImage(PathnameP,
                    driveP,
                    NativeCPC) != errNone)
  {
    // Problèmes pendant le chargement
    EjectDisk(driveP, NativeCPC);
    return (0);
  }
  else
  {
    return (ReadDiskCatalogue(DriveSelected, 0, Cat, NativeCPC));
  }
}
/*----------------------------------------------------------------------------*/

Err CPCLoadKeymapFromConfigFile(const char* Key, char** Settings)
{
    tDrive* driveP = DriveSelected == DriveA ? NativeCPC->DriveA : NativeCPC->DriveB;
    return(LoadKeymapFromConfigFile(driveP, Key, Settings));
}


void CPCSwapDrive(void)
/***********************************************************************
 *
 *  CPCSwapDrive
 *
 ***********************************************************************/
{
	//SwapDrive(NativeCPC);
}
/*----------------------------------------------------------------------------*/


void CPCEjectBoth(void)
/***********************************************************************
 *
 *  CPCEjectBoth
 *
 ***********************************************************************/
{
	/*if (SaveAndEjectBoth(NativeCPC) == true)
	{
		// Refresh Drive Panel
    FrmUpdateForm(MainForm,
                  SubPanelRedrawUpdateCode);
	}*/
}
/*----------------------------------------------------------------------------*/

Boolean CPCHandleEvent(event_t* Event)
/***********************************************************************
 *
 *  CPCHandleEvent
 *
 ***********************************************************************/
{
  if (SystemHalt)
    return false;

  // handle the input device events
  if (Event->type == EVENT_TYPE_KEYPAD)
  {
    printf("CPCHandleEvent %d:%d\n",Event->keypad.pressed, Event->keypad.released);
    if (Event->keypad.pressed)
    {
      return KeyDownHandleEvent(Event);
    }
    else if (Event->keypad.released)
    {
      return KeyUpHandleEvent(Event);
    }
  }

  return false;
}
/*----------------------------------------------------------------------------*/

void CPCPushEvent(const CapriceEvent caprice_event)
/***********************************************************************
 *
 *  CPCPushEvent
 *
 ***********************************************************************/
{
	event_t ev = {.caprice.head.type = EVENT_TYPE_CAPRICE, .caprice.event = caprice_event};
	push_event(&ev);
}

/*----------------------------------------------------------------------------*/

tVoid CPCPoke(tULong addr, tULong val)
/***********************************************************************
 *
 *  CPCPoke
 *
 ***********************************************************************/
{
  Engine_Poke(NativeCPC, addr, val);
}

tUChar CPCPeek(tULong addr)
/***********************************************************************
 *
 *  CPCPeek
 *
 ***********************************************************************/
{
  return(Engine_Peek(NativeCPC, addr));
}


void SetRenderingParameters(tUChar Rendering)
/***********************************************************************
 *
 *  SetRenderingParameters
 *
 ***********************************************************************/
{
	switch (Rendering)
	{
		case 4: // Most realistic
		  VideoFrameInitialDelay = 1; // Render and Display video frame at each CPC frame completed
		  break;

		case 3:
		  VideoFrameInitialDelay = 2; // Render and Display video frame each 2 CPC frames completed
		  break;

		case 2:
		  VideoFrameInitialDelay = 3; // Render and Display video frame each 3 CPC frames completed
		  break;

		case 1:
		  VideoFrameInitialDelay = 4; // Render and Display video frame each 4 CPC frames completed
		  break;

		default: // 0 = fastest
		  VideoFrameInitialDelay = 5; // Render and Display video frame each 5 CPC frames completed
	}
}
/*----------------------------------------------------------------------------*/


void SetLightGunParameters(tPreferences* prefP,
                           tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  SetLightGunParameters
 *
 ***********************************************************************/
{
	NativeCPC->lightgun_random_crtc = prefP->OnScreenMagnumGunActive;
}
/*----------------------------------------------------------------------------*/


void AppStop(void)
/***********************************************************************
 *
 *  AppStop
 *
 ***********************************************************************/
{

  // Save CPC context before forms close
  /*
  if (AllowSaveSession)
  {
    if ( (prefP->AutomaticContextSave) && (prefP != NULL) )
    {
      tCPCSaveReturnCode ReturnCode = SaveCPC(DEFAULT_CAPRICE_PATH,
                                              LASTSESSION_FILENAME,
                                              contextP);

      if (ReturnCode == File_Not_Created)
      {
        printf("CPCSaveCreationAlert\n");
      }
      else if (ReturnCode == Write_Error)
      {
        printf("BadCPCSaveAlert\n");
      }
    }
  }
  */
  StopKeyboard(&MiniKeyboardP);

  if (NativeCPC != NULL)
  {
    SoundStop(NativeCPC); // !! Should be done before CPCStop !!
    SaveAndEjectBoth(NativeCPC);
  }

  CPCStop();
  CPCLastStop();


  // Sauvegarder les préférences
  if (prefP != NULL)
  {
    if (prefP->PreferencesChanged)
    {
      EcrirePreferences(prefP);
    }

    MemPtrFree(prefP);
  }

  // Free CmdToGo Params
  if (CmdToGoParamsP != NULL)
  {
    MemPtrFree((MemPtr)CmdToGoParamsP);
  }
}
/*----------------------------------------------------------------------------*/


//==============================================================================
//
// Routines
//
//==============================================================================

static tUShort MemCmp(tUChar* s1,
                      tUChar* s2,
                      tULong numBytes)
/***********************************************************************
 *
 *  MemCmp
 *
 ***********************************************************************/
{
  while (numBytes--)
  {
    if (*(s1++) != *(s2++))
      return 1;
  }

  return 0;
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

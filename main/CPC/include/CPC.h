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

#ifndef CPC_H
#define CPC_H

#include "types.h"
#include "sections.h"
#include "Native_CPC.h"


#include "gbuf.h"
#include "event.h"
#include "display.h"

#define AUTOSTARTHOLDTIME_MS      100    // Key pressed duration for autostart (ms)
#define AUTOSTARTRELEASETIME_MS   50     // Key released duration for autostart (ms)
#define AUTOSTARTRESETTIME_MS     1500   // Time to wait from Reset before sending keys for autostart (ms)
#define AUTOSTARTHOLD_CYCLES          ((AUTOSTARTHOLDTIME_MS / CYCLES_MS) + 1)
#define AUTOSTARTRELEASE_CYCLES       ((AUTOSTARTRELEASETIME_MS / CYCLES_MS) + 1)
#define AUTOSTARTRESET_CYCLES         ((AUTOSTARTRESETTIME_MS / CYCLES_MS) + 1)

typedef enum
{
  NoLoad,
  AutomaticLoad,
  LoadWithConfirmation
} tAutomaticLoadType;


typedef struct
{
  UInt32 magicID;
  UInt32 descriptionOffset;
  UInt32 descriptionSize;
  UInt32 fileOffset;
  UInt32 fileSize;
} CmdToGoLaunchParamHeader; // from caprice.h

extern tNativeCPC* NativeCPC;
extern tUChar* contextP;
extern gbuf_t* OffScreenBuffer;
extern UInt8 SystemHalt;
extern UInt8 BufferToWrite;
extern tPreferences* prefP;
extern UInt8 VideoFrameInitialDelay;
extern char AutoStartCommand[20];
extern UInt8 ScreenshotRequested;
extern UInt8 DisplayEmuSpeed;
extern UInt8 IsRendering;

extern Err  CPCFirstStart(void);
extern void CPCLastStop(void);

extern void CPCStop(void);
extern void CPCReset(void);
extern void CPCResetWithConfirmation(void);
extern Err  CPCColdReset(const char* pathP,
                         const char* filenameP,
                         tAutomaticLoadType Load);

extern UInt32 CPCSetColor(void);
extern UInt32 CPCExecute(void);
extern void EmulatorFreeze(void);
extern void EmulatorUnfreeze(void);
extern Err GetCPCFrameBuffers(void);
extern void FlipAndCommitFrameBuffer(void);
extern void CPCSwapDrive(void);
extern void CPCEjectBoth(void);
extern void CPCPushEvent(const CapriceEvent caprice_event);
extern Boolean CPCHandleEvent(event_t* Event);
extern Err CPCLoadDiskImage(char* PathnameP, char* FilenameP);
extern UInt16 CPCLoadDiskAndGetCatalog(char* PathnameP, char* FilenameP, char** Cat);
extern void AppStop(void);

//extern void SetRenderingParameters(tUChar Rendering);

//extern void SetLightGunParameters(tPreferences* prefP,
//                                  tNativeCPC* NativeCPC);

#endif /* ! CPC_H */

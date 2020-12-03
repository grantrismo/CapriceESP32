#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

// Driver/Hardware
#include <backlight.h>
#include <display.h>
#include <event.h>
#include <keypad.h>
#include <sdcard.h>
#include <system.h>
#include <battery.h>
#include <status_bar.h>
#include <settings.h>
#include <audio.h>


#include "./common/types.h"
#include "./NativeCPC/include/Native_CPC.h"
#include "CPC.h"
#include "Keyboard.h"
#include "Routines.h"
#include "Sound.h"
#include "ttgui.h"

#ifndef SIM
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_task_wdt.h"
#include "soc/periph_defs.h"

#else
#include <sys/time.h>
#endif

#define errNone                       0x0000  // No error

#define _SHOW_FPS

 static int app_init(void)
 {
  do {

    // Offscreen Buffers allocation
    if (GetCPCFrameBuffers() != errNone)
    {
     printf("Error on getting Frame Buffers\n");
     continue;
    }

    // hardware or SDL display init
    display_init();
    backlight_init();
    backlight_percentage_set(50);
    keypad_init();
    event_init();
    system_led_init();
    battery_init();
    settings_init();


    // Setup sdcard and display error message on failure
  	// TODO: Make it nonfatal so user can still browse SPIFFS or so

  	if ((sdcard_init("/sd")) != 0) {
  		printf("Please insert the sdcard and restart the device.");
      continue;
    }



    // Init MiniKeyboard
    if (StartMiniKeyboard() != errNone)
    {
     printf("Error on Starting up MiniKeyboard\n");
     continue;
    }
    // Init the TTGUI
    if (ttgui_PanelConstructor(OffScreenBuffer, DISPLAY_WIDTH, DISPLAY_HEIGHT) != TTGUI_ERRNONE)
      continue;

    // TTGUI Color settings
    ttgui_setFgColorIndex(10);
    ttgui_setBgColorIndex(4);

    // Coldstart Emulator
    if (CPCFirstStart() != errNone)
      continue;

    // Todo load with configuration
    if (CPCColdReset(NULL, NULL, NoLoad) != errNone)
      continue;

    // first full screen update (  SystemHalt == 0)
    display_update();

    // goCPC
    EnableJoystick();
    SystemHalt = 0;

    // Enable Sound
    if (prefP->SoundEnabled == 1)
      EnableSound();

    return 0;

  }//do loop
  while (0);

  return -1;
}

static void app_shutdown(void)
{
	// stop and deallocate CPC
  AppStop();

  sdcard_deinit();
	display_poweroff();
	system_reboot_to_firmware();

}

#ifdef SIM
static char start_dir_buf[PATH_MAX];
static char *start_dir = start_dir_buf;
#else
#define start_dir "/sd/"
#endif

/***********************************************************************
 *
 *  AppEventLoop
 *
 ***********************************************************************/
void app_main_task(void *arg)
{
	if (app_init() != 0) {
		app_shutdown();
    printf("Error on Init Emulator\n");
		return;
  }

  event_t event;

  Err error;
  UInt32 Condition;
  UInt32 Ticks;
  UInt32 NextCycle = 0;
  UInt32 TicksPerSecond;
  UInt32 TicksPerCycle;
  UInt32 Tstart;
  UInt32 Tacc = 0;
  UInt32 NextSecond;
  UInt32 NextAutoToggle = 0;
  UInt32 CycleCount = 0;
  #ifdef PATCH_CPC_TRUE_SPEED
  UInt32 LongCycleCount = 0;
  #endif /* PATCH_CPC_TRUE_SPEED */
  UInt32 Percentage = 0;
  UInt32 HardKeysState;
  UInt32 CurrentKeyState;
  UInt32 oldHardKeyState = 0;
  UInt32 oldAttachKeyState = 0;
  UInt16 oldCoordSys;
  UInt8 RunEmulation;
  UInt8 oldFDCLed = 0;
  UInt8 DiskDisplayCounter = 0;
  UInt8 VideoFrameDelay;
  UInt32 SoundSamples = 0;
  #ifdef _DEBUG
  UInt32 SoundCbCount = 0;
  UInt32 SoundCbSamples = 0;
  UInt8 StepByStep = 0;
  #endif /* _DEBUG */
  #ifdef _SHOW_FPS
  UInt32 FPS;
  UInt32 FPSCount = 0;
  tULong ASPS;
  tULong numSoundSamples = 0;
  #endif /* _SHOW_FPS */
  char* indexAutoStartP;
  UInt32 AutoStartDuration = 0;
  UInt8 AutoStartStatus = 0;
  UInt8 AutoStartActive = 0;
  tUShort oldSHIFTandCTRLState;

  TicksPerSecond = SysTicksPerSecond();
  TicksPerCycle = TicksPerSecond / CYCLES_PER_SECOND;
  NextSecond = TimGetTicks() + TicksPerSecond;
  VideoFrameDelay = VideoFrameInitialDelay;

#ifdef _PROFILE
  NativeCPC->profileStates=0;

  for (int i=0;i<(PROFILE_NB_ITEMS+3);i++)
    printf("%s|", profileLabel[i]);
  printf("\n");
#endif

  do
  {
    // Emulator is running
    if (!SystemHalt)
    {
      do
      {
        // Get pending event
        poll_event(&event);
        Ticks = TimGetTicks();
        RunEmulation = 1;

        //
        // CPC True speed
        //
        if (prefP->CPCTrueSpeed)
        {
#ifdef SIM
          usleep(5000);
#endif
          if (!NextCycle)
          {
            NextCycle = Ticks + TicksPerCycle;
          }
          // Sound buffer has been filled by engine.
          else if ( (Condition == EC_SOUND_BUFFER) &&
                    (prefP->SoundEnabled) ) // Prevent Freeze when sound turned off while Condition = EC_SOUND_BUFFER
          {
            SoundSamples = SoundPush(NativeCPC);
            numSoundSamples += SoundSamples;
            printf("Soundbuffer full %d(%d)\n",SoundSamples, SND_BUFFER_SIZE);
            // Do not allow emulation while sound buffer not entirely read by sound stream callback
            if (IsBufferRead() == false)
            {
              RunEmulation = 0;
            }
          }
          // CPC True speed limitation
          else if (Condition == EC_CYCLE_COUNT)
          {

            if (Ticks < NextCycle)
            {
              // Wait for next true speed step
              RunEmulation = 0;
#ifdef PATCH_CPC_TRUE_SPEED
              LongCycleCount = 0; // Short cycle detected
#endif /* PATCH_CPC_TRUE_SPEED */
            }
            else
            {
              // Maintain true speed steps
              NextCycle += TicksPerCycle;
#ifdef PATCH_CPC_TRUE_SPEED
              LongCycleCount++;

              // Shorten too long delay
              /*if (Ticks > NextCycle)
              {
                NextCycle = Ticks + TicksPerCycle;
              }*/
#endif /* PATCH_CPC_TRUE_SPEED */
            }
          }
        }
        else
        {
          NextCycle = 0;
#ifdef PATCH_CPC_TRUE_SPEED
          LongCycleCount = 0;
#endif /* PATCH_CPC_TRUE_SPEED */
        }

        // Perform emulation
        if (RunEmulation)
        {
          //
          // Auto toggle
          //
          if (!AutoToggleActive)
          {
            if (NextAutoToggle)
            {
              NextAutoToggle = 0;

              SetCursorAndJoystickState(KeyCurrentState(),
                                        &oldHardKeyState);
            }
          }
          else
          {
            if ( (Ticks >= NextAutoToggle) || (!NextAutoToggle) )
            {
              ToggleAutoToggle();
              NextAutoToggle = Ticks + AutoToggleDurationInTicks;

              SetCursorAndJoystickState(KeyCurrentState(),
                                        &oldHardKeyState);
            }
          }

          // =================
          // Run Emulation
          // =================
          Tstart = TimGetTicks();
          Condition = CPCExecute();
          Tacc += (TimGetTicks() - Tstart);

          // =======================
          // VDU Frame completed
          // =======================
          if (Condition == EC_FRAME_COMPLETE)
          {
            //printf("**************Frame Complete %p:%d\n",(NativeCPC->BmpOffScreenBits), *(uint8_t*)(NativeCPC->BmpOffScreenBits));
            tCRTC* CRTC = NativeCPC->CRTC;

            // do we have new samples
            numSoundSamples += SoundPush(NativeCPC);

            if (!CRTC->stop_rendering)
            {

#ifdef _SHOW_FPS
              FPSCount++;
#endif /* _SHOW_FPS */

              // Stay out of Native coordinate system
              if (ScreenshotRequested)
              {
                ScreenshotRequested = 0;
                // Allow 5-ways navigation
                //KeySetMask(oldHardKeyMask);
                // Save screenshot
                //SaveScreenshot(NativeCPC);
                // Restore key mak for emulation
                //KeySetMask(emulatorHardKeyMask);
              }
              // swap fram buffer
              //FlipAndCommitFrameBuffer();
              // Transfer offscreen to draw window
              display_update();
            }
            if (!VideoFrameDelay)
            {
                CRTC->stop_rendering = 0;
                VideoFrameDelay = VideoFrameInitialDelay;
            }
            else
            {
                CRTC->stop_rendering = 1;
                VideoFrameDelay--;
            }

          } /* if (Condition == EC_FRAME_COMPLETE) */

          // ===============
          // Cycle end
          // ===============
          else if (Condition == EC_CYCLE_COUNT)
          {
            // Update speed measure
            //printf("******************** Cycle Complete %p\n",NativeCPC->BmpOffScreenBits);

            // Push the Sound Buffer
            numSoundSamples += SoundPush(NativeCPC);

            CycleCount++;
            //-----
            // Next Second reports
            //-----

            if (Ticks >= NextSecond)
            {
              Percentage = !DisplayEmuSpeed ? 0 : CycleCount * 100 / CYCLES_PER_SECOND;
              NextSecond += TicksPerSecond;
              CycleCount = 0;

#ifndef SIMPinnedToCore
              //esp_task_wdt_reset();
#endif

#ifdef _SHOW_FPS
              printf("%3d|%5d|%4d|",FPS,ASPS,Tacc);
              FPS = FPSCount;
              ASPS = numSoundSamples/SND_SAMPLE_SIZE ;
              FPSCount = 0;
              numSoundSamples = 0;
              Tacc=0;
#endif /* _SHOW_FPS */

#ifdef _PROFILE
              // disable logging
              if (NativeCPC->profileStates==3)
              {
                  NativeCPC->profileStates=2;
                  printf("\n Log Done \n");
              }

              // start logging
              if ((NativeCPC->profileCounter[0]>900000) && (NativeCPC->profileStates==0))
              {
                  printf("Logging Start\n");
                  NativeCPC->profileStates=1;
              }

              // log and clear the counteres
              for (int i=0;i<PROFILE_NB_ITEMS;i++)
              {
                printf("%7d|",NativeCPC->profileCounter[i]);
                NativeCPC->profileCounter[i] = 0;
              }
#endif
              printf("\n");

#ifndef SIM
              vTaskDelay(1);
#endif
            }

            // if Autostart enable
            if ( (prefP->AutoStartEnable) && (AutoStartCommand[0]) )
            {
              if (!AutoStartActive)
              {
                AutoStartDuration++;
                if (AutoStartDuration > AUTOSTARTRESET_CYCLES)
                {
                  AutoStartActive = 1;
                }
              }
              // Start Autostart command
              else if (AutoStartCommand[0])
              {
                switch (AutoStartStatus)
                {
                  case 0: // Start
                  {
                    indexAutoStartP = AutoStartCommand;
                    // Prepare Hold Duration
                    AutoStartDuration = AUTOSTARTHOLD_CYCLES;
                    // Save SHIFT and CTRL state before autostart
                    oldSHIFTandCTRLState = GetKeySHIFTandCTRLState();
                    // Press fist key
                    KeyboardSetAsciiKeyDown(*indexAutoStartP);
                    // Next Step
                    AutoStartStatus = 1;
                  }
                  break;

                  case 1: // Hold Period
                  {
                    if (!(--AutoStartDuration))
                    {
                      // Release key
                      KeyboardSetAsciiKeyUp(*indexAutoStartP);
                      // Prepare Release Duration
                      AutoStartDuration = AUTOSTARTRELEASE_CYCLES;
                      // Next Step
                      AutoStartStatus = 2;
                    }
                  }
                  break;

                  case 2: // Release key
                  {
                    if (!(--AutoStartDuration))
                    {
                      // If another key to press
                      if (*(indexAutoStartP+1))
                      {
                        // Prepare Hold Duration
                        AutoStartDuration = AUTOSTARTHOLD_CYCLES;
                        // Press next key
                        KeyboardSetAsciiKeyDown(*(++indexAutoStartP));
                        AutoStartStatus = 1;
                      }
                      else // End of string
                      {
                        // Stop and Prepare for next Autostart
                        AutoStartCommand[0] = 0;
                        AutoStartStatus = 0;
                        AutoStartActive = 0;
                        AutoStartDuration = 0;

                        // Restore SHIFT and CTRL state before autostart
                        SetKeySHIFTandCTRLState(oldSHIFTandCTRLState);
                      }
                    }
                  }
                  break;
                }
              }
            }

            ReleasePressedKey();
            // Cursor/Joystick detection
            SetCursorAndJoystickState(KeyCurrentState(),
                                      &oldHardKeyState);

          } /* if (Condition == EC_CYCLE_COUNT) */

        }
      } while (event.type == EVENT_TYPE_UNUSED);
    } //!SystemHalt

    // Emulator is halted
    else
    {
      // Emulation stopped, typical event loop
      wait_event(&event);

      // Update speed measure
      Ticks = TimGetTicks();
      NextCycle = 0;
      NextSecond = Ticks + TicksPerSecond;
      CycleCount = 0;
      Percentage = 0;
      NextAutoToggle = 0;
#ifdef _SHOW_FPS
      FPS = 0;
      FPSCount = 0;
#endif /* _SHOW_FPS */
    }


    //
    // Events management
    //
    CurrentKeyState = KeyCurrentState();

    // Keypad Input to be handled on the virtual Keyboard
    if (NewRockerAttach == RockerAsVirtualKeyboard)
    {
      if ((CurrentKeyState & KEYPAD_MENU) == 0)
      {
        CPCHandleEvent(&event);
      }
    }

    // Manage how the Rocker (LRUD-FIRE) is used
    RockerAttachManager(CurrentKeyState, &oldAttachKeyState);


    // Handle Menu Events
    if (NewRockerAttach == RockerAsMenuControl)
    {
      if (ttgui_windowManager(&event) == TTGUI_NEEDGUIUPDATE)
        {
          printf("ttgui_update request received\n");
          display_update();
        }
    }


    // Handle Caprice callback events
    if (event.type == EVENT_TYPE_CAPRICE)
    {
      printf("Caprice Callback Event received\n");
      BufferToWrite = 0;
      if (event.caprice.event == CapriceEventKeyboardRedraw)
      {
        PrepareKeyboard(ActiveKeyboardP);
        display_update();
      }

      if (event.caprice.event == CapriceEventMenuRedraw)
      {
        // let previous rendering finish within 35ms
        vTaskDelay(35);

        // call the ttgui and swow it
        ttgui_setup();
        display_update();
      }

      if (event.caprice.event == CapriceEventAboutRedraw)
      {
        BufferToWrite = 0;
        //ttgui_about();
        //display_update();
        EmulatorUnfreeze();
      }
    }

    // handle SDL update events
    if (event.type == EVENT_TYPE_UPDATE)
    {
      display_update();
    }

    // Mask hard keys events for emulation
    // Mask may have been changed by EmulatorFreeze/EmulatorUnfrezze
    // called by FrmDispatchEvent
    //KeySetMask(emulatorHardKeyMask);
  }  while (event.type != EVENT_TYPE_QUIT);

  // done -> exit programm
  app_shutdown();

} // Main loop end
/*----------------------------------------------------------------------------*/


void app_main(void)
{
#ifdef SIM
	app_main_task(NULL);
#else
	//xTaskCreate(app_main_task, "main_task", 16384 * 2, NULL, 5, NULL);
  xTaskCreatePinnedToCore(&app_main_task, "main_task", 16384 * 2, NULL,  5 , NULL,1);
#endif
}

#ifdef SIM

int main(int argc, char const *argv[])
{
	if (argc > 1) {
		if (realpath(argv[1], (char*)&start_dir_buf) == NULL) {
			perror("Could not resolve start path");
			start_dir = start_dir_buf;
			return -1;
		}
	} else {
		start_dir = "/home/";
	}

	app_main();
	return 0;
}

#endif

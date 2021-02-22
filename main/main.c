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
#include "a2dp.h"
#include "ringbuf.h"

#else
#include <sys/time.h>
#include <SDL2/SDL.h>
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

    // Setup sdcard and display error message on failure
    sdcard_init("/sd");
    
    // hardware or SDL display init
    display_init();
    backlight_init();
    backlight_percentage_set(50);
    keypad_init();
    event_init();
    system_led_init();
    battery_init();
    settings_init();

    // get sound Buffer;
    if (SoundBufferAlloc() != errNone)
    {
      printf("Error on getting Sound Buffers\n");
      continue;
    }

    // try to start BT
#ifndef SIM
    //heap_caps_print_heap_info(MALLOC_CAP_8BIT);
    printf("BT Classic Startup\n");
    a2dp_service_start(false);
    //heap_caps_print_heap_info(MALLOC_CAP_8BIT);
#endif

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

    return (0);

  }//do loop
  while (0);

  return -1;
}

static void app_shutdown(void)
{
	// stop and deallocate CPC
  AppStop();
  audio_shutdown();
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
  UInt32 TickLast;
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
  UInt8 NumFrameSubmitted = 0;
  #endif /* _SHOW_FPS */
  char* indexAutoStartP;
  UInt32 AutoStartDuration = 0;
  UInt8 AutoStartStatus = 0;
  UInt8 AutoStartActive = 0;
  tUShort oldSHIFTandCTRLState;
  float ted_i = 0.0;
  float ted_p = 0.0;
  float ted_error = 0.0;
  Int32 ted_ctr = 0;
  UInt8 SdCardMissing = 0;

  TicksPerSecond = SysTicksPerSecond();
  TicksPerCycle = TicksPerSecond / CYCLES_PER_SECOND;
  NextSecond = TimGetTicks() + TicksPerSecond;
  VideoFrameDelay = VideoFrameInitialDelay;

  static char osd_tmp_buf[20];

#ifndef SIM
  // setup the audio ringbuffer for BT
  ringbuf_handle_t rb;
  rb = rb_create(i2s_get_rb_size()*2 ,1);

  if (rb == NULL)
  {
    app_shutdown();
    printf("Error on Init Audio ringbuffer\n");
    return;
  }
  printf("Audio ringbuffer @%p,%p\n",rb,SoundBufferP);
  a2dp_set_rb_read(rb);
  audio_set_rb_read(rb);

#endif
  // Start sound
  SoundPlay(NativeCPC);

  // SD card Check
  if (!sdcard_present())
    ttgui_osdRegister("NO SDCARD?!", 5000);


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

          if (!NextCycle)
          {
            NextCycle = Ticks + TicksPerCycle;
          }
          // Sound buffer has been filled by engine.
          else if ( (Condition == EC_SOUND_BUFFER) &&
                    (prefP->SoundEnabled) ) // Prevent Freeze when sound turned off while Condition = EC_SOUND_BUFFER
          {
            //printf("Soundbuffer full %d(%d) -> Reset buffer\n",SoundSamples, NativeCPC->PSG->FilledBufferSize );
            //SoundBufferReset(NativeCPC);

            // Do not allow emulation while sound buffer not entirely read by sound stream callback
          }
          // CPC True speed limitation
          else if (Condition == EC_CYCLE_COUNT)
          {
            if (Ticks <= NextCycle)
            {
              // Wait for next true speed step
              RunEmulation = 0;
#ifdef PATCH_CPC_TRUE_SPEED
              LongCycleCount = 0; // Short cycle detected
#endif /* PATCH_CPC_TRUE_SPEED */
            }
            else
            {

/*#ifdef SIM
              if ((prefP->SoudRenderer == 1) && (NativeCPC->PSG->snd_enabled == 1) && NativeCPC->PSG->FilledBufferSize>0)
              {
              // calculate the PI Control error for frame rate
                ted_error = (4096.0 - NativeCPC->PSG->FilledBufferSize);
                ted_p = ted_error * 1e-3;
                ted_i = ted_i + ted_error * 1e-4;
                ted_ctr = (Int32)(ted_p + ted_i);
                if (ted_ctr<-5) {ted_ctr=-5;}
                if (ted_ctr>5) {ted_ctr=5;}
                printf("TED %d,%d\n", NativeCPC->PSG->FilledBufferSize, ted_ctr);

              }

#endif*/

              // Maintain true speed steps
              NextCycle += TicksPerCycle - ted_ctr;
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
            //numSoundSamples += SoundRender(NULL, 0);

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
              ttgui_osdExecute();
              display_update();
              NumFrameSubmitted++;
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
            //PSG->pbSndBuffer = (tUChar*)&SoundBufferP[SoundBufferCurrent][0];
            //PSG->pbSndBufferEnd = PSG->pbSndBuffer + SND_BUFFER_SIZE;
#ifdef SIM
            numSoundSamples += SoundPush(NativeCPC);
#else
            tPSG* PSG = NativeCPC->PSG;
            int bytes_moved = rb_write(rb, PSG->pbSndBuffer, PSG->FilledBufferSize , 0);

            //printf("S:P:%d,%d\n",bytes_moved, PSG->FilledBufferSize);
            numSoundSamples += bytes_moved;
            if (bytes_moved != PSG->FilledBufferSize)
             {
                printf("rb overflow %d, %d\n",bytes_moved, PSG->FilledBufferSize);
                SoundBufferReset(NativeCPC);
                rb_reset(rb);
              }
            else
            {
              PSG->FilledBufferSize = 0;
              PSG->snd_bufferptr = PSG->pbSndBuffer;
            }

            if (prefP->SoundRenderer == 0)
            {
              audio_submit();
            }

#endif



            CycleCount++;
            //-----
            // Next Second reports
            //-----
#ifdef SIM
            SDL_Delay(5);
#endif
            if (Ticks >= NextSecond)
            {
              Percentage = !DisplayEmuSpeed ? 0 : CycleCount * 100 / CYCLES_PER_SECOND;
              NextSecond += TicksPerSecond;
              CycleCount = 0;

#ifndef SIMPinnedToCore
              //esp_task_wdt_reset();
#endif

#ifdef _SHOW_FPS
              printf("%3d|%5d|%4d|%3d",FPS,ASPS,Tacc,timeDelta/NumFrameSubmitted);
              FPS = FPSCount;
              ASPS = numSoundSamples/SND_SAMPLE_SIZE ;
              FPSCount = 0;
              numSoundSamples = 0;
              Tacc=0;
              timeDelta = 0;
              NumFrameSubmitted = 0;
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
    if ((ttgui_getOsdState() & TTGUI_OSD_BLOCKKEYS) == 0)
    {
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
            display_update();
          }
      }
    }


    // Handle Caprice callback events
    if (event.type == EVENT_TYPE_CAPRICE)
    {
      BufferToWrite = 0;
      if (event.caprice.event == CapriceEventKeyboardRedraw)
      {
        PrepareKeyboard(ActiveKeyboardP);
        display_update();
      }

      else if (event.caprice.event == CapriceEventMenuRedraw)
      {

#ifndef SIM
        // let previous rendering finish within 35ms
        a2dp_set_emu_state_idle();
        vTaskDelay(35);
#endif

        // call the ttgui and swow it
        ttgui_setup();
        display_update();
      }

      else if (event.caprice.event == CapriceEventRestartAudioPipe)
      {
        ttgui_osdStop();
      }
      else if (event.caprice.event == CapriceEventTimerEvent)
      {
        if (ttgui_osdManager(&event) == TTGUI_NEEDGUIUPDATE)
          {
            display_update();
          }
      }
      else if (event.caprice.event == CapriceEventVolumeOkEvent)
      {
          sprintf(osd_tmp_buf, "%c %d%%",0x1D, audio_volume_get());
          ttgui_osdRegister(osd_tmp_buf, 1000);
      }
      else if (event.caprice.event == CapriceEventVolumeFailEvent)
      {
          sprintf(osd_tmp_buf, "%c N/A",0x1D);
          ttgui_osdRegister(osd_tmp_buf, 1000);
      }
      else if (event.caprice.event == CapriceEventBattery10Event)
      {
          static BatteryInfo battery_info;
          if (battery_read(&battery_info) == 0)
          {
            sprintf(osd_tmp_buf, "Bat: %d%%",battery_info.percentage + 1);
            ttgui_osdRegister(osd_tmp_buf, 1000);
          }
      }
      else if (event.caprice.event == CapriceEventBrightnessOkEvent)
      {
          sprintf(osd_tmp_buf, "Dbl: %d%%",backlight_percentage_get());
          ttgui_osdRegister(osd_tmp_buf, 1000);
      }

    }
    else if (event.type == EVENT_TYPE_A2DP)
    {
        switch(event.a2dp.event)
        {
          case A2dpEventDeviceConnected:
            ttgui_osdString("BT Paired", 1000, SystemHalt);
            break;
          case A2dpEventDeviceUnconnected:
            ttgui_osdString("BT Unpaird", 1000, SystemHalt);
            break;
          case A2dpEventMediaStarted:
            ttgui_osdString("BT Ready", 1000, SystemHalt);
            ttgui_osdTermTimer();
#ifndef SIM
            prefP->A2dpMediaStates = a2dp_get_states();
            if (prefP->A2dpMediaStates & 0x01)
            {
              prefP->SoundRenderer = 1;
              SoundBufferReset(NativeCPC);
              rb_reset(rb);
              i2s_set_emu_state_idle();
              a2dp_set_emu_state_running();

            }
#endif
            break;
          case A2dpEventMediaStopped:
            ttgui_osdString("BT Stopped", 1000, SystemHalt);
            break;

          default:
            break;
        }
        display_update();
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

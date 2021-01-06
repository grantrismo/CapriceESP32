/*
    Caprice32 - Amstrad CPC Emulator
    (c) Copyright 1997-2005 Ulrich Doewich

    CaPriCe for Palm OS - Amstrad CPC 464/664/6128 emulator for Palm devices
    Copyright (C) 2006-2011 by Fr�d�ric Coste
    CaPriCe Forever - Amstrad CPC 464/664/6128 emulator
    Copyright (C) 2014-2015 by Fr�d�ric Coste

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
#include <string.h>

#include "Sound.h"
#include "CPC.h"
#include "Math_math.h"
#include "Prefs.h"
#include "Routines.h"

#include "audio.h"
#include "a2dp.h"



//===================
// PATCH begin
#ifdef _PATCH_ENABLE

#endif /* _PATCH_ENABLE */
// PATCH end
//===================


#ifdef __cplusplus
extern "C" {
#endif

#define SOUND_MUTE_VOLUME           0

#define SOUND_VOLUME_STEPS          100
#define SOUND_VOLUME_CHANGE_STEP    2

#define SOUND_SAMPLE_GAIN           1 // 1024 = 100% Palm Sample Gain
#define SOUND_SCALE_VOLUME(vol)     ((int)vol)

#define errNone                       0x0000  // No error
#define memErrorClass                 0x0100  // Memory Manager
// copied from MemoryMgr.h
#define memErrNotEnoughSpace          (memErrorClass | 2)

#define	sysErrorClass			           	0x0500	// System Manager
#define	sysErrTimeout							    (sysErrorClass | 1)
#define	sysErrParamErr							  (sysErrorClass | 2)
#define	sysErrNoFreeResource					(sysErrorClass | 3)
#define	sysErrNoFreeRAM						    (sysErrorClass | 4)
#define	sysErrNotAllowed						  (sysErrorClass | 5)

// Amplitude table (c)Hacker KAY
static const tUShort Amplitudes_AY[16] =
{
   0, 836, 1212, 1773, 2619, 3875, 5397, 8823,
   10392, 16706, 23339, 29292, 36969, 46421, 55195, 65535,
};

// Routines
static tVoid MemMove(tUChar* destP,
                     tUChar* sourceP,
                     tULong numBytes);

// Sound buffer handling
tUChar** SoundBufferP[2] = {NULL};

tUChar SoundBufferCurrent = 0;
tULong SoundBytesToWrite = 0;
tUChar SoundBufferRead = 0;
tUChar *pbSndBufferLastP = NULL;

Err SoundBufferAlloc()
{
  SoundBufferP[0]=calloc(SND_BUFFER_SIZE,1);
  if (SoundBufferP[0] == NULL)
    return (memErrNotEnoughSpace);
  else
  {
    SoundBufferP[1]=SoundBufferP[0];
    return (errNone);
  }
}

tVoid SoundBufferFree()
{
  if (SoundBufferP[0] != NULL)
  {
    free(SoundBufferP[0]);
    SoundBufferP[0] = NULL;
    SoundBufferP[1] = NULL;
  }
}

Err SoundBufferReset(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  SoundBufferReset
 *
 ***********************************************************************/
#undef SOUNDSTART_TRACE_ENABLED
//#define SOUNDSTART_TRACE_ENABLED

#ifdef SOUNDSTART_TRACE_ENABLED
#  define SOUNDSTART_TRACE_SHOW_INT(value) TRACE_SHOW_INT("SoundStart", value)
#else /* SOUNDSTART_TRACE_ENABLED */
#  define SOUNDSTART_TRACE_SHOW_INT(value)
#endif /* SOUNDSTART_TRACE_ENABLED */
{
  tPSG* PSG;

  if (NativeCPC == NULL)
  {
    return (sysErrNoFreeResource);
  }

  PSG = (tPSG*)(NativeCPC->PSG);

  if (PSG == NULL)
  {
    return (memErrNotEnoughSpace);
  }

  if ((PSG->pbSndBuffer == NULL) || (PSG->snd_bufferptr == NULL))
  {
    return (memErrNotEnoughSpace);
  }

  SOUNDSTART_TRACE_SHOW_INT(1);

  //
  // Prepare callback parameters
  //
  SoundBufferRead = 0;
  SoundBufferCurrent = 0;
  SoundBytesToWrite = 0;
  SoundBufferRead = 0;
  PSG->pbSndBuffer = (tUChar*)SoundBufferP[SoundBufferCurrent];
  PSG->pbSndBufferEnd = PSG->pbSndBuffer + SND_BUFFER_SIZE;
  PSG->snd_bufferptr = PSG->pbSndBuffer;
  PSG->FilledBufferSize = 0;
  pbSndBufferLastP = PSG->pbSndBuffer;

  return errNone;
}

Err SoundStart(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  SoundStart
 *
 ***********************************************************************/
#undef SOUNDSTART_TRACE_ENABLED
//#define SOUNDSTART_TRACE_ENABLED

#ifdef SOUNDSTART_TRACE_ENABLED
#  define SOUNDSTART_TRACE_SHOW_INT(value) TRACE_SHOW_INT("SoundStart", value)
#else /* SOUNDSTART_TRACE_ENABLED */
#  define SOUNDSTART_TRACE_SHOW_INT(value)
#endif /* SOUNDSTART_TRACE_ENABLED */
{
  tPSG* PSG;

  if (NativeCPC == NULL)
  {
    return (sysErrNoFreeResource);
  }

  PSG = (tPSG*)(NativeCPC->PSG);

  if (PSG == NULL)
  {
    return (memErrNotEnoughSpace);
  }

  if ((PSG->pbSndBuffer == NULL) || (PSG->snd_bufferptr == NULL))
  {
    return (memErrNotEnoughSpace);
  }

  SOUNDSTART_TRACE_SHOW_INT(1);

  //
  // Prepare callback parameters
  //
  SoundBufferRead = 0;
  SoundBufferCurrent = 0;
  SoundBytesToWrite = 0;
  SoundBufferRead = 0;
  PSG->pbSndBuffer = (tUChar*)SoundBufferP[SoundBufferCurrent];
  PSG->pbSndBufferEnd = PSG->pbSndBuffer + SND_BUFFER_SIZE;
  PSG->snd_bufferptr = PSG->pbSndBuffer;
  PSG->FilledBufferSize = 0;
  pbSndBufferLastP = PSG->pbSndBuffer;

  // init hardware
  audio_init();

  return errNone;
}
/*----------------------------------------------------------------------------*/


tVoid SoundStop(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  SoundStop
 *
 ***********************************************************************/
{
  audio_shutdown();
}
/*----------------------------------------------------------------------------*/


tVoid SoundPlay(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  SoundPlay
 *
 ***********************************************************************/
#undef SOUNDPLAY_TRACE_ENABLED
//#define SOUNDPLAY_TRACE_ENABLED

#ifdef SOUNDPLAY_TRACE_ENABLED
#  define SOUNDPLAY_TRACE_SHOW_INT(value) TRACE_SHOW_INT("SoundPlay", value)
#else /* SOUNDPLAY_TRACE_ENABLED */
#  define SOUNDPLAY_TRACE_SHOW_INT(value)
#endif /* SOUNDPLAY_TRACE_ENABLED */
{

tPSG* PSG = (tPSG*)(NativeCPC->PSG);

  if (audio_device_active() == false)
    return;

  if (prefP->SoundEnabled == 0)
    return;

  if (SystemHalt)
    return;

  if (NativeCPC == NULL)
    return;

  SOUNDPLAY_TRACE_SHOW_INT(1);

  SOUNDPLAY_TRACE_SHOW_INT(2);

  audio_volume_set(SOUND_SCALE_VOLUME(prefP->SoundVolume));

  SOUNDPLAY_TRACE_SHOW_INT(3);

  PSG->snd_enabled = 1;

  if (prefP->SoundRenderer == 0)
  {
    i2s_set_emu_state_running();
    audio_play();
  }
  else
    a2dp_set_emu_state_running();


  SOUNDPLAY_TRACE_SHOW_INT(4);
}
/*----------------------------------------------------------------------------*/


tVoid SoundPause(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  SoundPause
 *
 ***********************************************************************/
{
  if (audio_device_active() == false)
    return;

  if (NativeCPC == NULL)
    return;

  ((tPSG*)(NativeCPC->PSG))->snd_enabled = 0;
  i2s_set_emu_state_idle();
  a2dp_set_emu_state_idle();

  if (prefP->SoundEnabled == 1)
  {
    audio_volume_set(SOUND_MUTE_VOLUME);

    audio_pause();
  }
}
/*----------------------------------------------------------------------------*/

Int32 SoundPush(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  SoundCallback
 *
 ***********************************************************************/
{

  tPSG* PSG = NativeCPC->PSG;

  if ((NativeCPC == NULL) || (prefP->SoundEnabled == 0))
    return (0);

  // make sure Emulator is acive
  PSG->snd_enabled = 1;

  // Get and Reset Current Size
  SoundBytesToWrite = (PSG->FilledBufferSize);

  // Data available?
  if (SoundBytesToWrite == 0)
    return (0);

  // debug
  //printf("SoundPush: %4d : %u\n",SoundBytesToWrite,TimGetTicks());

  // Flip the buffers ...
  SoundBufferCurrent = (SoundBufferCurrent == 0) ? 1 : 0;

  // ... and push it to the speaker
  audio_submit();

  // Update flip/flop Buffer
  PSG->pbSndBuffer = (tUChar*)SoundBufferP[SoundBufferCurrent];
  PSG->pbSndBufferEnd = PSG->pbSndBuffer + SND_BUFFER_SIZE - 1;
  PSG->snd_bufferptr = PSG->pbSndBuffer;
  PSG->FilledBufferSize = 0;

  SoundBufferRead = 1;
  return (SoundBytesToWrite);
}
/*----------------------------------------------------------------------------*/

Int32 SoundVariableCallback(tNativeCPC* NativeCPC, UInt8 *data, Int32 len)
/***********************************************************************
 *
 *  SoundVariableCallback
 *
 ***********************************************************************/
{


  // the inits are:
  /*
    PSG->pbSndBuffer = (tUChar*)&SoundBufferP[SoundBufferCurrent][0];
    PSG->pbSndBufferEnd = PSG->pbSndBuffer + SND_BUFFER_SIZE;
    PSG->snd_bufferptr = PSG->pbSndBuffer;
    PSG->FilledBufferSize = 0;
  */
  // everything is in samples (2*2 = 4 bytes order) -> ULong cast
  // in case EMU not ready, return silence
  //if (len == 0)
    //return (len);

  //printf("S:%d,%d\n",TimGetTicks(),len);

  // need to upsample, ESP_IDF has multiple bugs on differenet to 44100 kHz
  if ((NativeCPC == NULL) || (prefP->SoundEnabled == 0))
  {
      memset(data,0,len);
      return (len);
  }


  // enable emulator Sound
  tPSG* PSG = NativeCPC->PSG;
  PSG->snd_enabled = 1;

  tULong* LastPosP = (tULong*)pbSndBufferLastP;
  tULong* DataP = (tULong*)data;
  tULong* MaxPosP = ((tULong*)PSG->pbSndBufferEnd);

  Int32 i;
  Int32 request_len = len/2;
  Int32 available_len = PSG->FilledBufferSize;
  Int32 serve_len = (request_len > available_len) ?  available_len : request_len;

  if (available_len < request_len)
  {
    memset(data,0,len);
    return (len);
  }

  Int32 blockd_len = (MaxPosP - LastPosP) * SND_SAMPLE_SIZE;
  Int32 blocka_len = (blockd_len > serve_len) ? serve_len : blockd_len;
  Int32 blockb_len = serve_len - blocka_len;
  Int32 blockc_len = request_len - serve_len;

  PSG->FilledBufferSize = (available_len - serve_len);

  if (blocka_len > 0)
  {
      for (i=0;i<blocka_len;i+=SND_SAMPLE_SIZE)
      {
        *DataP++ = *LastPosP;
        *DataP++ = *LastPosP++;
      }


  }

  if (blockb_len > 0)
  {
      LastPosP = (tULong*)PSG->pbSndBuffer;
      for (i=0;i<blockb_len;i+=SND_SAMPLE_SIZE)
      {
        *DataP++ = *LastPosP;
        *DataP++ = *LastPosP++;
      }

  }

  if (blockc_len > 0)
  {
    for (i=0;i<blockc_len;i+=SND_SAMPLE_SIZE)
    {
      //*DataP++ = 0;
      //*DataP++ = 0;
    }
  }

  // advance buffer and status information
  pbSndBufferLastP = (LastPosP >= MaxPosP) ? PSG->pbSndBuffer : (tUChar*)LastPosP;
  //PSG->FilledBufferSize = (available_len - serve_len);
  SoundBufferRead = (available_len <= request_len) ? 1 : 0;

  //printf("%4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d,%d\n",request_len, available_len, serve_len, blocka_len, blockb_len, blockc_len, blockd_len, PSG->FilledBufferSize,TimGetTicks());

  return(len);
}

tVoid Sound_Calculate_Level_Tables(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  Sound_Calculate_Level_Tables
 *
 ***********************************************************************/
{
  tDouble k;
  tDouble Log2 = LOG_2;
  tPSG* PSG;
  tLong i;
  tLong b;
  tLong l;
  tLong r;
  tLong Index_A;
  tLong Index_B;
  tLong Index_C;
  tLong Index_AR;
  tLong Index_BR;
  tLong Index_CR;
  tLong* Level_AL;
  tLong* Level_AR;
  tLong* Level_BL;
  tLong* Level_BR;
  tLong* Level_CL;
  tLong* Level_CR;
  tLong* Level_PP;
  tLong LevelTape;

  Index_A  = AUDIO_INDEX_AL;
  Index_B  = AUDIO_INDEX_BL;
  Index_C  = AUDIO_INDEX_CL;
  Index_AR = AUDIO_INDEX_AR;
  Index_BR = AUDIO_INDEX_BR;
  Index_CR = AUDIO_INDEX_CR;

  // PNO to 68k
  PSG = (tPSG*) (NativeCPC->PSG);
  Level_AR = (tLong*) (PSG->Level_AR);
  Level_AL = (tLong*) (PSG->Level_AL);
  Level_BR = (tLong*) (PSG->Level_BR);
  Level_BL = (tLong*) (PSG->Level_BL);
  Level_CR = (tLong*) (PSG->Level_CR);
  Level_CL = (tLong*) (PSG->Level_CL);
  Level_PP = (tLong*) (PSG->Level_PP);

  l = Index_A + Index_B + Index_C;
  r = Index_AR + Index_BR + Index_CR;

#if SND_STEREO == 1
  if (l < r)
  {
    l = r;
  }
#else
  l += r;
  Index_A += Index_AR;
  Index_B += Index_BR;
  Index_C += Index_CR;
#endif

#if SND_16BITS == 0
  r = 127;
#else
  r = 32767;
#endif

  if (l == 0) l++; // Avoid division by 0
  l = 255 * r / l;

  for (i = 0; i < 16; i++)
  {
    b = (tLong)math_rint(Index_A / 255.0 * Amplitudes_AY[i]);
    b = (tLong)(b / 65535.0 * l);
    Level_AL[i * 2] = b;
    Level_AL[(i * 2) + 1] = b;
    b = (tLong)math_rint(Index_AR / 255.0 * Amplitudes_AY[i]);
    b = (tLong)math_rint(b / 65535.0 * l);
    Level_AR[i * 2] = b;
    Level_AR[(i * 2) + 1] = b;
    b = (tLong)math_rint(Index_B / 255.0 * Amplitudes_AY[i]);
    b = (tLong)math_rint(b / 65535.0 * l);
    Level_BL[i * 2] = b;
    Level_BL[(i * 2) + 1] = b;
    b = (tLong)math_rint(Index_BR / 255.0 * Amplitudes_AY[i]);
    b = (tLong)math_rint(b / 65535.0 * l);
    Level_BR[i * 2] = b;
    Level_BR[(i * 2) + 1] = b;
    b = (tLong)math_rint(Index_C / 255.0 * Amplitudes_AY[i]);
    b = (tLong)math_rint(b / 65535.0 * l);
    Level_CL[i * 2] = b;
    Level_CL[(i * 2) + 1] = b;
    b = (tLong)math_rint(Index_CR / 255.0 * Amplitudes_AY[i]);
    b = (tLong)math_rint(b / 65535.0 * l);
    Level_CR[i * 2] = b;
    Level_CR[(i * 2) + 1] = b;
  }

  k = math_exp(AUDIO_SAMPLE_VOLUME * Log2 / AUDIO_PREAMP_MAX) - 1;
  for (i = 0; i < 32; i++)
  {
    Level_AL[i] = (tLong) (math_rint(Level_AL[i] * k));
    Level_AR[i] = (tLong) (math_rint(Level_AR[i] * k));
    Level_BL[i] = (tLong) (math_rint(Level_BL[i] * k));
    Level_BR[i] = (tLong) (math_rint(Level_BR[i] * k));
    Level_CL[i] = (tLong) (math_rint(Level_CL[i] * k));
    Level_CR[i] = (tLong) (math_rint(Level_CR[i] * k));
  }

#if SND_16BITS == 0 // 8 bits per sample?
  LevelTape = -(tLong)math_rint((TAPE_VOLUME / 2) * k);
#else
  LevelTape = -(tLong)math_rint((TAPE_VOLUME * 128) * k);
#endif
  PSG->LevelTape  = (tLong) (LevelTape);


  for (i = 0, b = 255; i < 256; i++) // calculate the 256 levels of the Digiblaster/Soundplayer
  {
    Level_PP[i] = (tLong) (-math_rint((b << 8) * l / 65535.0 * k));
    b--;
  }
}
/*----------------------------------------------------------------------------*/



tBool IsBufferRead()
/***********************************************************************
 *
 *  IsBufferRead
 *
 ***********************************************************************/
{

  if (!SoundBufferRead)
    return false;

  SoundBufferRead = 0;

  return true;
}
/*----------------------------------------------------------------------------*/


Err SoundIncreaseVolume(tUChar step)
/***********************************************************************
 *
 *  SoundIncreaseVolume
 *
 ***********************************************************************/
{
  if (prefP == NULL)
    return (memErrNotEnoughSpace);

  if (prefP->SoundRenderer != 0)
    return (sysErrNotAllowed);

  prefP->SoundVolume += step;
  if (prefP->SoundVolume > SOUND_VOLUME_STEPS)
  {
    prefP->SoundVolume = SOUND_VOLUME_STEPS;
  }

  audio_volume_set(SOUND_SCALE_VOLUME(prefP->SoundVolume));

  // Ask for Preferences save
  prefP->PreferencesChanged = 1;
  return (errNone);
}
/*----------------------------------------------------------------------------*/


Err SoundDecreaseVolume(tUChar step)
/***********************************************************************
 *
 *  SoundDecreaseVolume
 *
 ***********************************************************************/
{

  if (prefP == NULL)
    return (memErrNotEnoughSpace);

  if (prefP->SoundRenderer != 0)
    return (sysErrNotAllowed);

  if (prefP->SoundVolume <= step)
  {
    prefP->SoundVolume = 0;
  }
  else
  {
    prefP->SoundVolume -= step;
  }

  audio_volume_set(SOUND_SCALE_VOLUME(prefP->SoundVolume));

  // Ask for Preferences save
  prefP->PreferencesChanged = 1;
  return (errNone);
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

#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

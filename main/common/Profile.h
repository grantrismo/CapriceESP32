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

#ifndef PROFILE_H
#define PROFILE_H

#ifdef _PROFILE

// List of functions
enum
{
  // 0
  PROFILE_CPCExecute_Main_Loop  = 0,
  PROFILE_z80_pfx_cb,
  PROFILE_z80_pfx_dd,
  PROFILE_z80_pfx_ddcb,
  PROFILE_z80_pfx_ed,
  PROFILE_z80_pfx_fd,
  PROFILE_z80_pfx_fdcb,
  PROFILE_z80_IN_handler,
  PROFILE_z80_OUT_handler,
  PROFILE_z80_OUT_handler_GA,
  PROFILE_z80_OUT_handler_CRTC,
  PROFILE_z80_OUT_handler_ROM,
  PROFILE_z80_OUT_handler_PPI,
  PROFILE_z80_OUT_handler_FDC,
  PROFILE_ga_memory_manager,
  // 10
  PROFILE_ga_init_banking,
  PROFILE_Z80_WAIT_STATES,
  PROFILE_read_mem,
  PROFILE_write_mem,
  PROFILE_PSG_WRITE,
  PROFILE_video_access_memory_Loop,
  PROFILE_video_draw_border,
  PROFILE_video_draw,
  PROFILE_audio_Synthesizer_Stereo16,
  PROFILE_audio_Synthesizer_Stereo16_Loop,
  // 20
  PROFILE_audio_Synthesizer_Stereo8,
  PROFILE_audio_Synthesizer_Stereo8_Loop,
  PROFILE_fdc_read_data,
  PROFILE_fdc_write_data,
  PROFILE_fdc_read_status,
  PROFILE_fdc_overrun,
  PROFILE_HandleSpecialEvent,
  PROFILE_SysHandleEvent,
  PROFILE_MenuHandleEvent,
  PROFILE_AppHandleEvent,
  // 30
  PROFILE_FrmDispatchEvent,
  PROFILE_audio_Synthesizer_Mono8,
  PROFILE_audio_Synthesizer_Mono8_Loop,
  PROFILE_Debug,

  // MUST be the last one
  PROFILE_NB_ITEMS
};
//

static const char profileLabel[][50] =
{
  // Other
  "Frames per Second",
  "Audio Samples",
  "CPC execute time [ms]",
  // 0
  "CPCExecute_Main_Loop",
  "z80_pfx_cb",
  "z80_pfx_dd",
  "z80_pfx_ddcb",
  "z80_pfx_ed",
  "z80_pfx_fd",
  "z80_pfx_fdcb",
  "z80_IN_handler",
  "z80_OUT_handler",
  "z80_OUT_handler_GA",
  "z80_OUT_handler_CRTC",
  "z80_OUT_handler_ROM",
  "z80_OUT_handler_PPI",
  "z80_OUT_handler_FDC",
  "ga_memory_manager",
  // 10
  "ga_init_banking",
  "Z80_WAIT_STATES",
  "read_mem",
  "write_mem",
  "PSG_WRITE",
  "video_access_memory_Loop",
  "video_draw_border",
  "video_draw",
  "audio_Synthesizer_Stereo16",
  "audio_Synthesizer_Stereo16_Loop",
  // 20
  "audio_Synthesizer_Stereo8",
  "audio_Synthesizer_Stereo8_Loop",
  "fdc_read_data",
  "fdc_write_data",
  "fdc_read_status",
  "fdc_overrun",
  "HandleSpecialEvent",
  "SysHandleEvent",
  "MenuHandleEvent",
  "AppHandleEvent",
  // 30
  "FrmDispatchEvent",
  "audio_Synthesizer_Mono8",
  "audio_Synthesizer_Mono8_Loop",
  "Debug",
};

#endif /* _PROFILE*/

#endif /* PROFILE_H */

#pragma once

#include <stdint.h>

#include "gbuf.h"
#include "rect.h"

#define DISPLAY_WIDTH (320)
#define DISPLAY_HEIGHT (240)
#define DISPLAY_KEYB_HEIGHT (40)

#ifndef SIM
  #ifndef STOP_DISPLAY_FUNCTION
      #define STOP_DISPLAY_FUNCTION display_stop
  #endif

  #ifndef RESUME_DISPLAY_FUNCTION
      #define RESUME_DISPLAY_FUNCTION display_resume
  #endif
#endif
int32_t timeDelta;

void display_init(void);
void display_poweroff(void);
void display_clear(uint16_t color);
void display_update();
void display_update_rect(rect_t r);
void display_drain(void);
void display_screenshot(const char *path);
void display_stop();
void display_resume();
void display_lock();
void display_unlock();

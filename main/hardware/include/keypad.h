#pragma once

#include <stdint.h>

enum { KEYPAD_UP = 1,
       KEYPAD_RIGHT = 2,
       KEYPAD_DOWN = 4,
       KEYPAD_LEFT = 8,
       KEYPAD_SELECT = 16,
       KEYPAD_START = 32,
       KEYPAD_A = 64,
       KEYPAD_B = 128,
       KEYPAD_MENU = 256,
       KEYPAD_VOLUME = 512,
};

// KeyCurrentState : 5-way rocker -> Mapped to Odroid keypad
#define keyBitRockerUp              KEYPAD_UP
#define keyBitRockerDown            KEYPAD_DOWN
#define keyBitRockerLeft            KEYPAD_LEFT
#define keyBitRockerRight           KEYPAD_RIGHT
#define keyBitRockerButtonA         (KEYPAD_A)
#define keyBitRockerButtonB         (KEYPAD_B)
#define keyBitRockerCenter          (KEYPAD_A | KEYPAD_B)
#define keyBitRockerMenu            (KEYPAD_B | KEYPAD_MENU)
#define keyBitRockerViKeyboard      (KEYPAD_A | KEYPAD_MENU)
#define keyBitRockerNextPanelUp   (KEYPAD_MENU | KEYPAD_UP)
#define keyBitRockerNextPanelDown   (KEYPAD_MENU | KEYPAD_DOWN)
#define keyBitRockerNextPanelLeft   (KEYPAD_MENU | KEYPAD_LEFT)
#define keyBitRockerNextPanelRight   (KEYPAD_MENU | KEYPAD_RIGHT)

void keypad_init(void);
uint16_t keypad_sample(void);
uint16_t keypad_debounce(uint16_t sample, uint16_t *changes);

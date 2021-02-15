#pragma once

void backlight_init(void);
void backlight_percentage_set(int value);
int is_backlight_initialized(void);
void backlight_percentage_decrease (int step);
void backlight_percentage_increase (int step);
int backlight_percentage_get(void);

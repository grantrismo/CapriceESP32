#ifndef GBUF_H
#define GBUF_H

#include <stdint.h>

// Graphics buffer, mainly used for framebuffer and graphics
typedef struct {
	uint16_t width;
	uint16_t height;
	uint16_t bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
	uint16_t num_of_blocks;
	uint32_t block_len;
	uint8_t **data;
} gbuf_t;

extern gbuf_t* WinCreateOffscreenWindow(uint16_t width, uint16_t height, uint16_t bytes_per_pixel, uint16_t num_of_blocks);
extern void WinDeleteWindow(gbuf_t *g);
extern void blit_ind_565(void* source, void* dest, int width, int height);
extern void blit_zero_565(void* source, void* dest, int width, int height);
extern void blit_keyboard_565(void* source, void* dest, int width, int height);

#endif /* ! GBUF */

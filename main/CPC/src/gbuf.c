#include <gbuf.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef SIM
#include "esp_heap_caps.h"
#endif

#include "CPC.h"
#include "Keyboard.h"

/*
#define RGB565(r, g, b)  ( (((r>>3) & 0x1F) << 11) | (((g>>2) & 0x3F) << 5) | ((b>>3) & 0x1F) )

#ifdef SIM
#define ESWAP(a) (a)
#else
#define ESWAP(a) ( (a >> 8) | ((a) << 8) )
#endif
*/

gbuf_t *WinCreateOffscreenWindow(uint16_t width, uint16_t height, uint16_t bytes_per_pixel, uint16_t num_of_blocks)
{
	gbuf_t *g = calloc(1, sizeof(gbuf_t));
	if (g == NULL)
		return NULL;

	// allocate the pointer array
	g->data = (uint8_t**)calloc(1,num_of_blocks * sizeof(void*));

	if (g->data == NULL) {
		free(g);
		return NULL;
	}

	// fill the structure
	g->width = width;
	g->height = height;
	g->bytes_per_pixel = bytes_per_pixel;
	g->num_of_blocks = num_of_blocks;
	g->block_len = width * height * bytes_per_pixel;

	// assign the blocks
	for (int i=0;i<num_of_blocks;i++)
	{

	#ifdef SIM
		g->data[i] = (uint8_t*)calloc(1, g->block_len);
	#else
		if (i==0)
			g->data[i] = heap_caps_calloc(1, g->block_len , MALLOC_CAP_32BIT | MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
		else
			g->data[i] = heap_caps_calloc(1, g->block_len , MALLOC_CAP_32BIT | MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
	#endif

		if (g->data[i] == NULL)
		{
			WinDeleteWindow(g);
			printf("Allocation Error\n");
			return NULL;
		}
		else
		{
			printf("Allocated Buffer %d : %p\n",i, g->data[i]);
		}
	}

	return g;
}

void WinDeleteWindow(gbuf_t *g)
{
	for (int i=0; i<g->num_of_blocks; i++)
	{
			if (g->data[i] != NULL)
				free(g->data[i]);
	}

	free(g->data);
	free(g);
}

void blit_ind_565(void* source, void* dest, int width, int height)
{
	int i,j;

	unsigned char ind;
	uint16_t *outp = (uint16_t*)dest;
	uint8_t  *inp  = (uint8_t*)source;

	for(i=0;i<height;i++)
   {
		for(j=0;j<width;j++)
      {
			ind=inp[i*width + j];
			(*outp++)= NativeCPC->RGB565PalettePtr[ind];
		}
	}
}

void blit_zero_565(void* source, void* dest, int width, int height)
{
	int i,j;

	unsigned char ind = 0;
	uint16_t *outp = (uint16_t*)dest;
	uint8_t  *inp  = (uint8_t*)source;

	for(i=0;i<height;i++)
   {
		for(j=0;j<width;j++)
      {
			ind++;

			if (ind==17)
				ind=0;

			(*outp++) = NativeCPC->RGB565PalettePtr[ind];

		}
	}
}

void blit_keyboard_565(void* source, void* dest, int width, int height)
{
	int i,j;
	unsigned  char ind;
	uint16_t *outp = (uint16_t*)dest;
	uint8_t  *inp  = (uint8_t*)source;

	for(i=0;i<height;i++)
   {
		for(j=0;j<width;j++)
      {
			ind = inp[i*width + j];
			(*outp++)= MiniKeyboardColorMap565P[ind];
		}
	}
}

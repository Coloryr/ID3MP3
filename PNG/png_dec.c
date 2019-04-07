#include "png_dec.h"
#include "upng.h"
#include <stdio.h>
#include <malloc.h>
#include <stdint.h>
#include <string.h>
#include "ff.h"
#include "lcd.h"
#include "show.h"
#include "mp3player.h"
#include "guix.h"

#define HI(w) (((w) >> 8) & 0xFF)
#define LO(w) ((w)&0xFF)

static uint8_t decoder_buffer[20 * 1024];

void raw_data_convert(upng_t *upng, uint8_t *in_buffer)
{
	uint32_t pixel_num = upng->width * upng->height;
	uint32_t color_idx = 0;

	uint32_t depth = 3;
	unsigned int y,x;

	for (y = upng->width - 1; y >= 0; --y)
		for (uint32_t x = 0; x != upng->width; ++x)
		{
			pic_phy.draw_point(realx + picinfo.S_XOFF, realy + picinfo.S_YOFF, *pencolor);
		}
if (color_idx != pixel_num * 3)
{
	printf("convert err!\n");
}
}

int test(void)
{
	upng_t *upng;
	unsigned width, height, depth;
	unsigned x, y, d;
	uint8_t header[54];
	fmp3_pic = (FIL *)mymalloc(sizeof(FIL)); //�����ڴ�
	f_open(fmp3_pic, "0:/test.png", FA_READ);
	upng = upng_new_from_file(fmp3_pic, info.pic_local);
	if (upng != NULL)
	{
		upng_decode_to_buffer(upng, (unsigned char *)decoder_buffer, sizeof(decoder_buffer));
		if (upng_get_error(upng) != UPNG_EOK)
		{
			LCD_ShowString(0, 0, 320, 16, 16, "error:upng_error");
			return 0;
		}
	}

	width = upng_get_width(upng);
	height = upng_get_height(upng);
	depth = upng_get_bpp(upng) / 8;

	if (upng_get_format(upng) == UPNG_RGB8 || upng_get_format(upng) == UPNG_RGBA8)
	{
		//encodeToBMP(header, width, height);
		raw_data_convert((uint8_t *)upng_get_buffer(upng), width, height, &bmp_buffer[54]);
	}

	upng_free(upng);
	return 0;
}

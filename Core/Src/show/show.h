#ifndef __SHOW_H__
#define __SHOW_H__
#include "main.h"
#include "fatfs.h"
#include "stdlib.h"

void time_go(void);
void fft_show_oneband(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t curval, uint16_t topval);
void FFT_post(uint16_t *pbuf);
void show_mp3_pic(void *pdata);
void show_all(uint8_t mode);

extern uint8_t lcd_bit;
extern FIL *fmp3_pic;

#define pic_show_x 0      //图片显示的位置_x
#define pic_show_y 0      //图片显示的位置_y
#define pic_show_size 240 //图片显示的大小

#endif

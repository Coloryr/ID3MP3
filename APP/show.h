#ifndef __SHOW_H_
#define __SHOW_H_
#include "sys.h"
#include "ff.h"
#include "stdlib.h"

void time_go(void);
void fft_show_oneband(u16 x, u16 y, u16 width, u16 height, u16 curval, u16 topval);
void FFT_post(u16 *pbuf);
void show_mp3_pic(void *pdata);
void show_all(void);

extern u8 time_k;
extern u8 lcd_bit;
extern FIL* fmp3;
extern u8 name_show;

#define pic_show_x   0						//图片显示的位置_x
#define pic_show_y   0						//图片显示的位置_y
#define pic_show_size   240				//图片显示的大小


#endif

#ifndef __SHOW_H_
#define __SHOW_H_
#include "sys.h"
#include "stdlib.h"

void time_go(void);
void fft_show_oneband(u16 x, u16 y, u16 width, u16 height, u16 curval, u16 topval);
void FFT_post(u16 *pbuf);
void mp3_index_show(u16 index, u16 total);
void mp3_vol_show(u8 vol);
void mp3_msg_show(void);
	
extern u8 time_k;
extern u8 lcd_bit;
extern FIL* fmp3;

#define pic_show_x   0						//图片显示的位置_x
#define pic_show_y   16						//图片显示的位置_y
#define pic_show_size   224				//图片显示的大小

void show_mp3_pic(void *pdata);
void mp3_next(void);

#endif

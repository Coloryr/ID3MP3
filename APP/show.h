#ifndef __SHOW_H_
#define __SHOW_H_
#include "sys.h"
#include "stdlib.h"

void time_go(void);
void fft_show_oneband(u16 x, u16 y, u16 width, u16 height, u16 curval, u16 topval);
void FFT_post(u16 *pbuf);
void mp3_index_show(u16 index, u16 total);
void mp3_vol_show(u8 vol);
void mp3_msg_show(u32 lenth);
	
extern u8 time_k;
extern u8 lcd_bit;

#endif

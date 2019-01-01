#ifndef __MP3PLAYER_H
#define __MP3PLAYER_H
#include "sys.h"
	
#define MP3_TITLE_COLOR   	0XFFFF		//播放器标题颜色	
#define MP3_TITLE_BKCOLOR   0X0000		//播放器标题背景色	

#define MP3_INFO_COLOR   	0X8410		//信息字体的颜色	
#define MP3_MAIN_BKCOLOR    0X18E3		//主背景色	
/////////////////////////////////////////////////////
#define FFT_TOP_COLOR		0XF800		//柱子顶部的颜色
#define FFT_BANDS_COLOR		0X07FF		//柱子的颜色


/////////////////////////////////////////////
#define MP3_LRC_MCOLOR       0XF810//0XFFE0		//当前歌词颜色为黄色
#define MP3_LRC_SCOLOR       0X07FF//0X07FF		//前一句和后一句歌词颜色为青色			    

#define FFT_BANDS			14			//14个频段

//fft变量管理
typedef struct
{
   u8 fft_top[FFT_BANDS];	 //频谱顶值表
   u8 fft_cur[FFT_BANDS];	 //频谱当前值表
   u8 fft_time[FFT_BANDS];	 //顶值停留时间表
} _f_fftdev;

void FFT_post(u16 *pbuf);
void fft_show_oneband(u16 x,u16 y,u16 width,u16 height,u16 curval,u16 topval);
void mp3_play(void);
u16 mp3_get_tnum(u8 *path);
u8 mp3_play_song(u8 *pname, u16 id3head);
void mp3_index_show(u16 index,u16 total);
void mp3_msg_show(u32 lenth);
#endif













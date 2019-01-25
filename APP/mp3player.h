#ifndef __MP3PLAYER_H
#define __MP3PLAYER_H
#include "sys.h"
#include "ff.h"

#define FFT_TOP_COLOR				0XF800		//柱子顶部的颜色
#define FFT_BANDS_COLOR			0X07FF		//柱子的颜色	    

#define FFT_BANDS			14			//14个频段

#define MP3_BUFF_SIZE			512			

//mp3变量管理
typedef struct
{
	u8 *pname;					//带路径的文件名
	u16 totmp3num; 			//音乐文件总数
	u16 curindex;				//音乐当前索引
	FILINFO mp3fileinfo;//文件信息
	u16 *mp3indextbl;		//音乐索引表 
	u16 size;						//MP3ID3大小
	FIL* fmp3;					//MP3文件

	u16 playtime;				//播放时间标记
	u16 time;						//时间变量
	u16 kbps;						//音乐的比特率

	u16 pic_local;			//PIC位置

	u8 *TIT2;						//歌名
	u8 *TPE1;						//作者
	u8 *TALB;           //专辑

	u8 fft_top[FFT_BANDS];	 //频谱顶值表
	u8 fft_cur[FFT_BANDS];	 //频谱当前值表
	u8 fft_time[FFT_BANDS];	 //顶值停留时间表

	u16 FFTbuf[15];    //频谱数组

	u8 pic_show;

}mp3_info;

extern mp3_info info;

void FFT_post(u16 *pbuf);
void fft_show_oneband(u16 x,u16 y,u16 width,u16 height,u16 curval,u16 topval);
void mp3_play(void *pdata);
u16 mp3_get_tnum(u8 *path);
u8 mp3_play_song(void);

#endif













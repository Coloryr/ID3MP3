#ifndef __MP3PLAYER_H__
#define __MP3PLAYER_H__
#include "main.h"
#include "fatfs.h"
#include "stm32f1xx_it.h"

#define MP3_BUFF_SIZE 1024

//mp3变量管理
typedef struct
{
	uint8_t *pname;		   //带路径的文件名
	uint16_t totmp3num;	   //音乐文件总数
	uint16_t curindex;	   //音乐当前索引
	FILINFO mp3fileinfo;   //文件信息
	uint16_t *mp3indextbl; //音乐索引表
	uint32_t size;			   //MP3ID3大小
	FIL *fmp3;			   //MP3文件
	uint8_t *fn;		   //文件名

	uint16_t playtime; //播放时间标记
	uint16_t time;	   //时间变量
	uint16_t kbps;	   //音乐的比特率

	uint32_t pic_local; //PIC位置
	uint32_t pic_size;  //PIC大小

	uint8_t *TIT2; //歌名
	uint8_t *TPE1; //作者
	uint8_t *TALB; //专辑

	uint8_t mode; //当前模式
				  //0正常播放，1音效调整

	uint8_t pic_show; //是否正在显示pic
	uint8_t pic_type; //pic类型
					  //0 JPG，1 PNG，2 没有图片

} mp3_info;

extern mp3_info info;

void FFT_post(uint16_t *pbuf);
void fft_show_oneband(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t curval, uint16_t topval);
void mp3_play(void *pdata);
uint16_t mp3_get_tnum(uint8_t *path);
uint8_t mp3_play_song(void);
void vs_reset(void);
uint8_t button_check(void);

#endif

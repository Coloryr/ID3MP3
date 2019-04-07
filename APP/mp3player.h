#ifndef __MP3PLAYER_H
#define __MP3PLAYER_H
#include "sys.h"
#include "ff.h"

#define MP3_BUFF_SIZE 1024

//mp3变量管理
typedef struct
{
	u8 *pname;			 //带路径的文件名
	u16 totmp3num;		 //音乐文件总数
	u16 curindex;		 //音乐当前索引
	FILINFO mp3fileinfo; //文件信息
	u16 *mp3indextbl;	//音乐索引表
	u32 size;			 //MP3ID3大小
	FIL *fmp3;			 //MP3文件
	u8 *fn;				 //文件名

	u16 playtime; //播放时间标记
	u16 time;	 //时间变量
	u16 kbps;	 //音乐的比特率

	u32 pic_local; //PIC位置
	u32 pic_size;  //PIC大小

	u8 *TIT2; //歌名
	u8 *TPE1; //作者
	u8 *TALB; //专辑

	u8 mode; //当前模式
			 //0正常播放，1音效调整

	u8 pic_show; //是否正在显示pic
	u8 pic_type; //pic类型
				 //0 JPG，1 PNG，2 没有图片

} mp3_info;

extern mp3_info info;

void FFT_post(u16 *pbuf);
void fft_show_oneband(u16 x, u16 y, u16 width, u16 height, u16 curval, u16 topval);
void mp3_play(void *pdata);
u16 mp3_get_tnum(u8 *path);
u8 mp3_play_song(void);

#endif

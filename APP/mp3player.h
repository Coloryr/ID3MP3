#ifndef __MP3PLAYER_H
#define __MP3PLAYER_H
#include "sys.h"
	
#define MP3_TITLE_COLOR   	0XFFFF		//������������ɫ	
#define MP3_TITLE_BKCOLOR   0X0000		//���������ⱳ��ɫ	

#define MP3_INFO_COLOR   	0X8410		//��Ϣ�������ɫ	
#define MP3_MAIN_BKCOLOR    0X18E3		//������ɫ	
/////////////////////////////////////////////////////
#define FFT_TOP_COLOR		0XF800		//���Ӷ�������ɫ
#define FFT_BANDS_COLOR		0X07FF		//���ӵ���ɫ


/////////////////////////////////////////////
#define MP3_LRC_MCOLOR       0XF810//0XFFE0		//��ǰ�����ɫΪ��ɫ
#define MP3_LRC_SCOLOR       0X07FF//0X07FF		//ǰһ��ͺ�һ������ɫΪ��ɫ			    

#define FFT_BANDS			14			//14��Ƶ��

//fft��������
typedef struct
{
   u8 fft_top[FFT_BANDS];	 //Ƶ�׶�ֵ��
   u8 fft_cur[FFT_BANDS];	 //Ƶ�׵�ǰֵ��
   u8 fft_time[FFT_BANDS];	 //��ֵͣ��ʱ���
} _f_fftdev;

void FFT_post(u16 *pbuf);
void fft_show_oneband(u16 x,u16 y,u16 width,u16 height,u16 curval,u16 topval);
void mp3_play(void);
u16 mp3_get_tnum(u8 *path);
u8 mp3_play_song(u8 *pname, u16 id3head);
void mp3_index_show(u16 index,u16 total);
void mp3_msg_show(u32 lenth);
#endif













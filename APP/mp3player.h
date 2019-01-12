#ifndef __MP3PLAYER_H
#define __MP3PLAYER_H
#include "sys.h"
#include "ff.h"

#define FFT_TOP_COLOR				0XF800		//���Ӷ�������ɫ
#define FFT_BANDS_COLOR			0X07FF		//���ӵ���ɫ	    

#define FFT_BANDS			14			//14��Ƶ��

//mp3��������
typedef struct
{
	u8 *pname;					//��·�����ļ���
	u16 totmp3num; 			//�����ļ�����
	u16 curindex;				//���ֵ�ǰ����
	FILINFO mp3fileinfo;//�ļ���Ϣ
	u16 *mp3indextbl;		//���������� 
	u16 size;						//MP3ID3��С
	
	u16 pic_local;			//PICλ��

	u8 *TIT2;						//����
	u8 *TPE1;						//����
	
  u8 fft_top[FFT_BANDS];	 //Ƶ�׶�ֵ��
  u8 fft_cur[FFT_BANDS];	 //Ƶ�׵�ǰֵ��
  u8 fft_time[FFT_BANDS];	 //��ֵͣ��ʱ���
	
	u16 FFTbuf[15];    //Ƶ������
	
}mp3_info;

extern mp3_info info;

void FFT_post(u16 *pbuf);
void fft_show_oneband(u16 x,u16 y,u16 width,u16 height,u16 curval,u16 topval);
void mp3_play(void *pdata);
u16 mp3_get_tnum(u8 *path);
u8 mp3_play_song(void);
void mp3_index_show(u16 index,u16 total);
void mp3_msg_show(u32 lenth);

#endif













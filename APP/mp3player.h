#ifndef __MP3PLAYER_H
#define __MP3PLAYER_H
#include "sys.h"
#include "ff.h"

#define MP3_BUFF_SIZE 1024

//mp3��������
typedef struct
{
	u8 *pname;			 //��·�����ļ���
	u16 totmp3num;		 //�����ļ�����
	u16 curindex;		 //���ֵ�ǰ����
	FILINFO mp3fileinfo; //�ļ���Ϣ
	u16 *mp3indextbl;	//����������
	u32 size;			 //MP3ID3��С
	FIL *fmp3;			 //MP3�ļ�
	u8 *fn;				 //�ļ���

	u16 playtime; //����ʱ����
	u16 time;	 //ʱ�����
	u16 kbps;	 //���ֵı�����

	u32 pic_local; //PICλ��
	u32 pic_size;  //PIC��С

	u8 *TIT2; //����
	u8 *TPE1; //����
	u8 *TALB; //ר��

	u8 mode; //��ǰģʽ
			 //0�������ţ�1��Ч����

	u8 pic_show; //�Ƿ�������ʾpic
	u8 pic_type; //pic����
				 //0 JPG��1 PNG��2 û��ͼƬ

} mp3_info;

extern mp3_info info;

void FFT_post(u16 *pbuf);
void fft_show_oneband(u16 x, u16 y, u16 width, u16 height, u16 curval, u16 topval);
void mp3_play(void *pdata);
u16 mp3_get_tnum(u8 *path);
u8 mp3_play_song(void);

#endif

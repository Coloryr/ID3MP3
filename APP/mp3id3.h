#ifndef __MP3ID3_H
#define __MP3ID3_H		   
#include "sys.h"  
#include "exfuns.h"
#include "ff.h"
 	 	 
#define READ_buff_size   4096			//���ļ��Ļ����С

#define pic_show_x   0						//ͼƬ��ʾ��λ��_x
#define pic_show_y   16						//ͼƬ��ʾ��λ��_y
#define pic_show_size   224				//ͼƬ��ʾ�Ĵ�С

void mp3id3_is(void *pdata);
extern u8 *TIT2;
extern u8 *TPE1;
extern u16 size;
extern u8 song_next;
#endif


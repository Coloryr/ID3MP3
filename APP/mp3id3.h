#ifndef __MP3ID3_H
#define __MP3ID3_H		   
#include "sys.h"  
#include "exfuns.h"
#include "ff.h"
 	 	 
#define READ_buff_size   4096			//���ļ��Ļ����С
#define WRITE_buff_size  1024		  //дͼƬ�Ļ����С�����Դ���1024������⣩
#define out_size         300			//д1024*300�ֽڣ�����256x256�����ͼƬ������Ҫ�Ĵ�һ��

u16 mp3id3_is(const TCHAR* path);

extern u8 type;

#endif


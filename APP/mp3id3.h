#ifndef __MP3ID3_H
#define __MP3ID3_H		   
#include "sys.h"  
#include "exfuns.h"
#include "ff.h"
 	 	 
#define READ_buff_size   4096			//读文件的缓存大小
#define WRITE_buff_size  1024		  //写图片的缓存大小（测试大于1024会出问题）
#define out_size         300			//写1024*300字节，对于256x256更大的图片可能需要改大一点

u16 mp3id3_is(const TCHAR* path);

extern u8 type;

#endif


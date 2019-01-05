#ifndef __MP3ID3_H
#define __MP3ID3_H		   
#include "sys.h"  
#include "exfuns.h"
#include "ff.h"
 	 	 
#define READ_buff_size   4096			//读文件的缓存大小

#define pic_show_x   0						//图片显示的位置_x
#define pic_show_y   16						//图片显示的位置_y
#define pic_show_size   224				//图片显示的大小

u16 mp3id3_is(const TCHAR* path, u8 pic_show);

#endif


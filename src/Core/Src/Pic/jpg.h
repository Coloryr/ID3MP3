#ifndef _JPG
#define _JPG

#include "main.h"

#define JPEG_WBUF_SIZE    4096    //定义工作区数组大小,最少应不小于3092字节.

uint8_t jpg_decode(const uint8_t *filename);

uint8_t jpg_get_size(const uint8_t *filename, uint32_t *width, uint32_t *height);


#endif

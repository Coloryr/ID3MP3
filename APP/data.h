#ifndef __DATA_H
#define __DATA_H
#include "sys.h"

extern u8 save_bit[5];

#define save_bit_local (1024*15)*1024			//Ĭ����15M�ĵ�ַ

void read_data(void);
void write_data(void);

#endif




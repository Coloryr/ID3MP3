#ifndef __DATA_H
#define __DATA_H
#include "sys.h"

extern u8 data_save_bit;

void read_data(void);
void write_data(void);
void TP_Save_Adjdata(void); //保存校准参数
u8 TP_Get_Adjdata(void);    //读取校准参数

#endif

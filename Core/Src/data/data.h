#ifndef __DATA_H
#define __DATA_H
#include "main.h"

extern uint8_t data_save_bit;

void read_data(void);
void write_data(void);
void TP_Save_Adjdata(void);   
uint8_t TP_Get_Adjdata(void); 
#endif

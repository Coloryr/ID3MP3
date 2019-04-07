#ifndef __APP_START_H
#define __APP_START_H
#include "sys.h"

void APP_start(void);
void APP_pic_start(void);
u32 app_get_rand(u32 max);
void Adc_Init(void);
u16 Get_Adc_Average(void);

#endif

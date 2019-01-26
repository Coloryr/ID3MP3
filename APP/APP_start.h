#ifndef __APP_START_H
#define __APP_START_H
#include "sys.h"

extern u8 key_now;

void APP_start(void);
void APP_pic_start(void);
void button_check(void);
u32 app_get_rand(u32 max);

#endif













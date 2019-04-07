#ifndef __KEY_H
#define __KEY_H
#include "sys.h"

#define KEY0_PRES 1 //KEY0����
#define KEY1_PRES 2 //KEY1����
#define KEY2_PRES 3 //KEY2����
#define KEY3_PRES 4 //KEY3����

#define KEY0 PCin(0) //PC5
#define KEY1 PCin(1) //PA15
#define KEY2 PCin(2) //PA15
#define KEY3 PCin(3) //PA0  WK_UP

u8 KEY_Scan(u8 mode); //����ɨ�躯��
u8 KEY0_Scan(void);   //����ɨ��KEY0����

#endif

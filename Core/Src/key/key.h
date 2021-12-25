#ifndef __KEY_H__
#define __KEY_H__
#include "main.h"
#include "../bit.h"

#define KEY0_PRES 1 //KEY0按下
#define KEY1_PRES 2 //KEY1按下
#define KEY2_PRES 3 //KEY2按下
#define KEY3_PRES 4 //KEY3按下

#define KEY0 PCin(0)
#define KEY1 PCin(1)
#define KEY2 PCin(2)
#define KEY3 PCin(3)

uint8_t KEY_Scan(uint8_t mode); //按键扫描函数

#endif

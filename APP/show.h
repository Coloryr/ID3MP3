#ifndef __SHOW_H_
#define __SHOW_H_
#include "sys.h"
#include "stdlib.h"

void time_go(void);
void game(void);

#define Sound2 2500          //??2
#define Sound1 1500 
#define Sound3 2000 
 
#define key1  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11)//��ȡ����0
#define key3  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_12)//��ȡ����1
#define key2  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_10)//��ȡ����1

extern u8 time_k;

#endif

#include "includes.h" 

u8 key_now;

//按键初始化函数 
//PA0.15和PC5 设置成输入
void KEY_Init(void)
{

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);//使能PORTA,PORTC时钟

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//关闭jtag，使能SWD，可以用SWD模式调试

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_2 | GPIO_Pin_1 | GPIO_Pin_0;//PA15
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
	GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIOA15
}
//按键处理函数
//返回按键值
//mode:0,不支持连续按;1,支持连续按;
//返回值：
//0，没有任何按键按下
//KEY0_PRES，KEY0按下
//KEY1_PRES，KEY1按下
//WKUP_PRES，WK_UP按下 
//注意此函数有响应优先级,KEY0>KEY1>WK_UP!!
u8 KEY_Scan(u8 mode)
{
	static u8 key_up = 1;//按键按松开标志
	if (mode)key_up = 1;  //支持连按		  
	if (key_up && (KEY0 == 0 || KEY1 == 0 || KEY2 == 0 || KEY3 == 0))
	{
		delay_ms(10);
		key_up = 0;
		if (KEY0 == 0)return KEY0_PRES;
		else if (KEY1 == 0)return KEY1_PRES;
		else if (KEY2 == 0)return KEY2_PRES;
		else if (KEY3 == 0)return KEY3_PRES;
	}
	else if (KEY0 == 1 && KEY1 == 1 && KEY2 == 1 && KEY3 == 1)key_up = 1;
	return 0;// 无按键按下
}
//单独扫描KEY0是否被按下
//返回值:0,没按下;1,按下了.
u8 KEY0_Scan(void)
{
	static u8 key_up = 1;//按键按松开标志 	  
	if (key_up&&KEY0 == 0)
	{
		delay_ms(10);//去抖动 
		key_up = 0;
		if (KEY0 == 0)return 1;
	}
	else if (KEY0 == 1)key_up = 1;
	return 0;// 无按键按下
}

void KEY_task(void *pdata)
{
	OS_ERR err;
	while (1)
	{
		key_now = KEY_Scan(0);
		OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_PERIODIC, &err);//延时10ms
	}
}




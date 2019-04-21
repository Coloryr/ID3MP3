#include "sys.h"
#include "lcd.h"
#include "sdio_sdcard.h"
#include "ff.h"
#include "flash.h"
#include "diskio.h"
#include "malloc.h"
#include "exfuns.h"
#include "app_start.h"
#include "includes.h"
#include "touch.h"
#include "delay.h"
#include "init.h"
#include "spi.h"
#include "key.h"
#include "fontupd.h"
#include "text.h"
#include "guix.h"
#include "piclib.h"
#include "show.h"

int main(void)
{
	delay_init(72);									//延时函数初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //中断分组配置
	init();											//GPIO等初始化
	PWM_Init(899, 0);								//不分频。PWM频率=72000/(899+1)=80Khz
	TIM_SetCompare3(TIM3, 850);
	LCD_Init(); //LCD初始化
	mem_init(); //初始化内存池
	Adc_Init();
	piclib_init();
	gui_init();
	SPI1_SetSpeed(SPI_BaudRatePrescaler_2); //设置为18M时钟,高速模式
	//TP_Init();								//触摸屏初始化
	POINT_COLOR = RED;
	BACK_COLOR = BLACK;
	LCD_Clear(BLACK); //清屏

	while (exfuns_init()) //为fatfs相关变量申请内存
	{
		LCD_ShowString(30, 20, 320, 16, 16, "Fatfs -> ERROR!");
	}
	while (SD_Init()) //SD卡初始化
	{
		LCD_ShowString(30, 20, 320, 16, 16, "TFcard -> ERROR!");
	}

	f_mount(fs[0], "0:", 1); //挂载SD卡
	f_mount(fs[1], "1:", 1); //挂载FLASH.

	if (font_init() == 1 || KEY_Scan(0) == 1)
	{
	a:
		update_font(30, 20, 16, 0);
		if (font_init() != 0)
		{
			goto a;
		}
		LCD_Clear(WHITE); //清屏
		TP_Adjust();	  //屏幕校准
		POINT_COLOR = RED;
		BACK_COLOR = BLACK;
	}
	if (KEY_Scan(0) == 2)
	{
		LCD_Clear(WHITE); //清屏
		TP_Adjust();	  //屏幕校准
		POINT_COLOR = RED;
		BACK_COLOR = BLACK;
		LCD_Clear(WHITE); //清屏
	}
	APP_start();
}

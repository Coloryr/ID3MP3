#include "sys.h" 
#include "lcd.h"
#include "sdio_sdcard.h" 
#include "ff.h" 
#include "flash.h"
#include "diskio.h"
#include "malloc.h"
#include "exfuns.h"
#include "rtc.h"
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

int main(void)
{
	delay_init(72);	    			//延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//中断分组配置
	init();										//GPIO等初始化
	LCD_Init();								//LCD初始化	
	POINT_COLOR = RED;
	BACK_COLOR = BLACK;
	mem_init();								//初始化内存池	 
	piclib_init();
	gui_init();
	SPI1_SetSpeed(SPI_BaudRatePrescaler_2);//设置为18M时钟,高速模式		
	TP_Init();								//触摸屏初始化
	LCD_Clear(BLACK);//清屏  
	while (exfuns_init())			//为fatfs相关变量申请内存 					
	{
		LCD_ShowString(30, 40, 320, 16, 16,"Fatfs -> ERROR!");
		delay_ms(200);
		LCD_Fill(30, 40, 320, 226, BLACK);//清除显示	     
		delay_ms(200);
	}
	while (SD_Init())					//SD卡初始化				
	{
		LCD_ShowString(30, 40, 320, 16, 16,"TFcard -> ERROR!");
		delay_ms(200);
		LCD_Fill(30, 40, 320, 226, BLACK);//清除显示	     
		delay_ms(200);
	}

	while (RTC_Init())					//RTC初始化			
	{
		LCD_ShowString(30, 40, 320, 16, 16,"RTC -> ERROR!   ");
		delay_ms(200);
		LCD_Fill(30, 40, 320, 226, BLACK);//清除显示	     
		delay_ms(200);
	}

	f_mount(fs[0], "0:", 1); 	//挂载SD卡 
	f_mount(fs[1], "1:", 1); 	//挂载FLASH.

	if (font_init() == 1 || KEY_Scan(0) == 1)
	{
	a:
		update_font(30, 100, 16, 0);
		if (font_init() != 0)
		{
			goto a;
		}
	}
	APP_start();
}














#include "sys.h"	
#include "delay.h"	
#include "key.h" 
#include "malloc.h"  
#include "sdio_sdcard.h"
#include "ff.h"  
#include "exfuns.h"
#include "fontupd.h"
#include "text.h"	
#include "vs10XX.h"	
#include "mp3player.h"
#include "lcd.h"
#include "rtc.h"
#include "guix.h"
#include "spi.h"
#include "mp3id3.h" 
#include "piclib.h"	
#include "includes.h" 
#include "app_start.h" 
#include "data.h" 

int main(void)
{
	Stm32_Clock_Init(16);
	delay_init(72);	    			 	//延时函数初始化	  
	KEY_Init();								//按键初始化
	LCD_Init();								//LCD初始化
	mem_init();								//初始化内存池	    
	exfuns_init();						//为fatfs相关变量申请内存 
	SD_Init();								//SD卡初始化
	gui_init();								//LCD-gui初始化
	RTC_Init();								//RTC初始化
	piclib_init();						//PIC初始化
	f_mount(fs[0], "0:", 1); 	//挂载SD卡 
	f_mount(fs[1], "1:", 1); 	//挂载FLASH.
	SPI1_Init();			 				//初始化SPI1总线
	SPI1_SetSpeed(SPI_BaudRatePrescaler_2);//设置为18M时钟,高速模式		
	VS1053_Init();						//VS1053初始化
	LCD_Clear(BLACK);
	read_data();
	POINT_COLOR = RED;
	BACK_COLOR = BLACK;
	if (font_init() == 1 || KEY_Scan(0) ==1)
	{
		a:
		update_font(0, 0, 16, 0);
		if(font_init() != 0)
		{
			goto a;
		}
	}
	POINT_COLOR = RED;
	LCD_Clear(BLACK);
	APP_start();
}














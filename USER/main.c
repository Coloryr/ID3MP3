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
 							   	 
int main(void)
{
		u32 fontcnt;		  
	u8 i,j;
	u8 fontx[2];//gbk码
	u8 key,t;	
	u16 ret;
	delay_init();	    	 //延时函数初始化	  
	NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	KEY_Init();
	LCD_Init();
	mem_init();				//初始化内存池	    
	exfuns_init();			//为fatfs相关变量申请内存 
	SD_Init();
	gui_init();
	//RTC_Init();
	f_mount(fs[0], "0:", 1); 	//挂载SD卡 
	f_mount(fs[1], "1:", 1); 	//挂载FLASH.
	SPI1_Init();			 //初始化SPI1总线
	SPI1_SetSpeed(SPI_BaudRatePrescaler_2);//设置为18M时钟,高速模式		
	VS1053_Init();
	LCD_Clear(BLACK);
	POINT_COLOR=RED;
	BACK_COLOR=BLACK;
	LCD_ShowString(0,0,320,240,12,"TEST");		
	if(font_init()==1)
	  update_font(0,0,16,0);
	while (1)
	{
			POINT_COLOR=RED;       			
  delay_ms(2000);	
		LCD_Clear(BLACK);
		VS_Sine_Test();
		mp3_play();
	}
}














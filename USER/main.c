#include "includes.h" 

int main(void)
{
	delay_init(72);	    			//延时函数初始化	  
	KEY_Init();								//按键初始化
	LCD_Init();								//LCD初始化	
	POINT_COLOR = RED;
	BACK_COLOR = BLACK;	
	Show_Str(30, 20, 320, 16, "LCD -> OK!", 16, 0);
	mem_init();								//初始化内存池	 
	piclib_init();						//PIC初始化
	gui_init();								//LCD-gui初始化
	SPI1_Init();			 				//初始化SPI1总线
	SPI1_SetSpeed(SPI_BaudRatePrescaler_2);//设置为18M时钟,高速模式		
	VS1053_Init();						//VS1053初始化
	
	while(exfuns_init())			//为fatfs相关变量申请内存 					
	{
		Show_Str(30, 40, 320, 16, "fatfs -> ERROR!", 16, 0);
		delay_ms(200);
		LCD_Fill(30, 40, 320, 226, BLACK);//清除显示	     
		delay_ms(200);
	}
	Show_Str(30, 40, 320, 16, "fatfs -> OK!", 16, 0);		
	
	while(SD_Init())					//SD卡初始化				
	{
		Show_Str(30, 60, 320, 16, "TFcard -> ERROR!", 16, 0);
		delay_ms(200);
		LCD_Fill(30, 60, 320, 226, BLACK);//清除显示	     
		delay_ms(200);
	}
	Show_Str(30, 60, 320, 16, "TFcard -> OK!", 16, 0);	
	
	while(RTC_Init())					//RTC初始化			
	{
		Show_Str(30, 80, 320, 16, "RTC -> ERROR!", 16, 0);
		delay_ms(200);
		LCD_Fill(30, 80, 320, 226, BLACK);//清除显示	     
		delay_ms(200);
	}
	Show_Str(30, 80, 320, 16, "RTC -> OK!", 16, 0);	
	
	f_mount(fs[0], "0:", 1); 	//挂载SD卡 
	f_mount(fs[1], "1:", 1); 	//挂载FLASH.
	
	if (font_init() == 1 || KEY_Scan(0) ==1)
	{
		a:
		update_font(30, 100, 16, 0);
		if(font_init() != 0)
		{
			goto a;
		}
	}
	show_mode=1;
	Show_Str(30, 100, 320, 16, "中文字库 -> OK!", 16, 0);	
	APP_start();
}














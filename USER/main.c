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
	delay_init(72);	    			//��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�жϷ�������
	init();										//GPIO�ȳ�ʼ��
	LCD_Init();								//LCD��ʼ��	
	POINT_COLOR = RED;
	BACK_COLOR = BLACK;
	mem_init();								//��ʼ���ڴ��	 
	piclib_init();
	gui_init();
	SPI1_SetSpeed(SPI_BaudRatePrescaler_2);//����Ϊ18Mʱ��,����ģʽ		
	TP_Init();								//��������ʼ��
	LCD_Clear(BLACK);//����  
	while (exfuns_init())			//Ϊfatfs��ر��������ڴ� 					
	{
		LCD_ShowString(30, 40, 320, 16, 16,"Fatfs -> ERROR!");
		delay_ms(200);
		LCD_Fill(30, 40, 320, 226, BLACK);//�����ʾ	     
		delay_ms(200);
	}
	while (SD_Init())					//SD����ʼ��				
	{
		LCD_ShowString(30, 40, 320, 16, 16,"TFcard -> ERROR!");
		delay_ms(200);
		LCD_Fill(30, 40, 320, 226, BLACK);//�����ʾ	     
		delay_ms(200);
	}

	while (RTC_Init())					//RTC��ʼ��			
	{
		LCD_ShowString(30, 40, 320, 16, 16,"RTC -> ERROR!   ");
		delay_ms(200);
		LCD_Fill(30, 40, 320, 226, BLACK);//�����ʾ	     
		delay_ms(200);
	}

	f_mount(fs[0], "0:", 1); 	//����SD�� 
	f_mount(fs[1], "1:", 1); 	//����FLASH.

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














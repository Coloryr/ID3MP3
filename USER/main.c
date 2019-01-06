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

int main(void)
{
	delay_init();	    			 	//��ʱ������ʼ��	  
	NVIC_Configuration(); 		//����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	KEY_Init();								//������ʼ��
	LCD_Init();								//LCD��ʼ��
	mem_init();								//��ʼ���ڴ��	    
	exfuns_init();						//Ϊfatfs��ر��������ڴ� 
	SD_Init();								//SD����ʼ��
	gui_init();								//LCD-gui��ʼ��
	//RTC_Init();								//RTC��ʼ��
	piclib_init();						//PIC��ʼ��
	f_mount(fs[0], "0:", 1); 	//����SD�� 
	f_mount(fs[1], "1:", 1); 	//����FLASH.
	SPI1_Init();			 				//��ʼ��SPI1����
	SPI1_SetSpeed(SPI_BaudRatePrescaler_2);//����Ϊ18Mʱ��,����ģʽ		
	VS1053_Init();						//VS1053��ʼ��
	LCD_Clear(BLACK);
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
	while (1)
	{
		POINT_COLOR = RED;
		LCD_Clear(BLACK);
		VS_Sine_Test();
				
		mp3_play();
	}
}














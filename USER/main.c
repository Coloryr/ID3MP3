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
	u8 fontx[2];//gbk��
	u8 key,t;	
	u16 ret;
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_Configuration(); 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	KEY_Init();
	LCD_Init();
	mem_init();				//��ʼ���ڴ��	    
	exfuns_init();			//Ϊfatfs��ر��������ڴ� 
	SD_Init();
	gui_init();
	//RTC_Init();
	f_mount(fs[0], "0:", 1); 	//����SD�� 
	f_mount(fs[1], "1:", 1); 	//����FLASH.
	SPI1_Init();			 //��ʼ��SPI1����
	SPI1_SetSpeed(SPI_BaudRatePrescaler_2);//����Ϊ18Mʱ��,����ģʽ		
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














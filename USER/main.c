#include "includes.h" 

int main(void)
{
	delay_init(72);	    			//��ʱ������ʼ��	  
	KEY_Init();								//������ʼ��
	LCD_Init();								//LCD��ʼ��
	mem_init();								//��ʼ���ڴ��	    
	exfuns_init();						//Ϊfatfs��ر��������ڴ� 
	SD_Init();								//SD����ʼ��
	gui_init();								//LCD-gui��ʼ��
	RTC_Init();								//RTC��ʼ��
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
	POINT_COLOR = RED;
	LCD_Clear(BLACK);
	APP_start();
}














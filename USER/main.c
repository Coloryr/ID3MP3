#include "includes.h" 

int main(void)
{
	delay_init(72);	    			//��ʱ������ʼ��	  
	KEY_Init();								//������ʼ��
	LCD_Init();								//LCD��ʼ��	
	POINT_COLOR = RED;
	BACK_COLOR = BLACK;	
	Show_Str(30, 20, 320, 16, "LCD -> OK!", 16, 0);
	mem_init();								//��ʼ���ڴ��	 
	piclib_init();						//PIC��ʼ��
	gui_init();								//LCD-gui��ʼ��
	SPI1_Init();			 				//��ʼ��SPI1����
	SPI1_SetSpeed(SPI_BaudRatePrescaler_2);//����Ϊ18Mʱ��,����ģʽ		
	VS1053_Init();						//VS1053��ʼ��
	
	while(exfuns_init())			//Ϊfatfs��ر��������ڴ� 					
	{
		Show_Str(30, 40, 320, 16, "fatfs -> ERROR!", 16, 0);
		delay_ms(200);
		LCD_Fill(30, 40, 320, 226, BLACK);//�����ʾ	     
		delay_ms(200);
	}
	Show_Str(30, 40, 320, 16, "fatfs -> OK!", 16, 0);		
	
	while(SD_Init())					//SD����ʼ��				
	{
		Show_Str(30, 60, 320, 16, "TFcard -> ERROR!", 16, 0);
		delay_ms(200);
		LCD_Fill(30, 60, 320, 226, BLACK);//�����ʾ	     
		delay_ms(200);
	}
	Show_Str(30, 60, 320, 16, "TFcard -> OK!", 16, 0);	
	
	while(RTC_Init())					//RTC��ʼ��			
	{
		Show_Str(30, 80, 320, 16, "RTC -> ERROR!", 16, 0);
		delay_ms(200);
		LCD_Fill(30, 80, 320, 226, BLACK);//�����ʾ	     
		delay_ms(200);
	}
	Show_Str(30, 80, 320, 16, "RTC -> OK!", 16, 0);	
	
	f_mount(fs[0], "0:", 1); 	//����SD�� 
	f_mount(fs[1], "1:", 1); 	//����FLASH.
	
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
	Show_Str(30, 100, 320, 16, "�����ֿ� -> OK!", 16, 0);	
	APP_start();
}














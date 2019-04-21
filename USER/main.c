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
	delay_init(72);									//��ʱ������ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //�жϷ�������
	init();											//GPIO�ȳ�ʼ��
	PWM_Init(899, 0);								//����Ƶ��PWMƵ��=72000/(899+1)=80Khz
	TIM_SetCompare3(TIM3, 850);
	LCD_Init(); //LCD��ʼ��
	mem_init(); //��ʼ���ڴ��
	Adc_Init();
	piclib_init();
	gui_init();
	SPI1_SetSpeed(SPI_BaudRatePrescaler_2); //����Ϊ18Mʱ��,����ģʽ
	//TP_Init();								//��������ʼ��
	POINT_COLOR = RED;
	BACK_COLOR = BLACK;
	LCD_Clear(BLACK); //����

	while (exfuns_init()) //Ϊfatfs��ر��������ڴ�
	{
		LCD_ShowString(30, 20, 320, 16, 16, "Fatfs -> ERROR!");
	}
	while (SD_Init()) //SD����ʼ��
	{
		LCD_ShowString(30, 20, 320, 16, 16, "TFcard -> ERROR!");
	}

	f_mount(fs[0], "0:", 1); //����SD��
	f_mount(fs[1], "1:", 1); //����FLASH.

	if (font_init() == 1 || KEY_Scan(0) == 1)
	{
	a:
		update_font(30, 20, 16, 0);
		if (font_init() != 0)
		{
			goto a;
		}
		LCD_Clear(WHITE); //����
		TP_Adjust();	  //��ĻУ׼
		POINT_COLOR = RED;
		BACK_COLOR = BLACK;
	}
	if (KEY_Scan(0) == 2)
	{
		LCD_Clear(WHITE); //����
		TP_Adjust();	  //��ĻУ׼
		POINT_COLOR = RED;
		BACK_COLOR = BLACK;
		LCD_Clear(WHITE); //����
	}
	APP_start();
}

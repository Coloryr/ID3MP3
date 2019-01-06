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

/////////////////////////UCOSII��������///////////////////////////////////
//START ����
//�����������ȼ�
#define START_TASK_PRIO      			10 //��ʼ��������ȼ�����Ϊ���
//���������ջ��С
#define START_STK_SIZE  				  64
//�����ջ��8�ֽڶ���	
OS_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata);	
 			   
//MP3����
//�����������ȼ�
#define MUSIC_PLAY_TASK_PRIO       		2 
//���������ջ��С
#define MUSIC_PLAY_STK_SIZE  		    256
//�����ջ��8�ֽڶ���	
OS_STK MUSIC_PLAY_TASK_STK[MUSIC_PLAY_STK_SIZE];
//������
void mp3_play(void *pdata);

//ͼƬ��ʾ����
//�����������ȼ�
#define PIC_SHOW_TASK_PRIO       		3
//���������ջ��С
#define PIC_SHOW_STK_SIZE  		    1024
//�����ջ��8�ֽڶ���	
__align(8) static OS_STK PIC_SHOW_TASK_STK[PIC_SHOW_STK_SIZE];
//������
void mp3id3_is(void *pdata);

////////////////////////////////α����������취////////////////////////////////
u32 random_seed=1;
void app_srand(u32 seed)
{
	random_seed=seed;
}
//��ȡα�����
//���Բ���0~RANDOM_MAX-1�������
//seed:����
//max:���ֵ	  		  
//����ֵ:0~(max-1)�е�һ��ֵ 		
u32 app_get_rand(u32 max)
{			    	    
	random_seed=random_seed*22695477+1;
	return (random_seed)%max; 
}  

//��ʼ����
void start_task(void *pdata)
{
    OS_CPU_SR cpu_sr=0;
	pdata = pdata; 
	OS_ENTER_CRITICAL();//�����ٽ���(�޷����жϴ��)    	   
	OSStatInit();		//��ʼ��ͳ������.�������ʱ1��������	
 	app_srand(OSTime);
  OSTaskCreate(mp3_play,(void *)0,(OS_STK*)&MUSIC_PLAY_TASK_STK[MUSIC_PLAY_STK_SIZE-1],MUSIC_PLAY_TASK_PRIO); 		
	OSTaskCreate(mp3id3_is,(void *)0,(OS_STK*)&PIC_SHOW_TASK_STK[PIC_SHOW_STK_SIZE-1],PIC_SHOW_TASK_PRIO); 				   
	OSTaskSuspend(START_TASK_PRIO);	//������ʼ����.
	OS_EXIT_CRITICAL();	//�˳��ٽ���(���Ա��жϴ��)
}

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
	POINT_COLOR = RED;
	LCD_Clear(BLACK);
	VS_Sine_Test();
	OSInit();   
	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//������ʼ����
	OSStart();
}














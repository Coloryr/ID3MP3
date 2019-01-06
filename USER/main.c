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

/////////////////////////UCOSII任务设置///////////////////////////////////
//START 任务
//设置任务优先级
#define START_TASK_PRIO      			10 //开始任务的优先级设置为最低
//设置任务堆栈大小
#define START_STK_SIZE  				  64
//任务堆栈，8字节对齐	
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);	
 			   
//MP3任务
//设置任务优先级
#define MUSIC_PLAY_TASK_PRIO       		2 
//设置任务堆栈大小
#define MUSIC_PLAY_STK_SIZE  		    256
//任务堆栈，8字节对齐	
OS_STK MUSIC_PLAY_TASK_STK[MUSIC_PLAY_STK_SIZE];
//任务函数
void mp3_play(void *pdata);

//图片显示任务
//设置任务优先级
#define PIC_SHOW_TASK_PRIO       		3
//设置任务堆栈大小
#define PIC_SHOW_STK_SIZE  		    1024
//任务堆栈，8字节对齐	
__align(8) static OS_STK PIC_SHOW_TASK_STK[PIC_SHOW_STK_SIZE];
//任务函数
void mp3id3_is(void *pdata);

////////////////////////////////伪随机数产生办法////////////////////////////////
u32 random_seed=1;
void app_srand(u32 seed)
{
	random_seed=seed;
}
//获取伪随机数
//可以产生0~RANDOM_MAX-1的随机数
//seed:种子
//max:最大值	  		  
//返回值:0~(max-1)中的一个值 		
u32 app_get_rand(u32 max)
{			    	    
	random_seed=random_seed*22695477+1;
	return (random_seed)%max; 
}  

//开始任务
void start_task(void *pdata)
{
    OS_CPU_SR cpu_sr=0;
	pdata = pdata; 
	OS_ENTER_CRITICAL();//进入临界区(无法被中断打断)    	   
	OSStatInit();		//初始化统计任务.这里会延时1秒钟左右	
 	app_srand(OSTime);
  OSTaskCreate(mp3_play,(void *)0,(OS_STK*)&MUSIC_PLAY_TASK_STK[MUSIC_PLAY_STK_SIZE-1],MUSIC_PLAY_TASK_PRIO); 		
	OSTaskCreate(mp3id3_is,(void *)0,(OS_STK*)&PIC_SHOW_TASK_STK[PIC_SHOW_STK_SIZE-1],PIC_SHOW_TASK_PRIO); 				   
	OSTaskSuspend(START_TASK_PRIO);	//挂起起始任务.
	OS_EXIT_CRITICAL();	//退出临界区(可以被中断打断)
}

int main(void)
{
	delay_init();	    			 	//延时函数初始化	  
	NVIC_Configuration(); 		//设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	KEY_Init();								//按键初始化
	LCD_Init();								//LCD初始化
	mem_init();								//初始化内存池	    
	exfuns_init();						//为fatfs相关变量申请内存 
	SD_Init();								//SD卡初始化
	gui_init();								//LCD-gui初始化
	//RTC_Init();								//RTC初始化
	piclib_init();						//PIC初始化
	f_mount(fs[0], "0:", 1); 	//挂载SD卡 
	f_mount(fs[1], "1:", 1); 	//挂载FLASH.
	SPI1_Init();			 				//初始化SPI1总线
	SPI1_SetSpeed(SPI_BaudRatePrescaler_2);//设置为18M时钟,高速模式		
	VS1053_Init();						//VS1053初始化
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
	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	OSStart();
}














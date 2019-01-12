#include "sys.h"
#include "APP_start.h"
#include "includes.h" 
#include "show.h" 
#include "mp3player.h"

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
#define MUSIC_PLAY_TASK_PRIO       		1 
//设置任务堆栈大小
#define MUSIC_PLAY_STK_SIZE  		    256
//任务堆栈，8字节对齐	
__align(8) OS_STK MUSIC_PLAY_TASK_STK[MUSIC_PLAY_STK_SIZE];
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
void show_mp3_pic(void *pdata);

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
  OSTaskCreate(mp3_play,(void *)0,(OS_STK *)&MUSIC_PLAY_TASK_STK[MUSIC_PLAY_STK_SIZE-1],MUSIC_PLAY_TASK_PRIO);
	OSTaskSuspend(START_TASK_PRIO);	//挂起起始任务.
	OS_EXIT_CRITICAL();	//退出临界区(可以被中断打断)
}

void APP_start(void)
{
	OSInit();   
	//创建起始任务
	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );
	OSStart();
}

void APP_pic_start(void)
{
		OS_CPU_SR cpu_sr=0;
	OS_ENTER_CRITICAL();//进入临界区(无法被中断打断)    
	OSTaskCreate(show_mp3_pic,(void *)0,(OS_STK *)&PIC_SHOW_TASK_STK[PIC_SHOW_STK_SIZE-1],PIC_SHOW_TASK_PRIO);
	OS_EXIT_CRITICAL();	//退出临界区(可以被中断打断)
}





#include "includes.h" 

/////////////////////////UCOSII任务设置///////////////////////////////////
//START 任务
//任务优先级
#define START_TASK_PRIO		3
//任务堆栈大小	
#define START_STK_SIZE 		128
//任务控制块
OS_TCB StartTaskTCB;
//任务堆栈	
CPU_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);	
 			   
//MP3任务
//设置任务优先级
#define MUSIC_PLAY_TASK_PRIO       	2 
//设置任务堆栈大小
#define MUSIC_PLAY_STK_SIZE  		    256
//任务控制块
OS_TCB MUSICTaskTCB;
//任务堆栈	
CPU_STK MUSIC_PLAY_TASK_STK[MUSIC_PLAY_STK_SIZE];
//任务函数
void mp3_play(void *pdata);

//图片显示任务
//设置任务优先级
#define PIC_SHOW_TASK_PRIO       		2
//设置任务堆栈大小
#define PIC_SHOW_STK_SIZE  		    2096
//任务控制块
OS_TCB PICTaskTCB;
//任务堆栈	
CPU_STK PIC_SHOW_TASK_STK[PIC_SHOW_STK_SIZE];
//任务函数
void show_mp3_pic(void *pdata);

//显示任务
//设置任务优先级
#define SHOW_TASK_PRIO       			2
//设置任务堆栈大小
#define SHOW_STK_SIZE  		    		64
//任务控制块
OS_TCB SHOWTaskTCB;
//任务堆栈	
CPU_STK SHOW_TASK_STK[SHOW_STK_SIZE];
//任务函数
//void show_all(void *pdata);

////////////////////////////////伪随机数产生办法////////////////////////////////
u32 random_seed=1;
void app_srand(u32 seed)
{
	random_seed = seed;
}
//获取伪随机数
//可以产生0~RANDOM_MAX-1的随机数
//seed:种子
//max:最大值	  		  
//返回值:0~(max-1)中的一个值 		
u32 app_get_rand(u32 max)
{
	random_seed = random_seed * 22695477 + 1;
	return (random_seed) % max;
}

//开始任务
void start_task(void *pdata)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	pdata = pdata;

	CPU_Init();
	//使能时间片轮转调度功能,时间片长度为1个系统时钟节拍，既1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED, 1, &err);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);//开启CRC时钟		

	OS_CRITICAL_ENTER();	//进入临界区
	//创建MP3任务
	OSTaskCreate((OS_TCB 	*)&MUSICTaskTCB,
		(CPU_CHAR	*)"mp3 task",
		(OS_TASK_PTR)mp3_play,
		(void		*)0,
		(OS_PRIO)MUSIC_PLAY_TASK_PRIO,
		(CPU_STK   *)&MUSIC_PLAY_TASK_STK[0],
		(CPU_STK_SIZE)MUSIC_PLAY_STK_SIZE / 10,
		(CPU_STK_SIZE)MUSIC_PLAY_STK_SIZE,
		(OS_MSG_QTY)0,
		(OS_TICK)0,
		(void   	*)0,
		(OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
		(OS_ERR 	*)&err);
	OSTaskCreate((OS_TCB 	*)&PICTaskTCB,
		(CPU_CHAR	*)"pic_show task",
		(OS_TASK_PTR)show_mp3_pic,
		(void		*)0,
		(OS_PRIO)PIC_SHOW_TASK_PRIO,
		(CPU_STK   *)&PIC_SHOW_TASK_STK[0],
		(CPU_STK_SIZE)PIC_SHOW_STK_SIZE / 10,
		(CPU_STK_SIZE)PIC_SHOW_STK_SIZE,
		(OS_MSG_QTY)0,
		(OS_TICK)0,
		(void   	*)0,
		(OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
		(OS_ERR 	*)&err);
	/*
	OSTaskCreate((OS_TCB 	*)&SHOWTaskTCB,
		(CPU_CHAR	*)"show task",
		(OS_TASK_PTR)show_all,
		(void		*)0,
		(OS_PRIO)SHOW_TASK_PRIO,
		(CPU_STK   *)&SHOW_TASK_STK[0],
		(CPU_STK_SIZE)SHOW_STK_SIZE / 10,
		(CPU_STK_SIZE)SHOW_STK_SIZE,
		(OS_MSG_QTY)0,
		(OS_TICK)0,
		(void   	*)0,
		(OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
		(OS_ERR 	*)&err);
		*/
	OS_TaskSuspend((OS_TCB*)&StartTaskTCB, &err);		//挂起开始任务			 
	OS_CRITICAL_EXIT();	//进入临界区
}

void APP_start(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	OSInit(&err);		//初始化UCOSIII
	OS_CRITICAL_ENTER();//进入临界区
	//创建开始任务
	OSTaskCreate((OS_TCB 	*)&StartTaskTCB,		//任务控制块
		(CPU_CHAR	*)"start task", 		//任务名字
		(OS_TASK_PTR)start_task, 			//任务函数
		(void		*)0,					//传递给任务函数的参数
		(OS_PRIO)PIC_SHOW_TASK_PRIO,     //任务优先级
		(CPU_STK   *)&START_TASK_STK[0],	//任务堆栈基地址
		(CPU_STK_SIZE)START_STK_SIZE / 10,	//任务堆栈深度限位
		(CPU_STK_SIZE)START_STK_SIZE,		//任务堆栈大小
		(OS_MSG_QTY)0,					//任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
		(OS_TICK)0,					//当使能时间片轮转时的时间片长度，为0时为默认长度，
		(void   	*)0,					//用户补充的存储区
		(OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR, //任务选项
		(OS_ERR 	*)&err);				//存放该函数错误时的返回值
	OS_CRITICAL_EXIT();	//退出临界区	 
	OSStart(&err);  //开启UCOSIII
	while (1);
}





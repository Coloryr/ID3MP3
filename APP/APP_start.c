#include "app_start.h" 
#include "mp3player.h"
#include "includes.h"
#include "show.h"
#include "GUIx.h"
#include "key.h"
#include "lcd.h"
#include "fontupd.h"
#include "data.h"
#include "WM.h"
#include "DIALOG.h"

/////////////////////////UCOSII任务设置///////////////////////////////////
//START 任务
//任务优先级
#define START_TASK_PRIO		3
//任务堆栈大小	
#define START_STK_SIZE 		64
//任务控制块
OS_TCB StartTaskTCB;
//任务堆栈	
CPU_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);	
 			   
//MP3任务
//设置任务优先级
#define MUSIC_PLAY_TASK_PRIO       	1
//设置任务堆栈大小
#define MUSIC_PLAY_STK_SIZE  		    128
//任务控制块
OS_TCB MUSICTaskTCB;
//任务堆栈	
CPU_STK MUSIC_PLAY_TASK_STK[MUSIC_PLAY_STK_SIZE];
//任务函数
void mp3_play(void *pdata);

//图片显示任务
//设置任务优先级
#define PIC_SHOW_TASK_PRIO       		1
//设置任务堆栈大小
#define PIC_SHOW_STK_SIZE  		      128
//任务控制块
OS_TCB PICTaskTCB;
//任务堆栈	
CPU_STK PIC_SHOW_TASK_STK[PIC_SHOW_STK_SIZE];
//任务函数
void show_mp3_pic(void *pdata);

//测试任务
//设置任务优先级
#define TEST_TASK_PRIO       		1
//设置任务堆栈大小
#define TEST_STK_SIZE  		      128
//任务控制块
OS_TCB TESTTaskTCB;
//任务堆栈	
CPU_STK TEST_TASK_STK[TEST_STK_SIZE];
//任务函数
void test(void *pdata);

static FIL PNGFile;

//static char pngBuffer[PNGPERLINESIZE];
/*******************************************************************
*
*       Static functions
*
********************************************************************
*/
/*********************************************************************
*
*       PngGetData
*
* Function description
*   This routine is called by GUI_PNG_DrawEx(). The routine is responsible
*   for setting the data pointer to a valid data location with at least
*   one valid byte.
*
* Parameters:
*   p           - Pointer to application defined data.
*   NumBytesReq - Number of bytes requested.
*   ppData      - Pointer to data pointer. This pointer should be set to
*                 a valid location.
*   StartOfFile - If this flag is 1, the data pointer should be set to the
*                 beginning of the data stream.
*
* Return value:
*   Number of data bytes available.
*/
static int PngGetData(void * p, const U8 ** ppData, unsigned NumBytesReq, U32 Off) 
{
	static int readaddress=0;
	FIL * phFile;
	U8 *pData;
	UINT NumBytesRead;
	OS_ERR err;
	CPU_SR_ALLOC();
	
	pData = (U8*)*ppData;
	phFile = (FIL *)p;
	
	//移动指针到应该读取的位置
	if(Off == 1) readaddress = 0;
	else readaddress=Off;
	
	OS_CRITICAL_ENTER();	//进入临界区
	
	f_lseek(phFile,readaddress); 
	
	//读取数据到缓冲区中
	f_read(phFile,pData,NumBytesReq,&NumBytesRead);
	
	OS_CRITICAL_EXIT();	//进入临界区

	return NumBytesRead;//返回读取到的字节数
}

//在指定位置显示无需加载到RAM中的PNG图片(需文件系统支持！对于小RAM，推荐使用此方法！PNG图片不能缩放！)
//PNGFileName:图片在SD卡或者其他存储设备中的路径
//mode:显示模式
//		0 在指定位置显示，有参数x,y确定显示位置
//		1 在LCD中间显示图片，当选择此模式的时候参数x,y无效。
//x:图片左上角在LCD中的x轴位置(当参数mode为1时，此参数无效)
//y:图片左上角在LCD中的y轴位置(当参数mode为1时，此参数无效)
//返回值:0 显示正常,其他 失败
int displaypngex(char *PNGFileName,u8 mode,u32 x,u32 y)
{
	char result;
	int XSize,YSize;

	result = f_open(&PNGFile,(const TCHAR*)PNGFileName,FA_READ);	//打开文件
	//文件打开错误
	if(result != FR_OK)	return 1;
	
	/*
	XSize = GUI_PNG_GetXSizeEx(PngGetData,&PNGFile);//PNG图片X大小
	YSize = GUI_PNG_GetYSizeEx(PngGetData,&PNGFile);//PNG图片Y大小
	switch(mode)
	{
		case 0:	//在指定位置显示图片
			GUI_PNG_DrawEx(PngGetData,&PNGFile,x,y);
			break;
		case 1:	//在LCD中间显示图片
			GUI_PNG_DrawEx(PngGetData,&PNGFile,(lcddev.width-XSize)/2-1,(lcddev.height-YSize)/2-1);
			break;
	}
	*/
	GUI_PNG_DrawEx(PngGetData, &PNGFile, x, y);
	f_close(&PNGFile);	//关闭PNGFile文件
	return 0;
}

void test(void *pdata)
{
	GUI_Init();
  BUTTON_SetDefaultSkin(BUTTON_SKIN_FLEX);
	CHECKBOX_SetDefaultSkin(CHECKBOX_SKIN_FLEX);
	DROPDOWN_SetDefaultSkin(DROPDOWN_SKIN_FLEX);
	FRAMEWIN_SetDefaultSkin(FRAMEWIN_SKIN_FLEX);
	HEADER_SetDefaultSkin(HEADER_SKIN_FLEX);
	MENU_SetDefaultSkin(MENU_SKIN_FLEX);
	MULTIPAGE_SetDefaultSkin(MULTIPAGE_SKIN_FLEX);
	PROGBAR_SetDefaultSkin(PROGBAR_SKIN_FLEX);
	RADIO_SetDefaultSkin(RADIO_SKIN_FLEX);
	SCROLLBAR_SetDefaultSkin(SCROLLBAR_SKIN_FLEX);
	SLIDER_SetDefaultSkin(SLIDER_SKIN_FLEX);
	SPINBOX_SetDefaultSkin(SPINBOX_SKIN_FLEX);
	//GUI_CURSOR_Show();
	GUI_SetBkColor(GUI_BLUE);
	//GUI_SetFont(&GUI_FontHZ16);
	GUI_SetColor(GUI_RED);
	GUI_Clear();
	displaypngex("0:/test.png",0,0,0);
	while(1);
}

//开始任务
void start_task(void *pdata)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	pdata = pdata;

	CPU_Init();
	/*
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//统计任务
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN		//如果使能了测量中断关闭时间
	CPU_IntDisMeasMaxCurReset();
#endif

#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //当使用时间片轮转的时候
	 //使能时间片轮转调度功能,时间片长度为1个系统时钟节拍，既1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);
#endif
	*/
	OSSchedRoundRobinCfg(DEF_ENABLED, 1, &err);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);//开启CRC时钟

	OS_CRITICAL_ENTER();	//进入临界区
	/*
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
		*/
	OSTaskCreate((OS_TCB 	*)&TESTTaskTCB,
		(CPU_CHAR	*)"TEST task",
		(OS_TASK_PTR)test,
		(void		*)0,
		(OS_PRIO)TEST_TASK_PRIO,
		(CPU_STK   *)&TEST_TASK_STK[0],
		(CPU_STK_SIZE)TEST_STK_SIZE / 10,
		(CPU_STK_SIZE)TEST_STK_SIZE,
		(OS_MSG_QTY)0,
		(OS_TICK)0,
		(void   	*)0,
		(OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
		(OS_ERR 	*)&err);	
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

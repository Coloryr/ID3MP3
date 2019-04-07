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

/////////////////////////UCOSII��������///////////////////////////////////
//START ����
//�������ȼ�
#define START_TASK_PRIO		3
//�����ջ��С	
#define START_STK_SIZE 		64
//������ƿ�
OS_TCB StartTaskTCB;
//�����ջ	
CPU_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata);	
 			   
//MP3����
//�����������ȼ�
#define MUSIC_PLAY_TASK_PRIO       	1
//���������ջ��С
#define MUSIC_PLAY_STK_SIZE  		    128
//������ƿ�
OS_TCB MUSICTaskTCB;
//�����ջ	
CPU_STK MUSIC_PLAY_TASK_STK[MUSIC_PLAY_STK_SIZE];
//������
void mp3_play(void *pdata);

//ͼƬ��ʾ����
//�����������ȼ�
#define PIC_SHOW_TASK_PRIO       		1
//���������ջ��С
#define PIC_SHOW_STK_SIZE  		      128
//������ƿ�
OS_TCB PICTaskTCB;
//�����ջ	
CPU_STK PIC_SHOW_TASK_STK[PIC_SHOW_STK_SIZE];
//������
void show_mp3_pic(void *pdata);

//��������
//�����������ȼ�
#define TEST_TASK_PRIO       		1
//���������ջ��С
#define TEST_STK_SIZE  		      128
//������ƿ�
OS_TCB TESTTaskTCB;
//�����ջ	
CPU_STK TEST_TASK_STK[TEST_STK_SIZE];
//������
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
	
	//�ƶ�ָ�뵽Ӧ�ö�ȡ��λ��
	if(Off == 1) readaddress = 0;
	else readaddress=Off;
	
	OS_CRITICAL_ENTER();	//�����ٽ���
	
	f_lseek(phFile,readaddress); 
	
	//��ȡ���ݵ���������
	f_read(phFile,pData,NumBytesReq,&NumBytesRead);
	
	OS_CRITICAL_EXIT();	//�����ٽ���

	return NumBytesRead;//���ض�ȡ�����ֽ���
}

//��ָ��λ����ʾ������ص�RAM�е�PNGͼƬ(���ļ�ϵͳ֧�֣�����СRAM���Ƽ�ʹ�ô˷�����PNGͼƬ�������ţ�)
//PNGFileName:ͼƬ��SD�����������洢�豸�е�·��
//mode:��ʾģʽ
//		0 ��ָ��λ����ʾ���в���x,yȷ����ʾλ��
//		1 ��LCD�м���ʾͼƬ����ѡ���ģʽ��ʱ�����x,y��Ч��
//x:ͼƬ���Ͻ���LCD�е�x��λ��(������modeΪ1ʱ���˲�����Ч)
//y:ͼƬ���Ͻ���LCD�е�y��λ��(������modeΪ1ʱ���˲�����Ч)
//����ֵ:0 ��ʾ����,���� ʧ��
int displaypngex(char *PNGFileName,u8 mode,u32 x,u32 y)
{
	char result;
	int XSize,YSize;

	result = f_open(&PNGFile,(const TCHAR*)PNGFileName,FA_READ);	//���ļ�
	//�ļ��򿪴���
	if(result != FR_OK)	return 1;
	
	/*
	XSize = GUI_PNG_GetXSizeEx(PngGetData,&PNGFile);//PNGͼƬX��С
	YSize = GUI_PNG_GetYSizeEx(PngGetData,&PNGFile);//PNGͼƬY��С
	switch(mode)
	{
		case 0:	//��ָ��λ����ʾͼƬ
			GUI_PNG_DrawEx(PngGetData,&PNGFile,x,y);
			break;
		case 1:	//��LCD�м���ʾͼƬ
			GUI_PNG_DrawEx(PngGetData,&PNGFile,(lcddev.width-XSize)/2-1,(lcddev.height-YSize)/2-1);
			break;
	}
	*/
	GUI_PNG_DrawEx(PngGetData, &PNGFile, x, y);
	f_close(&PNGFile);	//�ر�PNGFile�ļ�
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

//��ʼ����
void start_task(void *pdata)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	pdata = pdata;

	CPU_Init();
	/*
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//ͳ������
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN		//���ʹ���˲����жϹر�ʱ��
	CPU_IntDisMeasMaxCurReset();
#endif

#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //��ʹ��ʱ��Ƭ��ת��ʱ��
	 //ʹ��ʱ��Ƭ��ת���ȹ���,ʱ��Ƭ����Ϊ1��ϵͳʱ�ӽ��ģ���1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);
#endif
	*/
	OSSchedRoundRobinCfg(DEF_ENABLED, 1, &err);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);//����CRCʱ��

	OS_CRITICAL_ENTER();	//�����ٽ���
	/*
//����MP3����
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
	OS_TaskSuspend((OS_TCB*)&StartTaskTCB, &err);		//����ʼ����			 
	OS_CRITICAL_EXIT();	//�����ٽ���
}

void APP_start(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	OSInit(&err);		//��ʼ��UCOSIII
	OS_CRITICAL_ENTER();//�����ٽ���
	//������ʼ����
	OSTaskCreate((OS_TCB 	*)&StartTaskTCB,		//������ƿ�
		(CPU_CHAR	*)"start task", 		//��������
		(OS_TASK_PTR)start_task, 			//������
		(void		*)0,					//���ݸ��������Ĳ���
		(OS_PRIO)PIC_SHOW_TASK_PRIO,     //�������ȼ�
		(CPU_STK   *)&START_TASK_STK[0],	//�����ջ����ַ
		(CPU_STK_SIZE)START_STK_SIZE / 10,	//�����ջ�����λ
		(CPU_STK_SIZE)START_STK_SIZE,		//�����ջ��С
		(OS_MSG_QTY)0,					//�����ڲ���Ϣ�����ܹ����յ������Ϣ��Ŀ,Ϊ0ʱ��ֹ������Ϣ
		(OS_TICK)0,					//��ʹ��ʱ��Ƭ��תʱ��ʱ��Ƭ���ȣ�Ϊ0ʱΪĬ�ϳ��ȣ�
		(void   	*)0,					//�û�����Ĵ洢��
		(OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR, //����ѡ��
		(OS_ERR 	*)&err);				//��Ÿú�������ʱ�ķ���ֵ
	OS_CRITICAL_EXIT();	//�˳��ٽ���	 
	OSStart(&err);  //����UCOSIII
	while (1);
}

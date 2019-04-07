#include "app_start.h"
#include "mp3player.h"
#include "includes.h"
#include "show.h"
#include "GUIx.h"
#include "key.h"
#include "lcd.h"
#include "fontupd.h"
#include "data.h"

/////////////////////////UCOSII��������///////////////////////////////////
//START ����
//�������ȼ�
#define START_TASK_PRIO 3
//�����ջ��С
#define START_STK_SIZE 64
//������ƿ�
OS_TCB StartTaskTCB;
//�����ջ
CPU_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata);

//MP3����
//�����������ȼ�
#define MUSIC_PLAY_TASK_PRIO 1
//���������ջ��С
#define MUSIC_PLAY_STK_SIZE 128
//������ƿ�
OS_TCB MUSICTaskTCB;
//�����ջ
CPU_STK MUSIC_PLAY_TASK_STK[MUSIC_PLAY_STK_SIZE];
//������
void mp3_play(void *pdata);

//ͼƬ��ʾ����
//�����������ȼ�
#define PIC_SHOW_TASK_PRIO 1
//���������ջ��С
#define PIC_SHOW_STK_SIZE 128
//������ƿ�
OS_TCB PICTaskTCB;
//�����ջ
CPU_STK PIC_SHOW_TASK_STK[PIC_SHOW_STK_SIZE];
//������
void show_mp3_pic(void *pdata);

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
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE); //����CRCʱ��

	OS_CRITICAL_ENTER(); //�����ٽ���
						 //����MP3����
	OSTaskCreate((OS_TCB *)&MUSICTaskTCB,
				 (CPU_CHAR *)"mp3 task",
				 (OS_TASK_PTR)mp3_play,
				 (void *)0,
				 (OS_PRIO)MUSIC_PLAY_TASK_PRIO,
				 (CPU_STK *)&MUSIC_PLAY_TASK_STK[0],
				 (CPU_STK_SIZE)MUSIC_PLAY_STK_SIZE / 10,
				 (CPU_STK_SIZE)MUSIC_PLAY_STK_SIZE,
				 (OS_MSG_QTY)0,
				 (OS_TICK)0,
				 (void *)0,
				 (OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
				 (OS_ERR *)&err);
	OSTaskCreate((OS_TCB *)&PICTaskTCB,
				 (CPU_CHAR *)"pic_show task",
				 (OS_TASK_PTR)show_mp3_pic,
				 (void *)0,
				 (OS_PRIO)PIC_SHOW_TASK_PRIO,
				 (CPU_STK *)&PIC_SHOW_TASK_STK[0],
				 (CPU_STK_SIZE)PIC_SHOW_STK_SIZE / 10,
				 (CPU_STK_SIZE)PIC_SHOW_STK_SIZE,
				 (OS_MSG_QTY)0,
				 (OS_TICK)0,
				 (void *)0,
				 (OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
				 (OS_ERR *)&err);
	OS_TaskSuspend((OS_TCB *)&StartTaskTCB, &err); //����ʼ����
	OS_CRITICAL_EXIT();							   //�����ٽ���
}

void APP_start(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	OSInit(&err);		 //��ʼ��UCOSIII
	OS_CRITICAL_ENTER(); //�����ٽ���
	//������ʼ����
	OSTaskCreate((OS_TCB *)&StartTaskTCB,							//������ƿ�
				 (CPU_CHAR *)"start task",							//��������
				 (OS_TASK_PTR)start_task,							//������
				 (void *)0,											//���ݸ��������Ĳ���
				 (OS_PRIO)PIC_SHOW_TASK_PRIO,						//�������ȼ�
				 (CPU_STK *)&START_TASK_STK[0],						//�����ջ����ַ
				 (CPU_STK_SIZE)START_STK_SIZE / 10,					//�����ջ�����λ
				 (CPU_STK_SIZE)START_STK_SIZE,						//�����ջ��С
				 (OS_MSG_QTY)0,										//�����ڲ���Ϣ�����ܹ����յ������Ϣ��Ŀ,Ϊ0ʱ��ֹ������Ϣ
				 (OS_TICK)0,										//��ʹ��ʱ��Ƭ��תʱ��ʱ��Ƭ���ȣ�Ϊ0ʱΪĬ�ϳ��ȣ�
				 (void *)0,											//�û�����Ĵ洢��
				 (OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR, //����ѡ��
				 (OS_ERR *)&err);									//��Ÿú�������ʱ�ķ���ֵ
	OS_CRITICAL_EXIT();												//�˳��ٽ���
	OSStart(&err);													//����UCOSIII
	while (1)
		;
}

void Adc_Init(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_ADC1, ENABLE); //ʹ��IO��ʱ�ӣ�ADC1ͨ��ʱ��

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);

	//PC4 ��Ϊģ������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; //ģ����������
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	ADC_DeInit(ADC1); //��λADC1

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;					//ADC����ģʽ������ģʽ
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;						//ħ��ת�������ڵ�ͨ��
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;					//����ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //ת�����������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;				//ADC�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = 1;								//˳����й���ת��
	ADC_Init(ADC1, &ADC_InitStructure);									//����ADC_InitStruct��ָ�� �Ĳ�����ʼ������ADCx�ļĴ���

	ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 1, ADC_SampleTime_239Cycles5);
	ADC_Cmd(ADC1, ENABLE);		//ʹ��ָ����ADC1
	ADC_ResetCalibration(ADC1); //ʹ�ܸ�λУ׼
	while (ADC_GetResetCalibrationStatus(ADC1))
		;						//�ȴ���λУ׼����
	ADC_StartCalibration(ADC1); //����ADУ׼
	while (ADC_GetCalibrationStatus(ADC1))
		; //�ȴ�У׼����
}
u16 Get_Adc_Average(void)
{
	u16 temp_val;
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
		;
	temp_val = ADC_GetConversionValue(ADC1);
	ADC_SoftwareStartConvCmd(ADC1, DISABLE);
	return temp_val;
}

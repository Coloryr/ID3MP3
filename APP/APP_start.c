#include "includes.h" 

/////////////////////////UCOSII��������///////////////////////////////////
//START ����
//�������ȼ�
#define START_TASK_PRIO		3
//�����ջ��С	
#define START_STK_SIZE 		128
//������ƿ�
OS_TCB StartTaskTCB;
//�����ջ	
CPU_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata);	
 			   
//MP3����
//�����������ȼ�
#define MUSIC_PLAY_TASK_PRIO       	2 
//���������ջ��С
#define MUSIC_PLAY_STK_SIZE  		    256
//������ƿ�
OS_TCB MUSICTaskTCB;
//�����ջ	
CPU_STK MUSIC_PLAY_TASK_STK[MUSIC_PLAY_STK_SIZE];
//������
void mp3_play(void *pdata);

//ͼƬ��ʾ����
//�����������ȼ�
#define PIC_SHOW_TASK_PRIO       		2
//���������ջ��С
#define PIC_SHOW_STK_SIZE  		    2096
//������ƿ�
OS_TCB PICTaskTCB;
//�����ջ	
CPU_STK PIC_SHOW_TASK_STK[PIC_SHOW_STK_SIZE];
//������
void show_mp3_pic(void *pdata);

//��ʾ����
//�����������ȼ�
#define SHOW_TASK_PRIO       			2
//���������ջ��С
#define SHOW_STK_SIZE  		    		64
//������ƿ�
OS_TCB SHOWTaskTCB;
//�����ջ	
CPU_STK SHOW_TASK_STK[SHOW_STK_SIZE];
//������
//void show_all(void *pdata);

////////////////////////////////α����������취////////////////////////////////
u32 random_seed=1;
void app_srand(u32 seed)
{
	random_seed = seed;
}
//��ȡα�����
//���Բ���0~RANDOM_MAX-1�������
//seed:����
//max:���ֵ	  		  
//����ֵ:0~(max-1)�е�һ��ֵ 		
u32 app_get_rand(u32 max)
{
	random_seed = random_seed * 22695477 + 1;
	return (random_seed) % max;
}

//��ʼ����
void start_task(void *pdata)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	pdata = pdata;

	CPU_Init();
	//ʹ��ʱ��Ƭ��ת���ȹ���,ʱ��Ƭ����Ϊ1��ϵͳʱ�ӽ��ģ���1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED, 1, &err);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);//����CRCʱ��		

	OS_CRITICAL_ENTER();	//�����ٽ���
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





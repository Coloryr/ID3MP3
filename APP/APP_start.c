#include "sys.h"
#include "APP_start.h"
#include "includes.h" 
#include "show.h" 
#include "mp3player.h"

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
#define MUSIC_PLAY_TASK_PRIO       		1 
//���������ջ��С
#define MUSIC_PLAY_STK_SIZE  		    256
//�����ջ��8�ֽڶ���	
__align(8) OS_STK MUSIC_PLAY_TASK_STK[MUSIC_PLAY_STK_SIZE];
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
void show_mp3_pic(void *pdata);

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
  OSTaskCreate(mp3_play,(void *)0,(OS_STK *)&MUSIC_PLAY_TASK_STK[MUSIC_PLAY_STK_SIZE-1],MUSIC_PLAY_TASK_PRIO);
	OSTaskSuspend(START_TASK_PRIO);	//������ʼ����.
	OS_EXIT_CRITICAL();	//�˳��ٽ���(���Ա��жϴ��)
}

void APP_start(void)
{
	OSInit();   
	//������ʼ����
	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );
	OSStart();
}

void APP_pic_start(void)
{
		OS_CPU_SR cpu_sr=0;
	OS_ENTER_CRITICAL();//�����ٽ���(�޷����жϴ��)    
	OSTaskCreate(show_mp3_pic,(void *)0,(OS_STK *)&PIC_SHOW_TASK_STK[PIC_SHOW_STK_SIZE-1],PIC_SHOW_TASK_PRIO);
	OS_EXIT_CRITICAL();	//�˳��ٽ���(���Ա��жϴ��)
}





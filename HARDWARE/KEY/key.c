#include "includes.h" 

u8 key_now;

//������ʼ������ 
//PA0.15��PC5 ���ó�����
void KEY_Init(void)
{

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);//ʹ��PORTA,PORTCʱ��

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//�ر�jtag��ʹ��SWD��������SWDģʽ����

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_2 | GPIO_Pin_1 | GPIO_Pin_0;//PA15
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
	GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIOA15
}
//����������
//���ذ���ֵ
//mode:0,��֧��������;1,֧��������;
//����ֵ��
//0��û���κΰ�������
//KEY0_PRES��KEY0����
//KEY1_PRES��KEY1����
//WKUP_PRES��WK_UP���� 
//ע��˺�������Ӧ���ȼ�,KEY0>KEY1>WK_UP!!
u8 KEY_Scan(u8 mode)
{
	static u8 key_up = 1;//�������ɿ���־
	if (mode)key_up = 1;  //֧������		  
	if (key_up && (KEY0 == 0 || KEY1 == 0 || KEY2 == 0 || KEY3 == 0))
	{
		delay_ms(10);
		key_up = 0;
		if (KEY0 == 0)return KEY0_PRES;
		else if (KEY1 == 0)return KEY1_PRES;
		else if (KEY2 == 0)return KEY2_PRES;
		else if (KEY3 == 0)return KEY3_PRES;
	}
	else if (KEY0 == 1 && KEY1 == 1 && KEY2 == 1 && KEY3 == 1)key_up = 1;
	return 0;// �ް�������
}
//����ɨ��KEY0�Ƿ񱻰���
//����ֵ:0,û����;1,������.
u8 KEY0_Scan(void)
{
	static u8 key_up = 1;//�������ɿ���־ 	  
	if (key_up&&KEY0 == 0)
	{
		delay_ms(10);//ȥ���� 
		key_up = 0;
		if (KEY0 == 0)return 1;
	}
	else if (KEY0 == 1)key_up = 1;
	return 0;// �ް�������
}

void KEY_task(void *pdata)
{
	OS_ERR err;
	while (1)
	{
		key_now = KEY_Scan(0);
		OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_PERIODIC, &err);//��ʱ10ms
	}
}




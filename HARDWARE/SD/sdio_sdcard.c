/**********************************************************
* @ File name -> sdio_sdcard.c
* @ Version   -> V1.0
* @ Date      -> 02-09-2014
* @ Brief     -> SDCard��SDIO��������

 V1.
* @ Revise    -> 
**********************************************************/

#include "sdio_sdcard.h"

/* Ϊ�����ݶ��룬�����buffer��Ϊ��дdisk׼���ģ������ݲ���4K�����ʱ����õ�������ֱ�Ӳ������� */

__align(4) uint8_t SDIO_DATA_BUFFER[512];

/**********************************************************
                       ������صı���
**********************************************************/

static uint32_t CardType = SDIO_STD_CAPACITY_SD_CARD_V1_1; //Ĭ�Ͽ�������Ϊ��׼V1.1��
static uint32_t CSD_Tab[4], CID_Tab[4], RCA = 0;		   //CID��CSD��RCA����
static uint8_t SDSTATUS_Tab[16];						   //SD��״̬����

__IO SD_Error TransferError = SD_OK;
__IO uint32_t StopCondition = 0; //�Ƿ���ֹͣ���������־������дʱ���õ�
__IO uint32_t TransferEnd = 0;   //���������־
SD_CardInfo SDCardInfo;			 //SD����Ϣ�ṹ����

/**********************************************************
                        ���ܺ���
**********************************************************/

static SD_Error CmdError(void);
static SD_Error CmdResp1Error(uint8_t cmd);
static SD_Error CmdResp7Error(void);
static SD_Error CmdResp3Error(void);
static SD_Error CmdResp2Error(void);
static SD_Error CmdResp6Error(uint8_t cmd, uint16_t *prca);
static SD_Error SDEnWideBus(FunctionalState NewState);
static SD_Error IsCardProgramming(uint8_t *pstatus);
static SD_Error FindSCR(uint16_t rca, uint32_t *pscr);
uint8_t convert_from_bytes_to_power_of_two(uint16_t NumberOfBytes);

/**********************************************************
* �������� ---> ��ȡоƬ��������
* ��ڲ��� ---> *STMCapa�������ַ���ʾ����
* ������ֵ ---> ������ʮ���ƣ�
* ����˵�� ---> none
**********************************************************/
void STM32_Flash_Capacity(uint8_t *STMCapa)
{
	uint16_t capa;

	capa = *((uint16_t *)0x1ffff7e0); //��ȡ���������Ĵ�������16λ��Ч

	capa = ((capa >> 12) * 4096 + ((capa >> 8) & 0x0f) * 256 + ((capa >> 4) & 0x0f) * 16 + (capa & 0x0f)); //ת����ʮ����

	STMCapa[0] = 0x4d; //M
	STMCapa[1] = 0x43; //C
	STMCapa[2] = 0x55; //U
	STMCapa[3] = 0x43; //C
	STMCapa[4] = 0x61; //a
	STMCapa[5] = 0x70; //p
	STMCapa[6] = 0x3a; //:

	if ((capa / 1000) != 0)
		STMCapa[7] = capa / 1000 + 48; //ǧλ��Ϊ0ʱ��ʾ
	else
		STMCapa[7] = 0x20;

	STMCapa[8] = capa % 1000 / 100 + 48; //��λ
	STMCapa[9] = capa % 100 / 10 + 48;   //ʮλ
	STMCapa[10] = capa % 10 + 48;		 //��λ
	STMCapa[11] = 0x4b;					 //K
	STMCapa[12] = 0x62;					 //b
}
/**********************************************************
* �������� ---> ����������ƫ�Ƶ�ַ
* ��ڲ��� ---> NVIC_VectTab����ַ
*              Offset��ƫ����	
* ������ֵ ---> ������ʮ���ƣ�
* ����˵�� ---> none
**********************************************************/
void MY_NVIC_SetVectorTable(uint32_t NVIC_VectTab, uint32_t Offset)
{
	SCB->VTOR = NVIC_VectTab | (Offset & (uint32_t)0x1fffff80); //����NVIC��������ƫ�ƼĴ���
																//���ڱ�ʶ����������CODE��������RAM��
}
/**********************************************************
* �������� ---> �����жϷ���
* ��ڲ��� ---> NVIC_PriorityGroup: �жϷ���
* ������ֵ ---> none
* ����˵�� ---> 0 ~ 4�飬������5��
**********************************************************/
void MY_NVIC_PriorityGroup_Config(uint32_t NVIC_PriorityGroup)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup); //�����жϷ���
}
/**********************************************************
* �������� ---> �����жϷ������ȼ�
* ��ڲ��� ---> NVIC_PreemptionPriority���������ȼ�
*               NVIC_Subpriority����Ӧ���ȼ�
*               NVIC_Channel���жϱ��
*               NVIC_Group: �жϷ���
* ������ֵ ---> none
* ����˵�� ---> 1���黮��:
*                  ��0��0λ��ռ���ȼ���4λ��Ӧ���ȼ�
*                  ��1��1λ��ռ���ȼ���3λ��Ӧ���ȼ�
*                  ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
*                  ��3��3λ��ռ���ȼ���1λ��Ӧ���ȼ�
*                  ��4��4λ��ռ���ȼ���0λ��Ӧ���ȼ�
*               2���������ȼ���������Ӧ���ȼ�����ԭ��������ֵԽС���ȼ�Խ��
**********************************************************/
void MY_NVIC_Init(uint8_t NVIC_PreemptionPriority, uint8_t NVIC_Subpriority, uint8_t NVIC_Channel, uint32_t NVIC_Group)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	MY_NVIC_PriorityGroup_Config(NVIC_Group); //�����жϷ���

	NVIC_InitStructure.NVIC_IRQChannel = NVIC_Channel;								//�����ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PreemptionPriority; //�������ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = NVIC_Subpriority;				//��Ӧ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;									//ʹ���ж�

	NVIC_Init(&NVIC_InitStructure); //��ʼ���ж�
}

/**********************************************************
* �������� ---> ����ʱ�ӼĴ�����λ
* ��ڲ��� ---> none
* ������ֵ ---> none
* ����˵�� ---> ����������ִ���������踴λ!�����������𴮿ڲ�����
**********************************************************/
void MY_RCC_DeInit(void)
{
	RCC->APB1RSTR = 0x00000000; //��λ����
	RCC->APB2RSTR = 0x00000000;

	RCC->AHBENR = 0x00000014;  //˯��ģʽ�����SRAMʱ��ʹ��.�����ر�.
	RCC->APB2ENR = 0x00000000; //����ʱ�ӹر�.
	RCC->APB1ENR = 0x00000000;
	RCC->CR |= 0x00000001;   //ʹ���ڲ�����ʱ��HSION
	RCC->CFGR &= 0xf8ff0000; //��λSW[1:0],HPRE[3:0],PPRE1[2:0],PPRE2[2:0],ADCPRE[1:0],MCO[2:0]
	RCC->CR &= 0xfef6ffff;   //��λHSEON,CSSON,PLLON
	RCC->CR &= 0xfffbffff;   //��λHSEBYP
	RCC->CFGR &= 0xff80ffff; //��λPLLSRC, PLLXTPRE, PLLMUL[3:0] and USBPRE
	RCC->CIR = 0x00000000;   //�ر������ж�

	//����������
#ifdef VECT_TAB_RAM
	MY_NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else
	MY_NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif
}
/**********************************************************
* �������� ---> ����оƬ�������ģʽ
* ��ڲ��� ---> none
* ������ֵ ---> none
* ����˵�� ---> ����WKUP���Ż��ѣ�PA.0��
**********************************************************/
void SYS_Standby(void)
{
	SCB->SCR |= 1 << 2;		 //ʹ��sleep deep��SYS->CTRL��
	RCC->APB1ENR |= 1 << 28; //��Դ�ӿ�ʱ�ӿ���
	PWR->CSR |= 1 << 8;		 //����WKUP�������ڻ���
	PWR->CR |= 1 << 2;		 //���WAKE-UP��־
	PWR->CR |= 1 << 1;		 //PDDS��λ��������˯��
	WFI_SET();				 //ִ��WFIָ��
}
/**********************************************************
* �������� ---> ϵͳ��λ
* ��ڲ��� ---> none
* ������ֵ ---> none
* ����˵�� ---> none
**********************************************************/
void SYS_SoftReset(void)
{
	SCB->AIRCR = 0x05fa0000 | 0x00000004;
}
/**********************************************************
* �������� ---> JTAGģʽ����
* ��ڲ��� ---> mode��ģʽ����
*                    000��JTAG-DP + SW-DP����λ״̬��
*                    001��JTAG-DP + SW-DP������JNTRST���ţ��ͷ�JRST���ţ�
*                    010��JTAG-DP�ӿڽ�ֹ��SW-DP�ӿ�����
*                    100��JTAG-DP�ӿں�SW-DP�ӿڶ���ֹ
*                    xxx������ֵ����ֹ
* ������ֵ ---> none
* ����˵�� ---> none
**********************************************************/
void STM_JTAG_Set(uint32_t mode)
{
	RCC->APB2ENR |= 1 << 0;		//ʹ�ܸ���ʱ��
	AFIO->MAPR &= 0xf8ffffff;   //���SWJ_CFG[2:0]λ������26:24��
	AFIO->MAPR |= (mode << 24); //����JTAGģʽ
}
/**********************************************************
* �������� ---> ϵͳʱ�ӳ�ʼ��
* ��ڲ��� ---> pll����Ƶ����ȡֵ��Χ��2 ~ 16
* ������ֵ ---> none
* ����˵�� ---> none
**********************************************************/
void STM_Clock_Init(uint8_t pll)
{
	uint8_t tmp = 0;

	MY_RCC_DeInit(); //��λ���������������ҽ��ⲿ�жϺ�����ʱ��ȫ���ر�

	RCC->CR |= 0x00010000; //�ⲿ����ʱ��ʹ��HSEON

	while (!(RCC->CR >> 17))
		; //�ȴ��ⲿʱ�Ӿ���

	RCC->CFGR = 0X00000400; //APB1=DIV2;APB2=DIV1;AHB=DIV1;
	pll -= 2;				//����2����λ
	RCC->CFGR |= pll << 18; //����PLLֵ 2~16
	RCC->CFGR |= 1 << 16;   //PLLSRC ON
	FLASH->ACR |= 0x32;		//FLASH 2����ʱ����

	RCC->CR |= 0x01000000; //PLLON

	while (!(RCC->CR >> 25))
		; //�ȴ�PLL����

	RCC->CFGR |= 0x00000002; //PLL��Ϊϵͳʱ�ӣ������ܿ���PLL����Ϊ�趨PLLʱ��PLL���λ�����ڹر�״̬�½���

	while (tmp != 0x02) //�ȴ�PLL��Ϊϵͳʱ�����óɹ�
	{
		tmp = RCC->CFGR >> 2;
		tmp &= 0x03;
	}
}
/**********************************************************
* �������� ---> BCD��תΪHEX
* ��ڲ��� ---> BCD_Data��Ҫת����BCD����
* ������ֵ ---> HEX��
* ����˵�� ---> none
**********************************************************/
uint8_t BCD_to_HEX(uint8_t BCD_Data)
{
	return ((BCD_Data / 10) << 4 | (BCD_Data % 10));
}
/**********************************************************
* �������� ---> HEX��תΪBCD
* ��ڲ��� ---> HEX_Data��Ҫת����BCD����
* ������ֵ ---> BCD��
* ����˵�� ---> none
**********************************************************/
uint8_t HEX_to_BCD(uint8_t HEX_Data)
{
	return ((HEX_Data >> 4) * 10 + (HEX_Data & 0x0f));
}
/**********************************************************
* �������� ---> 10������תΪ16����
* ��ڲ��� ---> DX_Data��Ҫת����10��������
* ������ֵ ---> 16����
* ����˵�� ---> none
**********************************************************/
uint16_t DX_to_HX(uint16_t DX_Data)
{
	return (((DX_Data / 1000) << 12) | ((DX_Data % 1000 / 100) << 8) | ((DX_Data % 100 / 10) << 4) | (DX_Data % 10));
}
/**********************************************************
* �������� ---> 16������תΪ10����
* ��ڲ��� ---> HX_Data��Ҫת����16��������
* ������ֵ ---> 10����
* ����˵�� ---> none
**********************************************************/
uint16_t HX_to_DX(uint16_t HX_Data)
{
	return ((HX_Data >> 12) * 1000 + ((HX_Data & 0x0f00) >> 8) * 100 + ((HX_Data & 0x00f0) >> 4) * 10 + (HX_Data & 0x000f));
}

/**********************************************************
* �������� ---> DMA2����״̬
* ��ڲ��� ---> none
* ������ֵ ---> ״ֵ̬
* ����˵�� ---> none
**********************************************************/
FlagStatus SD_DMAEndOfTransferStatus(void)
{
	if (DMA2->ISR & (1 << 13))
		return (SET); //��ͨ��4�����˴�������¼�
	else
		return (RESET); //��ͨ��4û�д�������¼�
}
/**********************************************************
* �������� ---> SDIO DMA������������
* ��ڲ��� ---> *DMABuffer���������ݻ���
*               BufferSize���������ݴ�С
*               dir�����ݴ��ͷ���0����ȡ���ݣ��� --> SDIO������
*                                  1��д�����ݣ�SDIO���� --> ����
* ������ֵ ---> none
* ����˵�� ---> none
**********************************************************/
static void SDIO_DMA_SendData(uint32_t *DMABuffer, uint32_t BufferSize, uint8_t dir)
{
	DMA2->IFCR |= 0x0000f000; //���DMA2ͨ������жϱ�־λ

	DMA2_Channel4->CCR &= !(1 << 0);		   //ͨ�����������ر�DMA2
	DMA2_Channel4->CCR &= 0x00000000;		   //���֮ǰ�����ã�MEM2_MEM��PL��MSIZE��PSIZE��MINC��PINC��CIRC��DIR
	DMA2_Channel4->CCR |= 0 << 14;			   //�Ǵ洢�����洢��ģʽ
	DMA2_Channel4->CCR |= 2 << 12;			   //ͨ�����ȼ���
	DMA2_Channel4->CCR |= 2 << 10;			   //�洢�����ݿ��32bits
	DMA2_Channel4->CCR |= 2 << 8;			   //�������ݿ��32bits
	DMA2_Channel4->CCR |= 1 << 7;			   //ִ�д洢����ַ��������
	DMA2_Channel4->CCR |= 0 << 6;			   //��ִ�������ַ��������
	DMA2_Channel4->CCR |= 0 << 5;			   //��ִ��ѭ������
	DMA2_Channel4->CCR |= ((dir & 0x01) << 4); //���ô��䷽��

	DMA2_Channel4->CNDTR = BufferSize / 4;		 //���ݴ�����������16λ��Ч
	DMA2_Channel4->CPAR = (uint32_t)&SDIO->FIFO; //�����ַ
	DMA2_Channel4->CMAR = (uint32_t)DMABuffer;   //�洢����ַ�Ĵ���

	DMA2_Channel4->CCR |= 1 << 0; //ͨ��������ʹ��DMA2
}
/**********************************************************
* �������� ---> SDIO�����������
* ��ڲ��� ---> CMDindex�������ֽڣ������ֽڵ�6λ��Ч
*               Arg������
*               WaitResp����Ӧ���ȡ�00/10������Ӧ
*                                      01������Ӧ
*                                      11������Ӧ
* ������ֵ ---> none
* ����˵�� ---> none
**********************************************************/
static void SDIO_Send_CMDIndex(uint8_t CMDindex, uint32_t Arg, uint8_t WaitResp)
{
	SDIO->ARG = Arg;					   //�������
	SDIO->CMD &= 0xfffff800;			   //���index��waitresp
	SDIO->CMD |= (CMDindex & 0x3f);		   //�����������6λ��Ч
	SDIO->CMD |= ((WaitResp & 0x03) << 6); //������Ӧ
	SDIO->CMD |= 0 << 9;				   //�����ݵȴ��������
	SDIO->CMD |= 0 << 8;				   //�޵ȴ��ж�����
	SDIO->CMD |= 1 << 10;				   //����ͨ��״̬��(CPSM)ʹ��
}
/**********************************************************
* �������� ---> SDIO��������
* ��ڲ��� ---> DataTimeOut�����ݳ�ʱֵ
*               DataLEN�����ݳ��ȣ��Ĵ�����25λ��Ч������λ���������
*               BlockSize����Ĵ�С��Ҳ����2^BlockSize�ֽ�
*               Dir�����ݴ���ķ���0��SDIO���� --> ��
*                                    1���� --> SDIO����
* ������ֵ ---> none
* ����˵�� ---> none
**********************************************************/
static void SDIO_WriteRead_Data(uint32_t DataTimeOut, uint32_t DataLEN, uint8_t BlockSize, uint8_t Dir)
{
	SDIO->DTIMER = DataTimeOut;				  //�������ݶ�ʱ���Ĵ���
	SDIO->DLEN = (DataLEN & 0x01ffffff);	  //���ݳ��ȣ�������blocksize������������25λ��Ч
	SDIO->DCTRL &= 0xffffff08;				  //���֮ǰ������
	SDIO->DCTRL |= (uint8_t)(BlockSize << 4); //���ÿ��С
	SDIO->DCTRL |= 0 << 2;					  //�鴫������
	SDIO->DCTRL |= ((Dir & 0x01) << 1);		  //���ô��䷽��
	SDIO->DCTRL |= 1 << 0;					  //���ݴ���ʹ�ܣ�DPSM����ȴ��ź�״̬
}
/**********************************************************
* �������� ---> ����ʱ�ӷ�Ƶ
* ��ڲ��� ---> clkdiv��ʱ�ӷ�Ƶϵ��
* ������ֵ ---> none
* ����˵�� ---> none
**********************************************************/
static void SDIO_SetCLKDIV(uint8_t clkdiv)
{
	SDIO->CLKCR &= 0xffffff00; //�����Ƶϵ��
	SDIO->CLKCR |= clkdiv;	 //�����µķ�Ƶϵ��
}
/**********************************************************
* �������� ---> SD����ʼ��
* ��ڲ��� ---> none
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
SD_Error SD_Init(void)
{
	SD_Error errorstatus = SD_OK;

	MY_NVIC_Init(2, 2, SDIO_IRQn, NVIC_PriorityGroup_2); //SDIO�ж�����

	//�����ϵ����
	errorstatus = SD_PowerON();
	if (errorstatus != SD_OK)
		return (errorstatus); //����ʧ��
	//��ʼ����������״̬
	errorstatus = SD_InitializeCards();
	if (errorstatus != SD_OK)
		return (errorstatus); //����ʧ��
	/*!< Configure the SDIO peripheral */
	/*!< SDIOCLK = HCLK, SDIO_CK = HCLK/(2 + SDIO_TRANSFER_CLK_DIV) */
	/*!< on STM32F2xx devices, SDIOCLK is fixed to 48MHz */
	SDIO_SetCLKDIV(SDIO_TRANSFER_CLK_DIV); //��ߵ������ٶȣ�������������

	//�����������Ϣ���浽�ṹ����ȥ��CID��CSD��RCA��
	if (errorstatus == SD_OK)
		errorstatus = SD_GetCardInfo(&SDCardInfo);
	//ѡ�п�
	if (errorstatus == SD_OK)
		errorstatus = SD_SelectDeselect((uint32_t)(SDCardInfo.RCA << 16));
	//����4bģʽ
	if (errorstatus == SD_OK)
		errorstatus = SD_EnableWideBusOperation(SDIO_BusWide_4b);

	return (errorstatus);
}
/**********************************************************
* �������� ---> ȡ�ÿ��Ĵ���״̬
* ��ڲ��� ---> none
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
SDTransferState SD_GetStatus(void)
{
	SDCardState cardstate = SD_CARD_TRANSFER;
	cardstate = SD_GetState();

	if (cardstate == SD_CARD_TRANSFER)
		return (SD_TRANSFER_OK); //�������
	else if (cardstate == SD_CARD_ERROR)
		return (SD_TRANSFER_ERROR); //�������
	else
		return (SD_TRANSFER_BUSY); //æ����
}
/**********************************************************
* �������� ---> ȡ�ÿ��Ĵ���״ֵ̬
* ��ڲ��� ---> none
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
SDCardState SD_GetState(void)
{
	uint32_t resp1 = 0;

	if (SD_SendStatus(&resp1) != SD_OK)
		return (SD_CARD_ERROR);
	else
		return (SDCardState)((resp1 >> 9) & 0x0f);
}
/**********************************************************
* �������� ---> �����ϵ����
* ��ڲ��� ---> none
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
SD_Error SD_PowerON(void)
{
	SD_Error errorstatus = SD_OK;
	uint32_t response = 0, count = 0, validvoltage = 0;
	uint32_t SDType = SD_STD_CAPACITY;

	/*!< Power ON Sequence -----------------------------------------------------*/
	/*!< Configure the SDIO peripheral */
	/*!< SDIOCLK = HCLK, SDIO_CK = HCLK/(2 + SDIO_INIT_CLK_DIV) */
	/*!< on STM32F2xx devices, SDIOCLK is fixed to 48MHz */
	/*!< SDIO_CK for initialization should not exceed 400 KHz */
	SDIO->CLKCR = 0x00000000; //���֮ǰ������
	SDIO->CLKCR |= 0 << 9;	//�ر�ʡ��ģʽ
	SDIO->CLKCR |= 0 << 10;   //�ر���·��ʹ��CLKDIV�е���ֵ������SDIO_CKƵ��
	SDIO->CLKCR |= 0 << 11;   //һλ���ݿ��
	SDIO->CLKCR |= 0 << 13;   //����ʱ��SDIOCLK�������ز���SDIO_CK
	SDIO->CLKCR |= 0 << 14;   //�ر�Ӳ����
	SDIO->CLKCR |= 0xb2;	  //���ô����Ƶ��Ϊ400KHz�ĳ�ʼ���ٶ�
	SDIO->POWER = 0x03;		  //SDIO�ϵ�״̬������ʱ�ӿ�ʼ
	SDIO->CLKCR |= 1 << 8;	//ʹ��SDIO_CK

	//��������74��ʱ��
	for (count = 0; count < 74; count++)
	{
		/*!< CMD0: GO_IDLE_STATE ---------------------------------------------------*/
		/*!< No CMD response required */
		SDIO_Send_CMDIndex(SD_CMD_GO_IDLE_STATE, 0, SD_Response_No); //����CMD0 + ����Ӧ
		errorstatus = CmdError();
		if (errorstatus != SD_OK)
			return errorstatus; //����ʧ��
		else
			break; //�����ɹ����˳�
	}
	count = 0;

	/*!< CMD8: SEND_IF_COND ----------------------------------------------------*/
	/*!< Send CMD8 to verify SD card interface operating condition */
	/*!< Argument: - [31:12]: Reserved (shall be set to '0')
				   - [11:8]: Supply Voltage (VHS) 0x1 (Range: 2.7-3.6 V)
				   - [7:0]: Check Pattern (recommended 0xAA) */
	/*!< CMD Response: R7 */
	SDIO_Send_CMDIndex(SDIO_SEND_IF_COND, SD_CHECK_PATTERN, SD_Response_Short); //����CMD8 + ����Ӧ��R7Ӧ��
	errorstatus = CmdResp7Error();
	//�з�Ӧ˵����V2.0
	if (errorstatus == SD_OK)
	{
		CardType = SDIO_STD_CAPACITY_SD_CARD_V2_0; /*!< SD Card 2.0 */
		SDType = SD_HIGH_CAPACITY;
	}
	else
	{
		/*!< CMD55 */
		SDIO_Send_CMDIndex(SD_CMD_APP_CMD, 0x00, SD_Response_Short); //����CMD55 + ����Ӧ��R1Ӧ��
		errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
	}
	/*!< CMD55 */
	SDIO_Send_CMDIndex(SD_CMD_APP_CMD, 0x00, SD_Response_Short); //����CMD55 + ����Ӧ��R1Ӧ��
	errorstatus = CmdResp1Error(SD_CMD_APP_CMD);

	/*!< If errorstatus is Command TimeOut, it is a MMC card */
	/*!< If errorstatus is SD_OK it is a SD card: SD card 2.0 (voltage range mismatch)
		 or SD card 1.x */
	if (errorstatus == SD_OK)
	{
		/*!< SD CARD */
		/*!< Send ACMD41 SD_APP_OP_COND with Argument 0x80100000 */
		while ((!validvoltage) && (count < SD_MAX_VOLT_TRIAL))
		{
			/*!< SEND CMD55 APP_CMD with RCA as 0 */
			SDIO_Send_CMDIndex(SD_CMD_APP_CMD, 0x00, SD_Response_Short); //����CMD55 + ����Ӧ��R1Ӧ��
			errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
			if (errorstatus != SD_OK)
				return errorstatus; //����ʧ��

			//����ACMD41
			SDIO_Send_CMDIndex(SD_CMD_SD_APP_OP_COND, (SD_VOLTAGE_WINDOW_SD | SDType), SD_Response_Short); //����ACMD41 + ����Ӧ��R3Ӧ��
			errorstatus = CmdResp3Error();
			if (errorstatus != SD_OK)
				return errorstatus; //����ʧ��

			response = SDIO->RESP1; //�õ���Ӧֵ
			validvoltage = (((response >> 31) == 1) ? 1 : 0);
			count++;
		} //end while

		if (count >= SD_MAX_VOLT_TRIAL)
		{
			errorstatus = SD_INVALID_VOLTRANGE;
			return (errorstatus);
		}

		if (response &= SD_HIGH_CAPACITY)
		{
			CardType = SDIO_HIGH_CAPACITY_SD_CARD;
		}
	}	//enf if
	else //MMC card
	{
		CardType = SDIO_MULTIMEDIA_CARD;
		//MMC��,����CMD0 SDIO_SEND_OP_COND,����Ϊ:0x80FF8000
		while ((!validvoltage) && (count < SD_MAX_VOLT_TRIAL))
		{
			//����CMD1,����Ӧ
			SDIO_Send_CMDIndex(SD_CMD_SEND_OP_COND, SD_VOLTAGE_WINDOW_MMC, SD_Response_Short); //����CMD1 + ����Ӧ��R3Ӧ��
			errorstatus = CmdResp3Error();													   //�ȴ�R3��Ӧ
			if (errorstatus != SD_OK)
				return errorstatus; //����ʧ��

			response = SDIO->RESP1; //�õ���Ӧֵ
			validvoltage = (((response >> 31) == 1) ? 1 : 0);
			count++;
		} //end while
		if (count >= SD_MAX_VOLT_TRIAL)
		{
			errorstatus = SD_INVALID_VOLTRANGE;
			return (errorstatus);
		}
	} //end else
	return (errorstatus);
}
/**********************************************************
* �������� ---> SD���ص�
* ��ڲ��� ---> none
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
SD_Error SD_PowerOFF(void)
{
	SD_Error errorstatus = SD_OK;

	/*!< Set Power State to OFF */
	SDIO->POWER &= 0xfffffffc; //��Դ�رգ���ʱ��ֹͣ

	return (errorstatus);
}
/**********************************************************
* �������� ---> SD��ʼ��������״̬
* ��ڲ��� ---> none
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
SD_Error SD_InitializeCards(void)
{
	SD_Error errorstatus = SD_OK;
	uint16_t rca = 0x01;

	if ((SDIO->POWER & 0x00000003) == 0x00000000) //ȷ��SDIO��Դʱ�Ӵ�
	{
		errorstatus = SD_REQUEST_NOT_APPLICABLE;
		return (errorstatus);
	}

	if (SDIO_SECURE_DIGITAL_IO_CARD != CardType) //IO���������
	{
		/*!< Send CMD2 ALL_SEND_CID */
		SDIO_Send_CMDIndex(SD_CMD_ALL_SEND_CID, 0x0, SD_Response_Long); //����CMD2 + ����Ӧ��R2Ӧ��
		errorstatus = CmdResp2Error();
		if (errorstatus != SD_OK)
			return errorstatus; //����ʧ��
		//�õ�CID�Ĵ�����Ϣ
		CID_Tab[0] = SDIO->RESP1;
		CID_Tab[1] = SDIO->RESP2;
		CID_Tab[2] = SDIO->RESP3;
		CID_Tab[3] = SDIO->RESP4;
	}

	if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) || (SDIO_SECURE_DIGITAL_IO_COMBO_CARD == CardType) || (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
	{
		/*!< Send CMD3 SET_REL_ADDR with argument 0 */
		/*!< SD Card publishes its RCA. */
		SDIO_Send_CMDIndex(SD_CMD_SET_REL_ADDR, 0x00, SD_Response_Short); //����CMD3 + ����Ӧ��R6Ӧ��
		errorstatus = CmdResp6Error(SD_CMD_SET_REL_ADDR, &rca);			  //�õ�������Ե�ַ
		if (errorstatus != SD_OK)
			return errorstatus; //����ʧ��
	}

	if (SDIO_SECURE_DIGITAL_IO_CARD != CardType)
	{
		RCA = rca;

		/*!< Send CMD9 SEND_CSD with argument as card's RCA */
		SDIO_Send_CMDIndex(SD_CMD_SEND_CSD, (uint32_t)(rca << 16), SD_Response_Long); //����CMD9 + ����Ӧ��R2Ӧ��
		errorstatus = CmdResp2Error();
		if (errorstatus != SD_OK)
			return errorstatus; //����ʧ��
		//�õ�CSD�Ĵ�����Ϣ
		CSD_Tab[0] = SDIO->RESP1;
		CSD_Tab[1] = SDIO->RESP2;
		CSD_Tab[2] = SDIO->RESP3;
		CSD_Tab[3] = SDIO->RESP4;
	}
	errorstatus = SD_OK; //ȫ���������ɹ���
	return (errorstatus);
}
/**********************************************************
* �������� ---> ���濨����Ϣ����ؽṹ����
* ��ڲ��� ---> none
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
SD_Error SD_GetCardInfo(SD_CardInfo *cardinfo)
{
	SD_Error errorstatus = SD_OK;
	uint8_t tmp = 0;

	cardinfo->CardType = (uint8_t)CardType; //������
	cardinfo->RCA = (uint16_t)RCA;			//����Ե�ַ

	/******************************************************
					 ���ȶ�ȡCSD�Ĵ�����ֵ
	******************************************************/

	/* Byte 0 [ 127 ~ 120 ] */

	tmp = (uint8_t)((CSD_Tab[0] & 0xff000000) >> 24);
	cardinfo->SD_csd.CSDStruct = (tmp & 0xc0) >> 6; //CSD�Ĵ����ṹ ----> 2bits
	cardinfo->SD_csd.SysSpecVersion = (tmp & 0x3c) >> 2;
	cardinfo->SD_csd.Reserved1 = tmp & 0x03; //RESERVED      ----> 6bits

	/* Byte 1 [ 119 ~ 112 ] */

	tmp = (uint8_t)((CSD_Tab[0] & 0x00ff0000) >> 16); //data read access-time-1
	cardinfo->SD_csd.TAAC = tmp;					  //TAAC[Binary and MLC] ----> 8bits

	/* Byte 2 [ 111 ~ 104 ] */

	tmp = (uint8_t)((CSD_Tab[0] & 0x0000ff00) >> 8); //data read access-time-2
	cardinfo->SD_csd.NSAC = tmp;					 //NSAC                 ----> 8bits

	/* Byte 3 [ 103 ~ 96 ] */

	tmp = (uint8_t)(CSD_Tab[0] & 0x000000ff);
	cardinfo->SD_csd.MaxBusClkFrec = tmp; //TRAN SPEED ----> 8bits

	/* Byte 4 [ 95 ~ 88 ] */

	tmp = (uint8_t)((CSD_Tab[1] & 0xff000000) >> 24);
	cardinfo->SD_csd.CardComdClasses = tmp << 4;

	/* Byte 5 [ 87 ~ 80 ] */

	tmp = (uint8_t)((CSD_Tab[1] & 0x00ff0000) >> 16);
	cardinfo->SD_csd.CardComdClasses |= (tmp & 0xf0) >> 4; //CCC               ----> 12bits
	cardinfo->SD_csd.RdBlockLen = tmp & 0x0f;			   //----->READ_BL_LEN ----> 4bits

	/* Byte 6 [ 79 ~ 72 ] */

	tmp = (uint8_t)((CSD_Tab[1] & 0x0000ff00) >> 8);
	cardinfo->SD_csd.PartBlockRead = (tmp & 0x80) >> 7;   //READ_BL_PARTIAL      ----> 1bit
	cardinfo->SD_csd.WrBlockMisalign = (tmp & 0x40) >> 6; //WRITE_BLOCK_MISALIGN ----> 1bit
	cardinfo->SD_csd.RdBlockMisalign = (tmp & 0x20) >> 5; //READ_BLOCK_MISALIGN  ----> 1bit
	cardinfo->SD_csd.DSRImpl = (tmp & 0x10) >> 4;		  //DSR_IMP              ----> 1bit
	cardinfo->SD_csd.Reserved2 = 0;						  //RESERVED             ----> 2bits

	/******************************************************
			  ������ݲ�ͬ���͵Ŀ�������������
	******************************************************/

	/* standard V1.1��V2.0 and MMC card */
	if ((CardType == SDIO_STD_CAPACITY_SD_CARD_V1_1) || (CardType == SDIO_STD_CAPACITY_SD_CARD_V2_0) || (SDIO_MULTIMEDIA_CARD == CardType))
	{
		cardinfo->SD_csd.DeviceSize = (tmp & 0x03) << 10;

		/* Byte 7 [ 71 ~ 64 ] */

		tmp = (uint8_t)(CSD_Tab[1] & 0x000000ff);
		cardinfo->SD_csd.DeviceSize |= (tmp) << 2;

		/* Byte 8 [ 63 ~ 56 ] */

		tmp = (uint8_t)((CSD_Tab[2] & 0xff000000) >> 24);
		cardinfo->SD_csd.DeviceSize |= (tmp & 0xc0) >> 6; //----->C_SIZE   ----> 12bits

		cardinfo->SD_csd.MaxRdCurrentVDDMin = (tmp & 0x38) >> 3; //VDD_R_CURR_MIN ----> 3bits
		cardinfo->SD_csd.MaxRdCurrentVDDMax = (tmp & 0x07);		 //VDD_R_CURR_MAX ----> 3bits

		/* Byte 9 [ 55 ~ 48 ] */

		tmp = (uint8_t)((CSD_Tab[2] & 0x00ff0000) >> 16);
		cardinfo->SD_csd.MaxWrCurrentVDDMin = (tmp & 0xe0) >> 5; //VDD_W_CURR_MIN ----> 3bits
		cardinfo->SD_csd.MaxWrCurrentVDDMax = (tmp & 0x1c) >> 2; //VDD_W_CURR_MAX ----> 3bits

		cardinfo->SD_csd.DeviceSizeMul = (tmp & 0x03) << 1;

		/* Byte 10 [ 47 ~ 40 ] */

		tmp = (uint8_t)((CSD_Tab[2] & 0x0000ff00) >> 8);
		cardinfo->SD_csd.DeviceSizeMul |= (tmp & 0x80) >> 7; //C_SIZE_MULT

		/* ���������� */
		//���㹫ʽ: memory capacity = BLOCKNR * BLOCK_LEN
		//                  BLOCKNR = (C_SIZE + 1) * MULT
		//                     MULT = 2 ^ (C_SIZE_MULT + 2) ---> C_SIZE_MULT < 8
		//                BLOCK_LEN = 2 ^ READ_BL_LEN  ---> READ_BL_LEN < 12

		cardinfo->CardCapacity = (cardinfo->SD_csd.DeviceSize + 1);
		cardinfo->CardCapacity *= (1 << (cardinfo->SD_csd.DeviceSizeMul + 2));
		cardinfo->CardBlockSize = 1 << (cardinfo->SD_csd.RdBlockLen); //�����С
		cardinfo->CardCapacity *= cardinfo->CardBlockSize;			  //������
		cardinfo->CardCapacity = cardinfo->CardCapacity >> 20;		  //��λMB

	} //end standard V1.1��V2.0 and MMC card
	/* High Capacity Card */
	else if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
	{
		/* Byte 7 [ 71 ~ 64 ] */

		tmp = (uint8_t)(CSD_Tab[1] & 0x000000ff);
		cardinfo->SD_csd.DeviceSize = (tmp & 0x3f) << 16;

		/* Byte 8 [ 63 ~ 56 ] */

		tmp = (uint8_t)((CSD_Tab[2] & 0xff000000) >> 24);
		cardinfo->SD_csd.DeviceSize |= (tmp << 8);

		/* Byte 9 [ 55 ~ 48 ] */

		tmp = (uint8_t)((CSD_Tab[2] & 0x00ff0000) >> 16);
		cardinfo->SD_csd.DeviceSize |= tmp; //C_SIZE ----> 24bits

		/* Byte 10 [ 47 ~ 40 ] */

		tmp = (uint8_t)((CSD_Tab[2] & 0x0000ff00) >> 8);

		/* ���������� */
		//���㹫ʽ: memory capacity = (C_SIZE + 1) * 512byte

		cardinfo->CardCapacity = (long long)(cardinfo->SD_csd.DeviceSize + 1) * 512 * 1024; //������
		cardinfo->CardBlockSize = 512;														//�����С�̶�Ϊ512byte
		cardinfo->CardCapacity = cardinfo->CardCapacity >> 20;								//��λMB

	} //end High Capacity Card
	/******************************************************
						��������������
	******************************************************/

	cardinfo->SD_csd.EraseGrSize = (tmp & 0x40) >> 6; //ERASE_BLOCK_EN ----> 1bit
	cardinfo->SD_csd.EraseGrMul = (tmp & 0x3f) << 1;

	/* Byte 11 [ 39 ~ 32 ] */

	tmp = (uint8_t)(CSD_Tab[2] & 0x000000ff);
	cardinfo->SD_csd.EraseGrMul |= (tmp & 0x80) >> 7; //ERASE_GROUP_MUL ----> 8bits
	cardinfo->SD_csd.WrProtectGrSize = (tmp & 0x7f);  //WP_GRP_SIZE     ----> 7bits

	/* Byte 12 [ 31 ~ 24 ] */

	tmp = (uint8_t)((CSD_Tab[3] & 0xff000000) >> 24);
	cardinfo->SD_csd.WrProtectGrEnable = (tmp & 0x80) >> 7; //WP_GRP_ENABLE      ----> 1bit
	cardinfo->SD_csd.ManDeflECC = (tmp & 0x60) >> 5;		//RMCC               ----> 2bits
	cardinfo->SD_csd.WrSpeedFact = (tmp & 0x1c) >> 2;		//write speed factor ----> 3bits
	cardinfo->SD_csd.MaxWrBlockLen = (tmp & 0x03);

	/* Byte 13 [ 23 ~ 16 ] */

	tmp = (uint8_t)((CSD_Tab[3] & 0x00ff0000) >> 16);
	cardinfo->SD_csd.MaxWrBlockLen |= (tmp & 0xc0) >> 6;	  //WRITE_BLOCK_LEN.MAX ----> 4bits
	cardinfo->SD_csd.WriteBlockPaPartial = (tmp & 0x20) >> 5; //WRITE_BLOCK_PARTIAL ----> 1bit
	cardinfo->SD_csd.Reserved3 = 0;							  //RESEVED             ----> 5bits
	cardinfo->SD_csd.ContentProtectAppli = (tmp & 0x01);

	/* Byte 14 [ 15 ~ 8 ] */

	tmp = (uint8_t)((CSD_Tab[3] & 0x0000ff00) >> 8);
	cardinfo->SD_csd.FileFormatGrouop = (tmp & 0x80) >> 7; //FILE_FORMAT_GRP    ----> 1bit
	cardinfo->SD_csd.CopyFlag = (tmp & 0x40) >> 6;		   //COPY_FLAG          ----> 1bit
	cardinfo->SD_csd.PermWrProtect = (tmp & 0x20) >> 5;	//PERM_WRITE_PROTECT ----> 1bit
	cardinfo->SD_csd.TempWrProtect = (tmp & 0x10) >> 4;	//TMP_WRITE_PROTECT  ----> 1bit
	cardinfo->SD_csd.FileFormat = (tmp & 0x0c) >> 2;	   //FILE_FORMAT        ----> 2bits
	cardinfo->SD_csd.ECC = (tmp & 0x03);				   //ECC                ----> 2bits

	/* Byte 15 [ 7 ~ 0 ] */

	tmp = (uint8_t)(CSD_Tab[3] & 0x000000ff);
	cardinfo->SD_csd.CSD_CRC = (tmp & 0xfe) >> 1; //CSD_CRC  ----> 7bits
	cardinfo->SD_csd.Reserved4 = 1;				  //always 1 ----> 1bit

	/******************************************************
					 ����CSD�Ĵ�����ֵ��ȡ
	******************************************************/

	/******************************************************
					 ��ζ�ȡCID�Ĵ�����ֵ
	******************************************************/

	/* Byte 0 [ 127 ~ 120 ] */

	tmp = (uint8_t)((CID_Tab[0] & 0xff000000) >> 24);
	cardinfo->SD_cid.ManufacturerID = tmp; //MID ----> 8bits

	/* Byte 1 [ 119 ~ 112 ] */

	tmp = (uint8_t)((CID_Tab[0] & 0x00ff0000) >> 16);
	cardinfo->SD_cid.OEM_AppliID = tmp << 8;

	/* Byte 2 [ 111 ~ 104 ] */

	tmp = (uint8_t)((CID_Tab[0] & 0x0000ff00) >> 8);
	cardinfo->SD_cid.OEM_AppliID |= tmp; //OEM/APPLICATION ID ----> 16bits

	/* Byte 3 [ 103 ~ 96 ] */

	tmp = (uint8_t)(CID_Tab[0] & 0x000000ff);
	cardinfo->SD_cid.ProdName1 = (tmp << 24); //PRODUCT NAME = PRODUCT NAME1 + PRODUCT NAME2 ----> 40bits

	/* Byte 4 [ 95 ~ 88 ] */

	tmp = (uint8_t)((CID_Tab[1] & 0xff000000) >> 24);
	cardinfo->SD_cid.ProdName1 |= (tmp << 16);

	/* Byte 5 [ 87 ~ 80 ] */

	tmp = (uint8_t)((CID_Tab[1] & 0x00ff0000) >> 16);
	cardinfo->SD_cid.ProdName1 |= (tmp << 8);

	/* Byte 6 [ 79 ~ 72 ] */

	tmp = (uint8_t)((CID_Tab[1] & 0x0000ff00) >> 8);
	cardinfo->SD_cid.ProdName1 |= tmp; //PRODUCT NAME1 ----> 32bits

	/* Byte 7 [ 71 ~ 64 ] */

	tmp = (uint8_t)(CID_Tab[1] & 0x000000ff);
	cardinfo->SD_cid.ProdName2 = tmp; //PRODUCT NAME2 ----> 8bits

	/* Byte 8 [ 63 ~ 56 ] */

	tmp = (uint8_t)((CID_Tab[2] & 0xff000000) >> 24);
	cardinfo->SD_cid.ProdRev = tmp; //PRODUCT REVISION ----> 8bits

	/* Byte 9 [ 55 ~ 48 ] */

	tmp = (uint8_t)((CID_Tab[2] & 0x00ff0000) >> 16);
	cardinfo->SD_cid.ProdSN = (tmp << 24);

	/* Byte 10 [ 47 ~ 40 ] */

	tmp = (uint8_t)((CID_Tab[2] & 0x0000ff00) >> 8);
	cardinfo->SD_cid.ProdSN |= (tmp << 16);

	/* Byte 11 [ 39 ~ 32 ] */

	tmp = (uint8_t)(CID_Tab[2] & 0x000000ff);
	cardinfo->SD_cid.ProdSN |= (tmp << 8);

	/* Byte 12 [ 31 ~ 24 ] */

	tmp = (uint8_t)((CID_Tab[3] & 0xff000000) >> 24);
	cardinfo->SD_cid.ProdSN |= tmp; //PRODUCT SERIAL NUMBER ----> 32bits

	/* Byte 13 [ 23 ~ 16 ] */

	tmp = (uint8_t)((CID_Tab[3] & 0x00ff0000) >> 16);
	cardinfo->SD_cid.Reserved1 = (tmp & 0xf0) >> 4; //RESEVED ----> 4bits
	cardinfo->SD_cid.ManufactDate = (tmp & 0x0f) << 8;

	/* Byte 14 [ 15 ~ 8 ] */

	tmp = (uint8_t)((CID_Tab[3] & 0x0000ff00) >> 8);
	cardinfo->SD_cid.ManufactDate |= tmp; //MDT ----> 12bits

	/* Byte 15 [ 7 ~ 0 ] */

	tmp = (uint8_t)(CID_Tab[3] & 0x000000ff);
	cardinfo->SD_cid.CID_CRC = (tmp & 0xfe) >> 1; //CID_CRC  ----> 7bits
	cardinfo->SD_cid.Reserved2 = 1;				  //always 1 ----> 1bit

	/******************************************************
					 ����CID�Ĵ�����ֵ��ȡ
	******************************************************/

	return (errorstatus); //����Ӧ����Ϣ����������ж�
}
/**********************************************************
* �������� ---> ��ȡ��״̬��Ϣ�����浽�ṹ����
* ��ڲ��� ---> none
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
SD_Error SD_GetCardStatus(SD_CardStatus *cardstatus)
{
	SD_Error errorstatus = SD_OK;
	uint8_t tmp = 0;

	errorstatus = SD_SendSDStatus((uint32_t *)SDSTATUS_Tab);
	if (errorstatus != SD_OK)
		return errorstatus; //����ʧ��

	/*!< Byte 0 */
	tmp = (uint8_t)((SDSTATUS_Tab[0] & 0xc0) >> 6);
	cardstatus->DAT_BUS_WIDTH = tmp;

	/*!< Byte 0 */
	tmp = (uint8_t)((SDSTATUS_Tab[0] & 0x20) >> 5);
	cardstatus->SECURED_MODE = tmp;

	/*!< Byte 2 */
	tmp = (uint8_t)((SDSTATUS_Tab[2] & 0xff));
	cardstatus->SD_CARD_TYPE = tmp << 8;

	/*!< Byte 3 */
	tmp = (uint8_t)((SDSTATUS_Tab[3] & 0xff));
	cardstatus->SD_CARD_TYPE |= tmp;

	/*!< Byte 4 */
	tmp = (uint8_t)(SDSTATUS_Tab[4] & 0xff);
	cardstatus->SIZE_OF_PROTECTED_AREA = tmp << 24;

	/*!< Byte 5 */
	tmp = (uint8_t)(SDSTATUS_Tab[5] & 0xff);
	cardstatus->SIZE_OF_PROTECTED_AREA |= tmp << 16;

	/*!< Byte 6 */
	tmp = (uint8_t)(SDSTATUS_Tab[6] & 0xff);
	cardstatus->SIZE_OF_PROTECTED_AREA |= tmp << 8;

	/*!< Byte 7 */
	tmp = (uint8_t)(SDSTATUS_Tab[7] & 0xff);
	cardstatus->SIZE_OF_PROTECTED_AREA |= tmp;

	/*!< Byte 8 */
	tmp = (uint8_t)((SDSTATUS_Tab[8] & 0xff));
	cardstatus->SPEED_CLASS = tmp;

	/*!< Byte 9 */
	tmp = (uint8_t)((SDSTATUS_Tab[9] & 0xff));
	cardstatus->PERFORMANCE_MOVE = tmp;

	/*!< Byte 10 */
	tmp = (uint8_t)((SDSTATUS_Tab[10] & 0xf0) >> 4);
	cardstatus->AU_SIZE = tmp;

	/*!< Byte 11 */
	tmp = (uint8_t)(SDSTATUS_Tab[11] & 0xff);
	cardstatus->ERASE_SIZE = tmp << 8;

	/*!< Byte 12 */
	tmp = (uint8_t)(SDSTATUS_Tab[12] & 0xff);
	cardstatus->ERASE_SIZE |= tmp;

	/*!< Byte 13 */
	tmp = (uint8_t)((SDSTATUS_Tab[13] & 0xfc) >> 2);
	cardstatus->ERASE_TIMEOUT = tmp;

	/*!< Byte 13 */
	tmp = (uint8_t)((SDSTATUS_Tab[13] & 0x3));
	cardstatus->ERASE_OFFSET = tmp;

	return (errorstatus);
}
/**********************************************************
* �������� ---> ����������ģʽ
* ��ڲ��� ---> WideMode������ģʽ����
*               SDIO_BusWide_8b: 8-bit data transfer (Only for MMC)
*               SDIO_BusWide_4b: 4-bit data transfer
*               SDIO_BusWide_1b: 1-bit data transfer
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
SD_Error SD_EnableWideBusOperation(uint32_t WideMode)
{
	SD_Error errorstatus = SD_OK;

	/*!< MMC Card doesn't support this feature */
	if (SDIO_MULTIMEDIA_CARD == CardType)
	{
		errorstatus = SD_UNSUPPORTED_FEATURE;
		return (errorstatus);
	}

	else if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) || (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
	{
		if (SDIO_BusWide_8b == WideMode)
		{
			errorstatus = SD_UNSUPPORTED_FEATURE;
			return (errorstatus);
		}
		else if (SDIO_BusWide_4b == WideMode)
		{
			errorstatus = SDEnWideBus(ENABLE);
			if (errorstatus == SD_OK)
			{
				SDIO->CLKCR &= 0xffffe700;			  //���WIDBUS��CLKDIVλ
				SDIO->CLKCR |= 1 << 11;				  //����4bits�߿�
				SDIO->CLKCR |= SDIO_TRANSFER_CLK_DIV; //����ٶ�
				SDIO->CLKCR |= 0 << 14;				  //�ر�Ӳ��������
			}
		}
		else //����ٶȣ�����1b���߿��
		{
			errorstatus = SDEnWideBus(DISABLE);
			if (errorstatus == SD_OK)
			{
				SDIO->CLKCR &= 0xffffe700;			  //���WIDBUS��CLKDIVλ
				SDIO->CLKCR |= 0 << 11;				  //����1bit�߿�
				SDIO->CLKCR |= SDIO_TRANSFER_CLK_DIV; //����ٶ�
				SDIO->CLKCR |= 0 << 14;				  //�ر�Ӳ��������
			}
		}
	}
	return (errorstatus);
}
/**********************************************************
* �������� ---> ѡ�л���ȡ��������
* ��ڲ��� ---> addr��������Ե�ַ
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
SD_Error SD_SelectDeselect(uint32_t addr)
{
	SD_Error errorstatus = SD_OK;

	/*!< Send CMD7 SDIO_SEL_DESEL_CARD */
	SDIO_Send_CMDIndex(SD_CMD_SEL_DESEL_CARD, addr, SD_Response_Short); //����CMD7 + ����Ӧ��R1Ӧ��
	errorstatus = CmdResp1Error(SD_CMD_SEL_DESEL_CARD);

	return (errorstatus);
}
/**********************************************************
* �������� ---> ��ȡһ��block����
* ��ڲ��� ---> *readbuff����ȡ�����ݻ���
*               ReadAddr����ȡ������ʼ��ַ
*               BlockSize����ȡblock��С
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
SD_Error SD_ReadBlock(uint8_t *readbuff, uint32_t ReadAddr, uint16_t BlockSize)
{
	SD_Error errorstatus = SD_OK;
	uint8_t power = 0;

#if defined(SD_POLLING_MODE)

	uint32_t count = 0, *tempbuff = (uint32_t *)readbuff; //��ѯģʽ�õ�����

#endif

	TransferError = SD_OK;
	TransferEnd = 0;
	StopCondition = 0; //�����ȡ����Ҫ����ֹͣ����

	SDIO->DCTRL = 0x0; //���ݿ��ƼĴ������㣬�ر�DMA

	if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
	{
		BlockSize = 512;
		ReadAddr /= 512;
	}

	//���CPSM״̬��
	SDIO_WriteRead_Data(SD_DATATIMEOUT, 0, 0, SDIO_To_Card);

	if (SDIO->RESP1 & SD_CARD_LOCKED)
		return SD_LOCK_UNLOCK_FAILED; //������

	/* ���ÿ��Ŀ��С */
	/* ��Ҫ�Ƿ���CMD16����ȥ���� */
	/* SDSC�����������ÿ�Ĵ�С */
	/* SDHC������Ĵ�С��Ϊ512byte������CMD16Ӱ�� */
	if ((BlockSize > 0) && (BlockSize <= 2048) && ((BlockSize & (BlockSize - 1)) == 0))
	{
		power = convert_from_bytes_to_power_of_two(BlockSize);
		//����CMD16+�������ݳ���Ϊblksize,����Ӧ
		SDIO_Send_CMDIndex(SD_CMD_SET_BLOCKLEN, (uint32_t)BlockSize, SD_Response_Short); //����CMD16 + ����Ӧ��R1Ӧ��
		errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);								 //�ȴ�R1��Ӧ
		if (errorstatus != SD_OK)
			return errorstatus; //����ʧ��
	}
	else
		return (SD_INVALID_PARAMETER);
	/******************************************************
					   ���������С����
	******************************************************/

	//����SDIO��ؼĴ���
	SDIO_WriteRead_Data(SD_DATATIMEOUT, BlockSize, power, Card_To_SDIO); //power������д9Ҳ��

	/*!< Send CMD17 READ_SINGLE_BLOCK */
	SDIO_Send_CMDIndex(SD_CMD_READ_SINGLE_BLOCK, (uint32_t)ReadAddr, SD_Response_Short); //����CMD17 + ����Ӧ��R1Ӧ��
	errorstatus = CmdResp1Error(SD_CMD_READ_SINGLE_BLOCK);
	if (errorstatus != SD_OK)
		return errorstatus; //����ʧ��

#if defined(SD_POLLING_MODE) //��ѯģʽ

	/*!< In case of single block transfer, no need of stop transfer at all.*/
	/*!< Polling mode */
	while (!(SDIO->STA & ((1 << 5) | (1 << 1) | (1 << 3) | (1 << 10) | (1 << 9))))
	{ //  RXOVERR  DCRCFAIL DTIMEOUT  DBCKEND   STBITERR
		if (SDIO->STA & (1 << 15))
		{
			for (count = 0; count < 8; count++)
			{
				*(tempbuff + count) = SDIO_ReadData();
			}
			tempbuff += 8;
		}
	}

	if (SDIO->STA & (1 << 3)) //���ݳ�ʱ
	{
		SDIO->ICR = 1 << 3; //�����־
		errorstatus = SD_DATA_TIMEOUT;
		return (errorstatus);
	}
	else if (SDIO->STA & (1 << 1)) //CRCУ�����
	{
		SDIO->ICR = 1 << 1; //�����־
		errorstatus = SD_DATA_CRC_FAIL;
		return (errorstatus);
	}
	else if (SDIO->STA & (1 << 5)) //����FIFO�������
	{
		SDIO->ICR = 1 << 5; //�����־
		errorstatus = SD_RX_OVERRUN;
		return (errorstatus);
	}
	else if (SDIO->STA & (1 << 9)) //��ʼλ����
	{
		SDIO->ICR = 1 << 9; //�����־
		errorstatus = SD_START_BIT_ERR;
		return (errorstatus);
	}
	while (SDIO->STA & (1 << 21)) //�ڽ���FIFO�е����ݿ���
	{
		*tempbuff = SDIO->FIFO;
		tempbuff++;
	}

	/*!< Clear all the static flags */
	SDIO->ICR = SDIO_STATIC_FLAGS //������б�־

#elif defined(SD_DMA_MODE) //DMA����ģʽ

	SDIO->MASK |= ((1 << 5) | (1 << 3) | (1 << 1) | (1 << 8)); //��������жϱ�־λ
	//             RXOVERR DTIMEOUT DCRCFAIL DATAEND
	SDIO->DCTRL |= 1 << 3;											   //ʹ��DMA����
	SDIO_DMA_SendData((uint32_t *)readbuff, BlockSize, DMA2_ReadDATA); //DMA��ʼ��������

	SD_WaitReadOperation(); //ѭ����ѯDMA�����Ƿ����
	while (SD_GetStatus() != SD_TRANSFER_OK)
		;

#endif

		return (errorstatus);
}
/**********************************************************
* �������� ---> ��ȡ���block����
* ��ڲ��� ---> *readbuff����ȡ�����ݻ���
*               ReadAddr����ȡ������ʼ��ַ
*               BlockSize����ȡblock��С
*               NumberOfBlocks����ȡ�����Ŀ
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
SD_Error SD_ReadMultiBlocks(uint8_t *readbuff, uint32_t ReadAddr, uint16_t BlockSize, uint32_t NumberOfBlocks)
{
	SD_Error errorstatus = SD_OK;
	uint32_t timeout = 0;
	uint8_t power = 0;

	TransferError = SD_OK;
	TransferEnd = 0;
	StopCondition = 1; //��ȡ�������Ҫ����ֹͣ����

	SDIO->DCTRL = 0x0; //���ݿ��ƼĴ������㣬�ر�DMA

	if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
	{
		BlockSize = 512;
		ReadAddr /= 512;
	}

	//���CPSM״̬��
	SDIO_WriteRead_Data(SD_DATATIMEOUT, 0, 0, SDIO_TransferDir_ToCard);

	if (SDIO->RESP1 & SD_CARD_LOCKED)
		return SD_LOCK_UNLOCK_FAILED; //������

	/* ���ÿ��Ŀ��С */
	/* ��Ҫ�Ƿ���CMD16����ȥ���� */
	/* SDSC�����������ÿ�Ĵ�С */
	/* SDHC������Ĵ�С��λ512byte������CMD16Ӱ�� */
	if ((BlockSize > 0) && (BlockSize <= 2048) && ((BlockSize & (BlockSize - 1)) == 0))
	{
		power = convert_from_bytes_to_power_of_two(BlockSize);
		//����CMD16+�������ݳ���Ϊblksize,����Ӧ
		SDIO_Send_CMDIndex(SD_CMD_SET_BLOCKLEN, (uint32_t)BlockSize, SD_Response_Short); //����CMD16 + ����Ӧ��R1Ӧ��
		errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);								 //�ȴ�R1��Ӧ
		if (errorstatus != SD_OK)
			return errorstatus; //����ʧ��
	}
	else
		return (SD_INVALID_PARAMETER);
	/******************************************************
					   ���������С����
	******************************************************/

	/* �ж��Ƿ񳬹����յ���󻺳� */
	if (NumberOfBlocks * BlockSize > SD_MAX_DATA_LENGTH)
	{
		errorstatus = SD_INVALID_PARAMETER; //�����ˣ����ش�����Ϣ
		return errorstatus;
	}
	//û����
	SDIO_WriteRead_Data(SD_DATATIMEOUT, (NumberOfBlocks * BlockSize), power, Card_To_SDIO); //power������д9Ҳ��

	/*!< Send CMD18 READ_MULT_BLOCK with argument data address */
	SDIO_Send_CMDIndex(SD_CMD_READ_MULT_BLOCK, (uint32_t)ReadAddr, SD_Response_Short); //����CMD18 + ����Ӧ��R1Ӧ��
	errorstatus = CmdResp1Error(SD_CMD_READ_MULT_BLOCK);
	if (errorstatus != SD_OK)
		return errorstatus; //����ʧ��

	//�ȴ�DMA�������
	SDIO->MASK |= ((1 << 1) | (1 << 3) | (1 << 8) | (1 << 5) | (1 << 9)); //������Ҫ���ж�
	//             DCRCFAIL DTIMEOUT DATAEND RXOVERR  STBITERR
	SDIO->DCTRL |= 1 << 3;																  //ʹ��DMA����
	SDIO_DMA_SendData((uint32_t *)readbuff, (NumberOfBlocks * BlockSize), DMA2_ReadDATA); //DMA��ʼ��������

	timeout = SDIO_DATATIMEOUT;
	while (((DMA2->ISR & 0x2000) == RESET) && timeout)
		timeout--; //�ȴ��������

	if (timeout == 0)
		return SD_DATA_TIMEOUT; //��ʱ
	while ((TransferEnd == 0) && (TransferError == SD_OK))
		;
	if (TransferError != SD_OK)
		errorstatus = TransferError;

	return (errorstatus);
}
/**********************************************************
* �������� ---> дһ��block����
* ��ڲ��� ---> *writebuff��д�����ݻ���
*               WriteAddr��д��������ʼ��ַ
*               BlockSize��дblock��С
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
SD_Error SD_WriteBlock(uint8_t *writebuff, uint32_t WriteAddr, uint16_t BlockSize)
{
	SD_Error errorstatus = SD_OK;
	uint32_t timeout = 0;
	uint32_t cardstatus = 0;
	uint8_t cardstate = 0;
	uint8_t power = 0;

#if defined(SD_POLLING_MODE)

	uint32_t bytestransferred = 0, count = 0, restwords = 0;
	uint32_t *tempbuff = (uint32_t *)writebuff;
	uint32_t Datalen = BlockSize; //�ܳ���(�ֽ�)

#endif

	TransferError = SD_OK;
	TransferEnd = 0;
	StopCondition = 0; //����д����Ҫ����ֹͣ����

	if (writebuff == NULL)
		return SD_INVALID_PARAMETER; //��������

	SDIO->DCTRL = 0x0; //���ݿ��ƼĴ������㣬�ر�DMA

	//���CPSM״̬��
	SDIO_WriteRead_Data(SD_DATATIMEOUT, 0, 0, SDIO_To_Card);

	if (SDIO->RESP1 & SD_CARD_LOCKED)
		return SD_LOCK_UNLOCK_FAILED; //������

	if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
	{
		BlockSize = 512;
		WriteAddr /= 512;
	}

	/* ���ÿ��Ŀ��С */
	/* ��Ҫ�Ƿ���CMD16����ȥ���� */
	/* SDSC�����������ÿ�Ĵ�С */
	/* SDHC������Ĵ�С��λ512byte������CMD16Ӱ�� */
	if ((BlockSize > 0) && (BlockSize <= 2048) && ((BlockSize & (BlockSize - 1)) == 0))
	{
		power = convert_from_bytes_to_power_of_two(BlockSize);
		//����CMD16+�������ݳ���Ϊblksize,����Ӧ
		SDIO_Send_CMDIndex(SD_CMD_SET_BLOCKLEN, (uint32_t)BlockSize, SD_Response_Short); //����CMD16 + ����Ӧ��R1Ӧ��
		errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);								 //�ȴ�R1��Ӧ
		if (errorstatus != SD_OK)
			return errorstatus; //����ʧ��
	}
	else
		return (SD_INVALID_PARAMETER);
	/******************************************************
					   ���������С����
	******************************************************/

	/*����CMD13��ѯ����״̬*/
	SDIO_Send_CMDIndex(SD_CMD_SEND_STATUS, (uint32_t)RCA << 16, SD_Response_Short); //����CMD13 + ����Ӧ��R1Ӧ��
	errorstatus = CmdResp1Error(SD_CMD_SEND_STATUS);
	if (errorstatus != SD_OK)
		return errorstatus; //����ʧ��

	cardstatus = SDIO->RESP1;
	timeout = SD_DATATIMEOUT;
	while (((cardstatus & 0x00000100) == 0) && (timeout > 0)) //���READY_FOR_DATAλ�Ƿ���λ
	{
		timeout--;
		//����CMD13,��ѯ����״̬,����Ӧ
		SDIO_Send_CMDIndex(SD_CMD_SEND_STATUS, (uint32_t)RCA << 16, SD_Response_Short); //����CMD13 + ����Ӧ��R1Ӧ��
		errorstatus = CmdResp1Error(SD_CMD_SEND_STATUS);								//�ȴ�R1��Ӧ
		if (errorstatus != SD_OK)
			return errorstatus; //����ʧ��
		cardstatus = SDIO->RESP1;
	}
	if (timeout == 0)
		return SD_ERROR;

	/*!< Send CMD24 WRITE_SINGLE_BLOCK */
	SDIO_Send_CMDIndex(SD_CMD_WRITE_SINGLE_BLOCK, WriteAddr, SD_Response_Short); //����CMD24 + ����Ӧ��R1Ӧ��
	errorstatus = CmdResp1Error(SD_CMD_WRITE_SINGLE_BLOCK);
	if (errorstatus != SD_OK)
		return errorstatus; //����ʧ��

	StopCondition = 0; //����д,����Ҫ����ֹͣ����ָ��

	SDIO_WriteRead_Data(SD_DATATIMEOUT, BlockSize, power, SDIO_To_Card); //power������д9Ҳ��

	/*!< In case of single data block transfer no need of stop command at all */
#if defined(SD_POLLING_MODE)

	while (!(SDIO->STA & ((1 << 10) | (1 << 4) | (1 << 1) | (1 << 3) | (1 << 9))))
	{							   //               DBCKEND  TXUNDERR DCRCFAIL DTIMEOUT STBITERR
		if (SDIO->STA & (1 << 14)) //����FIFO��գ�FIFO�����ٻ�����д��8����
		{
			if ((Datalen - bytestransferred) < 32)
			{
				restwords = ((Datalen - bytestransferred) % 4 == 0) ? ((Datalen - bytestransferred) / 4) : ((Datalen - bytestransferred) / 4 + 1);
				for (count = 0; count < restwords; count++, tempbuff++, bytestransferred += 4)
				{
					SDIO->FIFO = *tempbuff;
				}
			}
			else
			{
				for (count = 0; count < 8; count++)
				{
					SDIO->FIFO = *(tempbuff + count);
				}
				tempbuff += 8;
				bytestransferred += 32;
			}
		}
	}

	if (SDIO->STA & (1 << 3)) //���ݳ�ʱ
	{
		SDIO->ICR = 1 << 3; //�����־
		errorstatus = SD_DATA_TIMEOUT;
		return (errorstatus);
	}
	else if (SDIO->STA & (1 << 1)) //CRCУ�����
	{
		SDIO->ICR = 1 << 1; //�����־
		errorstatus = SD_DATA_CRC_FAIL;
		return (errorstatus);
	}
	else if (SDIO->STA & (1 << 5)) //����FIFO�������
	{
		SDIO->ICR = 1 << 5; //�����־
		errorstatus = SD_TX_UNDERRUN;
		return (errorstatus);
	}
	else if (SDIO->STA & (1 << 9)) //��ʼλ����
	{
		SDIO->ICR = 1 << 9; //�����־
		errorstatus = SD_START_BIT_ERR;
		return (errorstatus);
	}

#elif defined(SD_DMA_MODE)

	TransferError = SD_OK;
	TransferEnd = 0;   //�����������λ�����жϷ�����1
	StopCondition = 0; //����д,����Ҫ����ֹͣ����ָ��

	SDIO->MASK |= ((1 << 4) | (1 << 3) | (1 << 1) | (1 << 8) | (1 << 9)); //��������жϱ�־λ
	//            TXUNDERR DTIMEOUT DCRCFAIL DATAEND  STBITERR

	SDIO_DMA_SendData((uint32_t *)writebuff, BlockSize, DMA2_WriteDATA); //DMA��ʼд������
	SDIO->DCTRL |= 1 << 3;												 //ʹ��DMA����
																		 /*
	timeout = SDIO_DATATIMEOUT;
	while(((DMA2->ISR&0x2000)==RESET)&&timeout)	timeout--;//�ȴ��������
	if(timeout==0)
	{
		SD_Init();	 					//���³�ʼ��SD��,���Խ��д������������
		return SD_DATA_TIMEOUT;			//��ʱ
	}
	timeout = SDIO_DATATIMEOUT;
	while((TransferEnd==0)&&(TransferError==SD_OK)&&timeout)timeout--;
	if(timeout==0)return SD_DATA_TIMEOUT;			//��ʱ
	if(TransferError!=SD_OK)return TransferError;
*/
	SD_WaitWriteOperation();											 //ѭ����ѯdma�����Ƿ����
	while (SD_GetStatus() != SD_TRANSFER_OK)
		; //Wait until end of DMA transfer

#endif

	SDIO->ICR = SDIO_STATIC_FLAGS; //������б��
	errorstatus = IsCardProgramming(&cardstate);
	while ((errorstatus == SD_OK) && ((cardstate == SD_CARD_PROGRAMMING) || (cardstate == SD_CARD_RECEIVING)))
	{
		errorstatus = IsCardProgramming(&cardstate);
	}

	return (errorstatus);
}
/**********************************************************
* �������� ---> д���block����
* ��ڲ��� ---> *writebuff��д�����ݻ���
*               WriteAddr��д��������ʼ��ַ
*               BlockSize��дblock��С
*               NumberOfBlocks��д�����Ŀ
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
SD_Error SD_WriteMultiBlocks(uint8_t *writebuff, uint32_t WriteAddr, uint16_t BlockSize, uint32_t NumberOfBlocks)
{
	SD_Error errorstatus = SD_OK;
	__IO uint32_t count = 0;
	__IO uint32_t timeout = 0;
	uint8_t cardstate = 0;
	uint8_t power = 0;

	TransferError = SD_OK;
	TransferEnd = 0;
	StopCondition = 1; //�����д�������Ҫ����ֹͣ����

	SDIO->DCTRL = 0x0; //���ݿ��ƼĴ������㣬�ر�DMA

	//���CPSM״̬��
	SDIO_WriteRead_Data(SD_DATATIMEOUT, 0, 0, SDIO_To_Card);

	if (SDIO->RESP1 & SD_CARD_LOCKED)
		return SD_LOCK_UNLOCK_FAILED; //������

	if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
	{
		BlockSize = 512;
		WriteAddr /= 512;
	}

	/* ���ÿ��Ŀ��С */
	/* ��Ҫ�Ƿ���CMD16����ȥ���� */
	/* SDSC�����������ÿ�Ĵ�С */
	/* SDHC������Ĵ�С��λ512byte������CMD16Ӱ�� */
	if ((BlockSize > 0) && (BlockSize <= 2048) && ((BlockSize & (BlockSize - 1)) == 0))
	{
		power = convert_from_bytes_to_power_of_two(BlockSize);
		//����CMD16+�������ݳ���Ϊblksize,����Ӧ
		SDIO_Send_CMDIndex(SD_CMD_SET_BLOCKLEN, (uint32_t)BlockSize, SD_Response_Short); //����CMD16 + ����Ӧ��R1Ӧ��
		errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);								 //�ȴ�R1��Ӧ
		if (errorstatus != SD_OK)
			return errorstatus; //����ʧ��
	}
	else
		return (SD_INVALID_PARAMETER);
	/******************************************************
					   ���������С����
	******************************************************/

	/* �������ݳ����Ƿ񳬳���Χ */
	if (NumberOfBlocks * BlockSize > SD_MAX_DATA_LENGTH)
	{
		errorstatus = SD_INVALID_PARAMETER; //������Χ�ˣ����ش�����Ϣ
		return errorstatus;
	}

	/*!< To improve performance */
	SDIO_Send_CMDIndex(SD_CMD_APP_CMD, (uint32_t)(RCA << 16), SD_Response_Short); //����CMD55 + ����Ӧ��R1Ӧ��
	errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
	if (errorstatus != SD_OK)
		return errorstatus; //����ʧ��

	//����CMD23���ÿ���Ŀ
	/*!< To improve performance */
	SDIO_Send_CMDIndex(SD_CMD_SET_BLOCK_COUNT, (uint32_t)NumberOfBlocks, SD_Response_Short); //����CMD23 + ����Ӧ��R1Ӧ��
	errorstatus = CmdResp1Error(SD_CMD_SET_BLOCK_COUNT);
	if (errorstatus != SD_OK)
		return errorstatus; //����ʧ��

	/*!< Send CMD25 WRITE_MULT_BLOCK with argument data address */
	SDIO_Send_CMDIndex(SD_CMD_WRITE_MULT_BLOCK, (uint32_t)WriteAddr, SD_Response_Short); //����CMD25 + ����Ӧ��R1Ӧ��
	errorstatus = CmdResp1Error(SD_CMD_WRITE_MULT_BLOCK);
	if (errorstatus != SD_OK)
		return errorstatus; //����ʧ��

	SDIO_WriteRead_Data(SD_DATATIMEOUT, (NumberOfBlocks * BlockSize), power, SDIO_To_Card); //power������д9Ҳ��

	TransferError = SD_OK;
	TransferEnd = 0;
	StopCondition = 1;

	SDIO->MASK |= ((1 << 1) | (1 << 3) | (1 << 8) | (1 << 4) | (1 << 9)); //������Ҫ���ж�
	//             DCRCFAIL DTIMEOUT DATAEND TXUNDERR  STBITERR

	SDIO->DCTRL |= 1 << 3;																	//ʹ��DMA����
	SDIO_DMA_SendData((uint32_t *)writebuff, (NumberOfBlocks * BlockSize), DMA2_WriteDATA); //DMA��ʼд������

	timeout = SDIO_DATATIMEOUT;
	while (((DMA2->ISR & 0x2000) == RESET) && timeout)
		timeout--;	//�ȴ��������
	if (timeout == 0) //��ʱ
	{
		SD_Init();				//���³�ʼ��SD��,���Խ��д������������
		return SD_DATA_TIMEOUT; //��ʱ
	}
	timeout = SDIO_DATATIMEOUT;
	while ((TransferEnd == 0) && (TransferError == SD_OK) && timeout)
		timeout--;

	if (timeout == 0)
		return SD_DATA_TIMEOUT; //��ʱ
	if (TransferError != SD_OK)
		return TransferError;

	SDIO->ICR = SDIO_STATIC_FLAGS; //������б��
	errorstatus = IsCardProgramming(&cardstate);
	while ((errorstatus == SD_OK) && ((cardstate == SD_CARD_PROGRAMMING) || (cardstate == SD_CARD_RECEIVING)))
	{
		errorstatus = IsCardProgramming(&cardstate);
	}

	return (errorstatus);
}
/**********************************************************
* �������� ---> �ȴ�������
* ��ڲ��� ---> none
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
SD_Error SD_WaitReadOperation(void)
{
	SD_Error errorstatus = SD_OK;

	while ((SD_DMAEndOfTransferStatus() == RESET) && (TransferEnd == 0) && (TransferError == SD_OK))
	{
	}

	if (TransferError != SD_OK)
		return (TransferError);

	return (errorstatus);
}
/**********************************************************
* �������� ---> �ȴ�д����
* ��ڲ��� ---> none
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
SD_Error SD_WaitWriteOperation(void)
{
	SD_Error errorstatus = SD_OK;

	while ((SD_DMAEndOfTransferStatus() == RESET) && (TransferEnd == 0) && (TransferError == SD_OK))
	{
	}

	if (TransferError != SD_OK)
		return (TransferError);

	SDIO->ICR = SDIO_STATIC_FLAGS; //������б��

	return (errorstatus);
}
/**********************************************************
* �������� ---> ��ȡ����״̬
* ��ڲ��� ---> none
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
SDTransferState SD_GetTransferState(void)
{
	if (SDIO->STA & ((1 << 12) | (1 << 13)))
		return (SD_TRANSFER_BUSY);
	//               TXACT     RXACT
	else
		return (SD_TRANSFER_OK);
}
/**********************************************************
* �������� ---> ����ֹͣ����
* ��ڲ��� ---> none
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
SD_Error SD_StopTransfer(void)
{
	SD_Error errorstatus = SD_OK;

	/*!< Send CMD12 STOP_TRANSMISSION  */
	SDIO_Send_CMDIndex(SD_CMD_STOP_TRANSMISSION, 0x0, SD_Response_Short); //����CMD12 + ����Ӧ��R1Ӧ��
	errorstatus = CmdResp1Error(SD_CMD_STOP_TRANSMISSION);

	return (errorstatus);
}
/**********************************************************
* �������� ---> ��������
* ��ڲ��� ---> startaddr����ʼ��ַ
*               endaddr��������ַ
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
SD_Error SD_Erase(uint32_t startaddr, uint32_t endaddr)
{
	SD_Error errorstatus = SD_OK;
	uint32_t delay = 0;
	__IO uint32_t maxdelay = 0;
	uint8_t cardstate = 0;

	/*!< Check if the card coomnd class supports erase command */
	if (((CSD_Tab[1] >> 20) & SD_CCCC_ERASE) == 0)
	{
		errorstatus = SD_REQUEST_NOT_APPLICABLE;
		return (errorstatus);
	}

	maxdelay = 120000 / ((SDIO->CLKCR & 0xFF) + 2);

	if (SDIO->RESP1 & SD_CARD_LOCKED) //������
	{
		errorstatus = SD_LOCK_UNLOCK_FAILED;
		return (errorstatus);
	}

	if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
	{
		startaddr /= 512;
		endaddr /= 512;
	}

	/*!< According to sd-card spec 1.0 ERASE_GROUP_START (CMD32) and erase_group_end(CMD33) */
	if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) || (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
	{
		/*!< Send CMD32 SD_ERASE_GRP_START with argument as addr  */
		SDIO_Send_CMDIndex(SD_CMD_SD_ERASE_GRP_START, startaddr, SD_Response_Short); //����CMD32 + ����Ӧ��R1Ӧ��
		errorstatus = CmdResp1Error(SD_CMD_SD_ERASE_GRP_START);
		if (errorstatus != SD_OK)
			return errorstatus; //����ʧ��

		/*!< Send CMD33 SD_ERASE_GRP_END with argument as addr  */
		SDIO_Send_CMDIndex(SD_CMD_SD_ERASE_GRP_END, endaddr, SD_Response_Short); //����CMD33 + ����Ӧ��R1Ӧ��
		errorstatus = CmdResp1Error(SD_CMD_SD_ERASE_GRP_END);
		if (errorstatus != SD_OK)
			return errorstatus; //����ʧ��
	}
	/*!< Send CMD38 ERASE */
	SDIO_Send_CMDIndex(SD_CMD_ERASE, 0, SD_Response_Short); //����CMD38 + ����Ӧ��R1Ӧ��
	errorstatus = CmdResp1Error(SD_CMD_ERASE);
	if (errorstatus != SD_OK)
		return errorstatus; //����ʧ��

	for (delay = 0; delay < maxdelay; delay++)
	{
	} //��ʱ�������в�������

	/*!< Wait till the card is in programming state */
	errorstatus = IsCardProgramming(&cardstate);

	while ((errorstatus == SD_OK) && ((SD_CARD_PROGRAMMING == cardstate) || (SD_CARD_RECEIVING == cardstate)))
	{
		errorstatus = IsCardProgramming(&cardstate);
	}

	return (errorstatus);
}
/**********************************************************
* �������� ---> ��ȡ��״̬�Ĵ���
* ��ڲ��� ---> *pcardstatus������״ֵ̬����
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
SD_Error SD_SendStatus(uint32_t *pcardstatus)
{
	SD_Error errorstatus = SD_OK;

	SDIO_Send_CMDIndex(SD_CMD_SEND_STATUS, (uint32_t)RCA << 16, SD_Response_Short); //����CMD13 + ����Ӧ��R1Ӧ��
	errorstatus = CmdResp1Error(SD_CMD_SEND_STATUS);

	if (errorstatus != SD_OK)
		return (errorstatus);

	*pcardstatus = SDIO->RESP1;
	return (errorstatus);
}
/**********************************************************
* �������� ---> ��ȡSD״̬�Ĵ���
* ��ڲ��� ---> *psdstatus������״ֵ̬����
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
SD_Error SD_SendSDStatus(uint32_t *psdstatus)
{
	SD_Error errorstatus = SD_OK;
	uint32_t count = 0;

	if (SDIO->RESP1 & SD_CARD_LOCKED) //������
	{
		errorstatus = SD_LOCK_UNLOCK_FAILED;
		return (errorstatus);
	}

	/*!< Set block size for card if it is not equal to current block size for card. */
	SDIO_Send_CMDIndex(SD_CMD_SET_BLOCKLEN, 64, SD_Response_Short); //����CMD16 + ����Ӧ��R1Ӧ��
	errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);
	if (errorstatus != SD_OK)
		return errorstatus; //����ʧ��

	/*!< CMD55 */
	SDIO_Send_CMDIndex(SD_CMD_APP_CMD, (uint32_t)RCA << 16, SD_Response_Short); //����CMD55 + ����Ӧ��R1Ӧ��
	errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
	if (errorstatus != SD_OK)
		return errorstatus; //����ʧ��

	SDIO_WriteRead_Data(SD_DATATIMEOUT, 64, 6, Card_To_SDIO); //power������д9Ҳ��

	/*!< Send ACMD13 SD_APP_STAUS  with argument as card's RCA.*/
	SDIO_Send_CMDIndex(SD_CMD_SD_APP_STAUS, 0, SD_Response_Short); //����ACMD13 + ����Ӧ��R1Ӧ��
	errorstatus = CmdResp1Error(SD_CMD_SD_APP_STAUS);
	if (errorstatus != SD_OK)
		return errorstatus; //����ʧ��

	//��ʼ��������
	while (!(SDIO->STA & ((1 << 5) | (1 << 1) | (1 << 3) | (1 << 10) | (1 << 9))))
	{							   //               RXOVERR DCRCFAIL DTIMEOUT DBCKEND    STBITERR
		if (SDIO->STA & (1 << 15)) //����FIFO������FIFO�����ٻ���8����
		{
			for (count = 0; count < 8; count++)
			{
				*(psdstatus + count) = SDIO->FIFO;
			}
			psdstatus += 8;
		}
	}

	if (SDIO->STA & (1 << 3)) //���ݳ�ʱ
	{
		SDIO->ICR = 1 << 3; //�����־
		errorstatus = SD_DATA_TIMEOUT;
		return (errorstatus);
	}
	else if (SDIO->STA & (1 << 1)) //CRCУ�����
	{
		SDIO->ICR = 1 << 1; //�����־
		errorstatus = SD_DATA_CRC_FAIL;
		return (errorstatus);
	}
	else if (SDIO->STA & (1 << 5)) //����FIFO�������
	{
		SDIO->ICR = 1 << 5; //�����־
		errorstatus = SD_RX_OVERRUN;
		return (errorstatus);
	}
	else if (SDIO->STA & (1 << 9)) //����FIFO�������
	{
		SDIO->ICR = 1 << 9; //�����־
		errorstatus = SD_START_BIT_ERR;
		return (errorstatus);
	}

	while (SDIO->STA & (1 << 21)) //����FIFO�е����ݿ���
	{
		*psdstatus = SDIO->FIFO;
		psdstatus++;
	}

	SDIO->ICR = SDIO_STATIC_FLAGS; //������б��

	return (errorstatus);
}
/**********************************************************
* �������� ---> ���CMD0ִ�����
* ��ڲ��� ---> none
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
static SD_Error CmdError(void)
{
	SD_Error errorstatus = SD_OK;
	uint32_t timeout;

	timeout = SDIO_CMD0TIMEOUT; /*!< 10000 */

	while (timeout--)
	{
		if (SDIO->STA & (1 << 7))
			break; //�����ѷ���(����Ҫ��Ӧ)
	}

	if (timeout == 0)
	{
		errorstatus = SD_CMD_RSP_TIMEOUT;
		return (errorstatus);
	}

	SDIO->ICR = SDIO_STATIC_FLAGS; //������б��

	return (errorstatus);
}
/**********************************************************
* �������� ---> ���R7��Ӧ���
* ��ڲ��� ---> none
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
static SD_Error CmdResp7Error(void)
{
	SD_Error errorstatus = SD_OK;
	uint32_t status;
	uint32_t timeout = SDIO_CMD0TIMEOUT;

	status = SDIO->STA;

	while (!(status & ((1 << 0) | (1 << 6) | (1 << 2))) && (timeout > 0))
	{ //           CCRCFAIL CMDREND  CTIMEOUT
		timeout--;
		status = SDIO->STA;
	}

	if ((timeout == 0) || (status & (1 << 2))) //������Ӧ��ʱ
	{
		/*!< Card is not V2.0 complient or card does not support the set voltage range */
		errorstatus = SD_CMD_RSP_TIMEOUT;
		SDIO->ICR = 1 << 2; //�����־
		return (errorstatus);
	}

	if (status & (1 << 6)) //�ѽ��յ���Ӧ(CRC���ɹ�)
	{
		/*!< Card is SD V2.0 compliant */
		errorstatus = SD_OK;
		SDIO->ICR = 1 << 6; //�����־
		return (errorstatus);
	}
	return (errorstatus);
}
/**********************************************************
* �������� ---> ���R1��Ӧ���
* ��ڲ��� ---> cmd������
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
static SD_Error CmdResp1Error(uint8_t cmd)
{
	while (!(SDIO->STA & ((1 << 0) | (1 << 6) | (1 << 2))))
	{
	}
	//                   CCRCFAIL CMDREND CTIMEOUT
	SDIO->ICR = SDIO_STATIC_FLAGS; //������б��

	return (SD_Error)(SDIO->RESP1 & SD_OCR_ERRORBITS);
}
/**********************************************************
* �������� ---> ���R3��Ӧ���
* ��ڲ��� ---> none
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
static SD_Error CmdResp3Error(void)
{
	SD_Error errorstatus = SD_OK;
	uint32_t status;

	status = SDIO->STA;

	while (!(status & ((1 << 0) | (1 << 6) | (1 << 2))))
		status = SDIO->STA;
	//                CCRCFAIL CMDREND CTIMEOUT
	if (status & (1 << 2)) //������Ӧ��ʱ
	{
		errorstatus = SD_CMD_RSP_TIMEOUT;
		SDIO->ICR = 1 << 2; //�����־
		return (errorstatus);
	}
	SDIO->ICR = SDIO_STATIC_FLAGS; //������б��

	return (errorstatus);
}
/**********************************************************
* �������� ---> ���R2��Ӧ���
* ��ڲ��� ---> none
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
static SD_Error CmdResp2Error(void)
{
	SD_Error errorstatus = SD_OK;
	uint32_t status;

	status = SDIO->STA;

	while (!(status & ((1 << 0) | (1 << 2) | (1 << 6))))
		status = SDIO->STA;
	//                CCRCFAIL CTIMEOUT CMDREND
	if (status & (1 << 2)) //������Ӧ��ʱ
	{
		errorstatus = SD_CMD_RSP_TIMEOUT;
		SDIO->ICR = 1 << 2; //�����־
		return (errorstatus);
	}
	else if (status & (1 << 0)) //���յ�������Ӧ(CRC���ʧ��)
	{
		errorstatus = SD_CMD_CRC_FAIL;
		SDIO->ICR = 1 << 0; //�����־
		return (errorstatus);
	}
	SDIO->ICR = SDIO_STATIC_FLAGS; //������б��

	return (errorstatus);
}
/**********************************************************
* �������� ---> ���R6��Ӧ���
* ��ڲ��� ---> cmd������
*               *prca�����ؿ���Ե�ַ����
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
static SD_Error CmdResp6Error(uint8_t cmd, uint16_t *prca)
{
	SD_Error errorstatus = SD_OK;
	uint32_t status;
	uint32_t response_r1;

	status = SDIO->STA;

	while (!(status & ((1 << 0) | (1 << 2) | (1 << 6))))
		status = SDIO->STA;
	//                CCRCFAIL CTIMEOUT CMDREND
	if (status & (1 << 2)) //������Ӧ��ʱ
	{
		errorstatus = SD_CMD_RSP_TIMEOUT;
		SDIO->ICR = 1 << 2; //�����־
		return (errorstatus);
	}
	else if (status & (1 << 0)) //���յ�������Ӧ(CRC���ʧ��)
	{
		errorstatus = SD_CMD_CRC_FAIL;
		SDIO->ICR = 1 << 0; //�����־
		return (errorstatus);
	}

	/*!< Check response received is of desired command */
	if ((uint8_t)SDIO->RESPCMD != cmd)
	{
		errorstatus = SD_ILLEGAL_CMD;
		return (errorstatus);
	}
	SDIO->ICR = SDIO_STATIC_FLAGS; //������б��

	/*!< We have received response, retrieve it.  */
	response_r1 = SDIO->RESP1;

	if (SD_ALLZERO == (response_r1 & (SD_R6_GENERAL_UNKNOWN_ERROR | SD_R6_ILLEGAL_CMD | SD_R6_COM_CRC_FAILED)))
	{
		*prca = (uint16_t)(response_r1 >> 16);
		return (errorstatus);
	}

	if (response_r1 & SD_R6_GENERAL_UNKNOWN_ERROR)
		return (SD_GENERAL_UNKNOWN_ERROR);
	if (response_r1 & SD_R6_ILLEGAL_CMD)
		return (SD_ILLEGAL_CMD);
	if (response_r1 & SD_R6_COM_CRC_FAILED)
		return (SD_COM_CRC_FAILED);

	return (errorstatus);
}
/**********************************************************
* �������� ---> ��ȡSCR�Ĵ���ֵ
* ��ڲ��� ---> rca������Ե�ַ
*               *pscr��SCR�Ĵ���ֵ����
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
static SD_Error FindSCR(uint16_t rca, uint32_t *pscr)
{
	SD_Error errorstatus = SD_OK;
	uint32_t index = 0;
	uint32_t tempscr[2] = {0, 0};

	/*!< Set Block Size To 8 Bytes */
	/*!< Send CMD55 APP_CMD with argument as card's RCA */
	SDIO_Send_CMDIndex(SD_CMD_SET_BLOCKLEN, (uint32_t)8, SD_Response_Short); //����CMD16 + ����Ӧ��R1Ӧ��
	errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);
	if (errorstatus != SD_OK)
		return errorstatus; //����ʧ��

	/*!< Send CMD55 APP_CMD with argument as card's RCA */
	SDIO_Send_CMDIndex(SD_CMD_APP_CMD, (uint32_t)RCA << 16, SD_Response_Short); //����CMD55 + ����Ӧ��R1Ӧ��
	errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
	if (errorstatus != SD_OK)
		return errorstatus; //����ʧ��

	SDIO_WriteRead_Data(SD_DATATIMEOUT, 8, 3, Card_To_SDIO); //power������д9Ҳ��

	/******************************************************
				 ��ʱһ��ʱ���CPU��������
	******************************************************/

	for (index = 0; index < 20; index++)
	{
	}

	index = 0;

	/*****************************************************/

	/*!< Send ACMD51 SD_APP_SEND_SCR with argument as 0 */
	SDIO_Send_CMDIndex(SD_CMD_SD_APP_SEND_SCR, 0x0, SD_Response_Short); //����ACMD51 + ����Ӧ��R1Ӧ��
	errorstatus = CmdResp1Error(SD_CMD_SD_APP_SEND_SCR);
	if (errorstatus != SD_OK)
		return errorstatus; //����ʧ��

	while (!(SDIO->STA & ((1 << 5) | (1 << 1) | (1 << 3) | (1 << 10) | (1 << 9))))
	{							   //               RXOVERR DCRCFAIL DTIMEOUT DBCKEND   STBITERR
		if (SDIO->STA & (1 << 21)) //�ڽ���FIFO�е����ݿ���
		{
			*(tempscr + index) = SDIO->FIFO;
			index++;
			if (index == 2)
				break;
		}
	}

	if (SDIO->STA & (1 << 3)) //���ݳ�ʱ
	{
		SDIO->ICR = 1 << 3; //�����־
		errorstatus = SD_DATA_TIMEOUT;
		return (errorstatus);
	}
	else if (SDIO->STA & (1 << 1)) //�ѷ���/�������ݿ�(CRC���ʧ��)
	{
		SDIO->ICR = 1 << 1; //�����־
		errorstatus = SD_DATA_CRC_FAIL;
		return (errorstatus);
	}
	else if (SDIO->STA & (1 << 5)) //����FIFO�������
	{
		SDIO->ICR = 1 << 5; //�����־
		errorstatus = SD_RX_OVERRUN;
		return (errorstatus);
	}
	else if (SDIO->STA & (1 << 9)) //��ʼλ����
	{
		SDIO->ICR = 1 << 9; //�����־
		errorstatus = SD_START_BIT_ERR;
		return (errorstatus);
	}
	SDIO->ICR = SDIO_STATIC_FLAGS; //������б��

	*(pscr + 1) = ((tempscr[0] & SD_0TO7BITS) << 24) | ((tempscr[0] & SD_8TO15BITS) << 8) | ((tempscr[0] & SD_16TO23BITS) >> 8) | ((tempscr[0] & SD_24TO31BITS) >> 24);
	*(pscr) = ((tempscr[1] & SD_0TO7BITS) << 24) | ((tempscr[1] & SD_8TO15BITS) << 8) | ((tempscr[1] & SD_16TO23BITS) >> 8) | ((tempscr[1] & SD_24TO31BITS) >> 24);

	return (errorstatus);
}
/**********************************************************
* �������� ---> �������߿��
* ��ڲ��� ---> NewState��״̬������
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
static SD_Error SDEnWideBus(FunctionalState NewState)
{
	SD_Error errorstatus = SD_OK;
	uint32_t scr[2] = {0, 0};

	if (SDIO->RESP1 & SD_CARD_LOCKED) //������
	{
		errorstatus = SD_LOCK_UNLOCK_FAILED;
		return (errorstatus);
	}

	/*!< Get SCR Register */
	errorstatus = FindSCR(RCA, scr);
	if (errorstatus != SD_OK)
		return errorstatus; //����ʧ��

	/*!< If wide bus operation to be enabled */
	if (NewState == ENABLE)
	{
		/*!< If requested card supports wide bus operation */
		if ((scr[1] & SD_WIDE_BUS_SUPPORT) != SD_ALLZERO)
		{
			/*!< Send CMD55 APP_CMD with argument as card's RCA.*/
			SDIO_Send_CMDIndex(SD_CMD_APP_CMD, (uint32_t)RCA << 16, SD_Response_Short); //����CMD55 + ����Ӧ��R1Ӧ��
			errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
			if (errorstatus != SD_OK)
				return errorstatus; //����ʧ��

			/*!< Send ACMD6 APP_CMD with argument as 2 for wide bus mode */
			SDIO_Send_CMDIndex(SD_CMD_APP_SD_SET_BUSWIDTH, 0x2, SD_Response_Short); //����ACMD6 + ����Ӧ��R1Ӧ��
			errorstatus = CmdResp1Error(SD_CMD_APP_SD_SET_BUSWIDTH);
			if (errorstatus != SD_OK)
				return errorstatus; //����ʧ��

			return (errorstatus);
		}
		else
		{
			errorstatus = SD_REQUEST_NOT_APPLICABLE;
			return (errorstatus);
		}
	} /*!< If wide bus operation to be disabled */
	else
	{
		/*!< If requested card supports 1 bit mode operation */
		if ((scr[1] & SD_SINGLE_BUS_SUPPORT) != SD_ALLZERO)
		{
			/*!< Send CMD55 APP_CMD with argument as card's RCA.*/
			SDIO_Send_CMDIndex(SD_CMD_APP_CMD, (uint32_t)RCA << 16, SD_Response_Short); //����CMD55 + ����Ӧ��R1Ӧ��
			errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
			if (errorstatus != SD_OK)
				return errorstatus; //����ʧ��

			/*!< Send ACMD6 APP_CMD with argument as 2 for wide bus mode */
			SDIO_Send_CMDIndex(SD_CMD_APP_SD_SET_BUSWIDTH, 0x00, SD_Response_Short); //����ACMD6 + ����Ӧ��R1Ӧ��
			errorstatus = CmdResp1Error(SD_CMD_APP_SD_SET_BUSWIDTH);
			if (errorstatus != SD_OK)
				return errorstatus; //����ʧ��

			return (errorstatus);
		}
		else
		{
			errorstatus = SD_REQUEST_NOT_APPLICABLE;
			return (errorstatus);
		}
	}
}
/**********************************************************
* �������� ---> ��ȡ��ִ��״̬
* ��ڲ��� ---> *pstatus������״ֵ̬����
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
static SD_Error IsCardProgramming(uint8_t *pstatus)
{
	SD_Error errorstatus = SD_OK;
	__IO uint32_t respR1 = 0, status = 0;

	//����CMD13����
	SDIO_Send_CMDIndex(SD_CMD_SEND_STATUS, (uint32_t)RCA << 16, SD_Response_Short); //����CMD13 + ����Ӧ��R1Ӧ��

	status = SDIO->STA;
	while (!(status & ((1 << 0) | (1 << 6) | (1 << 2))))
		status = SDIO->STA;
	//                CCRCFAIL CMDREND  CTIMEOUT
	if (status & (1 << 2)) //������Ӧ��ʱ
	{
		errorstatus = SD_CMD_RSP_TIMEOUT;
		SDIO->ICR = 1 << 2; //�����־
		return (errorstatus);
	}
	else if (status & (1 << 0)) //���յ�������Ӧ(CRC���ʧ��)
	{
		errorstatus = SD_CMD_CRC_FAIL;
		SDIO->ICR = 1 << 0; //�����־
		return (errorstatus);
	}

	status = (uint32_t)SDIO->RESPCMD;

	/*!< Check response received is of desired command */
	if (status != SD_CMD_SEND_STATUS)
	{
		errorstatus = SD_ILLEGAL_CMD;
		return (errorstatus);
	}
	SDIO->ICR = SDIO_STATIC_FLAGS; //������б��

	/*!< We have received response, retrieve it for analysis  */
	respR1 = SDIO->RESP1;

	/*!< Find out card status */
	*pstatus = (uint8_t)((respR1 >> 9) & 0x0000000F);

	if ((respR1 & SD_OCR_ERRORBITS) == SD_ALLZERO)
		return (errorstatus);
	if (respR1 & SD_OCR_ADDR_OUT_OF_RANGE)
		return (SD_ADDR_OUT_OF_RANGE);
	if (respR1 & SD_OCR_ADDR_MISALIGNED)
		return (SD_ADDR_MISALIGNED);
	if (respR1 & SD_OCR_BLOCK_LEN_ERR)
		return (SD_BLOCK_LEN_ERR);
	if (respR1 & SD_OCR_ERASE_SEQ_ERR)
		return (SD_ERASE_SEQ_ERR);
	if (respR1 & SD_OCR_BAD_ERASE_PARAM)
		return (SD_BAD_ERASE_PARAM);
	if (respR1 & SD_OCR_WRITE_PROT_VIOLATION)
		return (SD_WRITE_PROT_VIOLATION);
	if (respR1 & SD_OCR_LOCK_UNLOCK_FAILED)
		return (SD_LOCK_UNLOCK_FAILED);
	if (respR1 & SD_OCR_COM_CRC_FAILED)
		return (SD_COM_CRC_FAILED);
	if (respR1 & SD_OCR_ILLEGAL_CMD)
		return (SD_ILLEGAL_CMD);
	if (respR1 & SD_OCR_CARD_ECC_FAILED)
		return (SD_CARD_ECC_FAILED);
	if (respR1 & SD_OCR_CC_ERROR)
		return (SD_CC_ERROR);
	if (respR1 & SD_OCR_GENERAL_UNKNOWN_ERROR)
		return (SD_GENERAL_UNKNOWN_ERROR);
	if (respR1 & SD_OCR_STREAM_READ_UNDERRUN)
		return (SD_STREAM_READ_UNDERRUN);
	if (respR1 & SD_OCR_STREAM_WRITE_OVERRUN)
		return (SD_STREAM_WRITE_OVERRUN);
	if (respR1 & SD_OCR_CID_CSD_OVERWRIETE)
		return (SD_CID_CSD_OVERWRITE);
	if (respR1 & SD_OCR_WP_ERASE_SKIP)
		return (SD_WP_ERASE_SKIP);
	if (respR1 & SD_OCR_CARD_ECC_DISABLED)
		return (SD_CARD_ECC_DISABLED);
	if (respR1 & SD_OCR_ERASE_RESET)
		return (SD_ERASE_RESET);
	if (respR1 & SD_OCR_AKE_SEQ_ERROR)
		return (SD_AKE_SEQ_ERROR);

	return (errorstatus);
}
/**********************************************************
* �������� ---> �õ�NumberOfBytes��2Ϊ�׵�ָ��ֵ
* ��ڲ��� ---> NumberOfBytes��2��ָ��ֵ
* ������ֵ ---> ָ��������
* ����˵�� ---> none
**********************************************************/
uint8_t convert_from_bytes_to_power_of_two(uint16_t NumberOfBytes)
{
	uint8_t count = 0;

	while (NumberOfBytes != 1)
	{
		NumberOfBytes >>= 1;
		count++;
	}
	return (count);
}
/**********************************************************
* �������� ---> �жϴ�����
* ��ڲ��� ---> *psdstatus������״ֵ̬����
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
SD_Error SD_ProcessIRQSrc(void)
{
	//	if(StopCondition == 1)
	//	{
	//		SDIO->ARG = 0x0;
	//		SDIO->CMD = 0x44C;
	//		TransferError = CmdResp1Error(SD_CMD_STOP_TRANSMISSION);
	//	}
	//	else
	//	{
	//		TransferError = SD_OK;
	//	}
	//	SDIO_ClearITPendingBit(SDIO_IT_DATAEND);
	//	SDIO_ITConfig(SDIO_IT_DATAEND, DISABLE);
	//	TransferEnd = 1;
	//	return(TransferError);

	if (SDIO->STA & (1 << 8)) //��������ж�
	{
		if (StopCondition == 1)
		{
			//����CMD12,��������
			SDIO_Send_CMDIndex(SD_CMD_STOP_TRANSMISSION, 0, SD_Response_Short); //����CMD16 + ����Ӧ��R1Ӧ��
			TransferError = CmdResp1Error(SD_CMD_STOP_TRANSMISSION);
		}
		else
			TransferError = SD_OK;

		SDIO->ICR = 1 << 8;																						  //�������жϱ��
		SDIO->MASK &= ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | (1 << 4) | (1 << 5) | (1 << 9)); //�ر�����ж�

		TransferEnd = 1;
		return (TransferError);
	}
	if (SDIO->STA & (1 << 1)) //�ѷ���/�������ݿ�(CRC���ʧ��)
	{
		SDIO->ICR = 1 << 1;																						  //�����־
		SDIO->MASK &= ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | (1 << 4) | (1 << 5) | (1 << 9)); //�ر�����ж�
		TransferError = SD_DATA_CRC_FAIL;
		return (SD_DATA_CRC_FAIL);
	}
	if (SDIO->STA & (1 << 3)) //���ݳ�ʱ
	{
		SDIO->ICR = 1 << 3;																						  //�����־
		SDIO->MASK &= ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | (1 << 4) | (1 << 5) | (1 << 9)); //�ر�����ж�
		TransferError = SD_DATA_TIMEOUT;
		return (SD_DATA_TIMEOUT);
	}
	if (SDIO->STA & (1 << 5)) //����FIFO�������
	{
		SDIO->ICR = 1 << 5;																						  //�����־
		SDIO->MASK &= ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | (1 << 4) | (1 << 5) | (1 << 9)); //�ر�����ж�
		TransferError = SD_RX_OVERRUN;
		return (SD_RX_OVERRUN);
	}
	if (SDIO->STA & (1 << 4)) //����FIFO�������
	{
		SDIO->ICR = 1 << 4;																						  //�����־
		SDIO->MASK &= ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | (1 << 4) | (1 << 5) | (1 << 9)); //�ر�����ж�
		TransferError = SD_TX_UNDERRUN;
		return (SD_TX_UNDERRUN);
	}
	if (SDIO->STA & (1 << 9)) //��ʼλ����
	{
		SDIO->ICR = 1 << 9;																						  //�����־
		SDIO->MASK &= ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | (1 << 4) | (1 << 5) | (1 << 9)); //�ر�����ж�
		TransferError = SD_START_BIT_ERR;
		return (SD_START_BIT_ERR);
	}
	return (SD_OK);
}
/**********************************************************
* �������� ---> SDIO�жϷ�����
* ��ڲ��� ---> none
* ������ֵ ---> none
* ����˵�� ---> none
**********************************************************/
void SDIO_IRQHandler(void)
{
	SD_ProcessIRQSrc(); //��������SDIO����ж�
}
/**********************************************************
* �������� ---> ��SD��
* ��ڲ��� ---> *readbuff����ȡ���ݻ���
*               sector����ȡblock��ַ
*               cnt����ȡblock��Ŀ
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
SD_Error SD_ReadDisk(uint8_t *readbuff, uint32_t sector, uint16_t cnt)
{
	SD_Error sta = SD_OK;
	uint16_t n;

	if (CardType != SDIO_STD_CAPACITY_SD_CARD_V1_1)
		sector <<= 9;

	if ((uint32_t)readbuff % 4 != 0)
	{
		for (n = 0; n < cnt; n++)
		{
			sta = SD_ReadBlock(SDIO_DATA_BUFFER, sector, 512); //����sector�Ķ�����
			memcpy(readbuff, SDIO_DATA_BUFFER, 512);
			readbuff += 512;
		}
	}
	else
	{
		if (cnt == 1)
			sta = SD_ReadBlock(readbuff, sector, 512); //����sector�Ķ�����
		else
			sta = SD_ReadMultiBlocks(readbuff, sector, 512, cnt); //���sector
	}
	return sta;
}
/**********************************************************
* �������� ---> дSD��
* ��ڲ��� ---> *writebuff����ȡ���ݻ���
*               sector����ȡblock��ַ
*               cnt����ȡblock��Ŀ
* ������ֵ ---> ���ؿ�Ӧ��
* ����˵�� ---> none
**********************************************************/
SD_Error SD_WriteDisk(uint8_t *writebuff, uint32_t sector, uint16_t cnt)
{
	SD_Error sta = SD_OK;
	uint16_t n;

	if (CardType != SDIO_STD_CAPACITY_SD_CARD_V1_1)
		sector <<= 9;

	if ((uint32_t)writebuff % 4 != 0)
	{
		for (n = 0; n < cnt; n++)
		{
			memcpy(SDIO_DATA_BUFFER, writebuff, 512);
			sta = SD_WriteBlock(SDIO_DATA_BUFFER, sector, 512); //����sector��д����
			writebuff += 512;
		}
	}
	else
	{
		if (cnt == 1)
			sta = SD_WriteBlock(writebuff, sector, 512); //����sector��д����
		else
			sta = SD_WriteMultiBlocks(writebuff, sector, 512, cnt); //���sector
	}
	return sta;
}

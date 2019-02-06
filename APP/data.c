#include "data.h" 
#include "flash.h" 
#include "vs10xx.h" 
#include "mp3player.h" 
#include "touch.h" 
#include "includes.h"
#include "vs10xx.h"

u8 save_bit[8]={0,0,
	160,	//����:160
	15,		//�������� 150Hz
	15,		//�������� 15dB	
	10,		//�������� 10Khz	
	0,		//�������� 0dB
	0,		//�ռ�Ч��	
};

//������EEPROM����ĵ�ַ�����ַ,ռ��14���ֽ�(RANGE:SAVE_ADDR_BASE~SAVE_ADDR_BASE+13)
#define SAVE_ADDR_BASE (1024*23)*1024 + 20//���������λ��
#define save_bit_local (1024*23)*1024			//Ĭ����23�ĵ�ַ

void read_data(void)
{
	u8 buff[8];
	SPI_Flash_Read(buff, save_bit_local, 8);
	if (info.curindex > info.totmp3num 
		|| buff[2] > 200 || buff[3] > 15 || buff[4] > 15
		|| buff[5] > 15 || buff[6] > 15 || buff[7] > 3)
	{
		info.curindex = 0;
		buff[0] = save_bit[0] = (info.curindex >> 8) & 0xff;
		buff[1] = save_bit[1] = info.curindex & 0xff;
		buff[2] = vsset.mvol = save_bit[2];
		buff[3] = vsset.bflimit = save_bit[3];
		buff[4] = vsset.bass = save_bit[4];
		buff[5] = vsset.tflimit = save_bit[5];
		buff[6] = vsset.treble= save_bit[6];
		buff[7] = vsset.effect = save_bit[7];

		SPI_Flash_Write(buff, save_bit_local, 8);
	}
	else
	{
		info.curindex = (buff[0] << 8) | buff[1];
		save_bit[0] = buff[0];
		save_bit[1] = buff[1];
		save_bit[2] = vsset.mvol = buff[2];
		save_bit[3] = vsset.bflimit = buff[3];
		save_bit[4] = vsset.bass = buff[4];
		save_bit[5] = vsset.tflimit = buff[5];
		save_bit[6] = vsset.treble= buff[6];
		save_bit[7] = vsset.effect = buff[7];
	}
	VS_Set_All();
}

void write_data(void)
{
	CPU_SR_ALLOC();
	u8 buff[8];
	buff[0] = save_bit[0] = (info.curindex >> 8) & 0xff;
	buff[1] = save_bit[1] = info.curindex & 0xff;
	buff[2] = vsset.mvol = save_bit[2];
	buff[3] = vsset.bflimit = save_bit[3];
	buff[4] = vsset.bass = save_bit[4];
	buff[5] = vsset.tflimit = save_bit[5];
	buff[6] = vsset.treble= save_bit[6];
	buff[7] = vsset.effect = save_bit[7];
	VS_Set_All();
	OS_CRITICAL_ENTER();	//�����ٽ���
	SPI_Flash_Write(buff, save_bit_local, 8);
	OS_CRITICAL_EXIT();
}

//����У׼����										    
void TP_Save_Adjdata(void)
{
	u8 a[1] = { 0 };
	a[0] = 0X0A;
	SPI_Flash_Write((u8*)&tp_dev.xfac, SAVE_ADDR_BASE, 14);	//ǿ�Ʊ���&tp_dev.xfac��ַ��ʼ��14���ֽ����ݣ������浽tp_dev.touchtype
	SPI_Flash_Write(a, SAVE_ADDR_BASE + 14, 1);					//�����д0X0A���У׼����
}
//�õ�������EEPROM�����У׼ֵ
//����ֵ��1���ɹ���ȡ����
//        0����ȡʧ�ܣ�Ҫ����У׼
u8 TP_Get_Adjdata(void)
{
	u8 read[1] = { 0 };
	SPI_Flash_Read(read, SAVE_ADDR_BASE + 14, 1);//��ȡ�����,���Ƿ�У׼���� 		 
	if (read[0] == 0X0A)//�������Ѿ�У׼����			   
	{
		SPI_Flash_Read((u8*)&tp_dev.xfac, SAVE_ADDR_BASE, 14);//��ȡ֮ǰ�����У׼���� 
		if (tp_dev.touchtype)//X,Y��������Ļ�෴
		{
			CMD_RDX = 0X90;
			CMD_RDY = 0XD0;
		}
		else				   //X,Y��������Ļ��ͬ
		{
			CMD_RDX = 0XD0;
			CMD_RDY = 0X90;
		}
		return 1;
	}
	return 0;
}

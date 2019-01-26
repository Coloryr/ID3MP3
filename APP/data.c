#include "data.h" 
#include "flash.h" 
#include "vs10xx.h" 
#include "mp3player.h" 
#include "touch.h" 
#include "includes.h"

u8 save_bit[5]={0,0,0,0,0};

//������EEPROM����ĵ�ַ�����ַ,ռ��14���ֽ�(RANGE:SAVE_ADDR_BASE~SAVE_ADDR_BASE+13)
#define SAVE_ADDR_BASE (1024*23)*1024 + 20//���������λ��
#define save_bit_local (1024*23)*1024			//Ĭ����23�ĵ�ַ

void read_data(void)
{
	SPI_Flash_Read(save_bit, save_bit_local, 5);
	info.curindex = (save_bit[3] << 8) | save_bit[4];
	vsset.mvol = save_bit[2];
	if (info.curindex > info.totmp3num || save_bit[2] > 200 || save_bit[2] == 0x00)
	{
		info.curindex = 0;
		vsset.mvol = 160;
		save_bit[3] = (info.curindex >> 8) & 0xff;
		save_bit[4] = info.curindex & 0xff;
		save_bit[2] = vsset.mvol;
		SPI_Flash_Write(save_bit, save_bit_local, 5);
	}
}

void write_data(void)
{
	CPU_SR_ALLOC();
	save_bit[3] = (info.curindex >> 8) & 0xff;
	save_bit[4] = info.curindex & 0xff;
	OS_CRITICAL_ENTER();	//�����ٽ���
	SPI_Flash_Write(save_bit, save_bit_local, 5);
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

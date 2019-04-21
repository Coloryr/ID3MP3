#include "data.h"
#include "flash.h"
#include "vs10xx.h"
#include "mp3player.h"
#include "touch.h"
#include "vs10xx.h"
#include "show.h"
#include "stmflash.h"

u8 data_save_bit = 0;

//������EEPROM����ĵ�ַ�����ַ,ռ��14���ֽ�(RANGE:SAVE_ADDR_BASE~SAVE_ADDR_BASE+13)
#define SAVE_ADDR_BASE (1024 * 23) * 1024 + 20 //���������λ��
#define save_bit_local 0x08070000			   //Ĭ����23�ĵ�ַ

void data(u16 *data)
{
	data[1] = info.curindex;
	data[2] = vsset.mvol;
	data[3] = vsset.bflimit;
	data[4] = vsset.bass;
	data[5] = vsset.tflimit;
	data[6] = vsset.treble;
	data[7] = vsset.effect;
}
void read_data(void)
{
	u16 buff[8];
	STMFLASH_Read(save_bit_local, buff, 8);
	info.curindex = buff[1];
	if (info.curindex > info.totmp3num || buff[2] > 200 || buff[3] > 15 || buff[4] > 15 || buff[5] > 15 || buff[6] > 15 || buff[7] > 3)
	{
		info.curindex = 0;
		data(buff);
		STMFLASH_Write(save_bit_local, buff, 8);
	}
	else
	{
		info.curindex = buff[1];
		vsset.mvol = buff[2];
		vsset.bflimit = buff[3];
		vsset.bass = buff[4];
		vsset.tflimit = buff[5];
		vsset.treble = buff[6];
		vsset.effect = buff[7];
	}
	VS_Set_All();
}

void write_data(void)
{
	u16 buff[8];
	data(buff);
	STMFLASH_Write(save_bit_local, buff, 8);
	data_save_bit = 0;
}

//����У׼����
void TP_Save_Adjdata(void)
{
	u8 a[1];
	a[0] = 0X0A;
	SPI_Flash_Write((u8 *)&tp_dev.xfac, SAVE_ADDR_BASE, 14); //ǿ�Ʊ���&tp_dev.xfac��ַ��ʼ��14���ֽ����ݣ������浽tp_dev.touchtype
	SPI_Flash_Write(a, SAVE_ADDR_BASE + 14, 1);				 //�����д0X0A���У׼����
}
//�õ�������EEPROM�����У׼ֵ
//����ֵ��1���ɹ���ȡ����
//        0����ȡʧ�ܣ�Ҫ����У׼
u8 TP_Get_Adjdata(void)
{
	u8 read[1];
	SPI_Flash_Read(read, SAVE_ADDR_BASE + 14, 1); //��ȡ�����,���Ƿ�У׼����
	if (read[0] == 0X0A)						  //�������Ѿ�У׼����
	{
		SPI_Flash_Read((u8 *)&tp_dev.xfac, SAVE_ADDR_BASE, 14); //��ȡ֮ǰ�����У׼����
		if (tp_dev.touchtype)									//X,Y��������Ļ�෴
		{
			CMD_RDX = 0X90;
			CMD_RDY = 0XD0;
		}
		else //X,Y��������Ļ��ͬ
		{
			CMD_RDX = 0XD0;
			CMD_RDY = 0X90;
		}
		return 1;
	}
	return 0;
}

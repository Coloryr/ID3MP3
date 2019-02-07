#include "data.h" 
#include "flash.h" 
#include "vs10xx.h" 
#include "mp3player.h" 
#include "touch.h" 
#include "vs10xx.h"
#include "show.h"

u8 data_save_bit=0;

//������EEPROM����ĵ�ַ�����ַ,ռ��14���ֽ�(RANGE:SAVE_ADDR_BASE~SAVE_ADDR_BASE+13)
#define SAVE_ADDR_BASE (1024*23)*1024 + 20//���������λ��
#define save_bit_local (1024*23)*1024 		//Ĭ����23�ĵ�ַ

void read_data(void)
{
	u8 buff[10];
	SPI_Flash_Read(buff, save_bit_local, 10);
	info.curindex = (buff[0] << 8) | buff[1];
	pwmval = (buff[8] << 8) | buff[9];
	 pwmval = 700;
	if (info.curindex > info.totmp3num
		|| buff[2] > 200 || buff[3] > 15 || buff[4] > 15
		|| buff[5] > 15 || buff[6] > 15 || buff[7] > 3
		//|| pwmval > 890
		)
	{
		info.curindex = 0;
		pwmval = 0;
		buff[0] = (info.curindex >> 8) & 0xff;
		buff[1] = info.curindex & 0xff;
		buff[2] = vsset.mvol;
		buff[3] = vsset.bflimit;
		buff[4] = vsset.bass;
		buff[5] = vsset.tflimit;
		buff[6] = vsset.treble;
		buff[7] = vsset.effect;
		buff[8] = (pwmval >> 8) & 0xff;
		buff[9] = pwmval & 0xff;
		SPI_Flash_Write(buff, save_bit_local, 10);
		SPI_Flash_Read(buff, save_bit_local, 10);
		if (buff[0] != (info.curindex >> 8) & 0xff || buff[1] != info.curindex & 0xff
			|| buff[2] != vsset.mvol || buff[3] != vsset.bflimit || buff[4] != vsset.bass
			|| buff[5] != vsset.tflimit || buff[6] != vsset.treble || buff[7] != vsset.effect
			|| buff[8] != (pwmval >> 8) & 0xff || buff[9] != pwmval & 0xff)
		{	//��дһ��
			buff[0] = (info.curindex >> 8) & 0xff;
			buff[1] = info.curindex & 0xff;
			buff[2] = vsset.mvol;
			buff[3] = vsset.bflimit;
			buff[4] = vsset.bass;
			buff[5] = vsset.tflimit;
			buff[6] = vsset.treble;
			buff[7] = vsset.effect;
			buff[8] = (pwmval >> 8) & 0xff;
			buff[9] = pwmval & 0xff;
			SPI_Flash_Write(buff, save_bit_local, 10);
		}
	}
	else
	{
		info.curindex = (buff[0] << 8) | buff[1];
		pwmval = (buff[8] << 8) | buff[9];
		vsset.mvol = buff[2];
		vsset.bflimit = buff[3];
		vsset.bass = buff[4];
		vsset.tflimit = buff[5];
		vsset.treble = buff[6];
		vsset.effect = buff[7];
	}
	VS_Set_All();
	TIM_SetCompare3(TIM3, pwmval);
}

void write_data(void)
{
	u8 buff[10];
	buff[0] = (info.curindex >> 8) & 0xff;
	buff[1] = info.curindex & 0xff;
	buff[2] = vsset.mvol;
	buff[3] = vsset.bflimit;
	buff[4] = vsset.bass;
	buff[5] = vsset.tflimit;
	buff[6] = vsset.treble;
	buff[7] = vsset.effect;
	buff[6] = vsset.treble;
	buff[7] = vsset.effect;
	buff[8] = (pwmval >> 8) & 0xff;
	buff[9] = pwmval & 0xff;
	SPI_Flash_Write(buff, save_bit_local, 10);
	SPI_Flash_Read(buff, save_bit_local,10);
	if (buff[0] != (info.curindex >> 8) & 0xff || buff[1] != info.curindex & 0xff
		|| buff[2] != vsset.mvol || buff[3] != vsset.bflimit || buff[4] != vsset.bass
		|| buff[5] != vsset.tflimit || buff[6] != vsset.treble || buff[7] != vsset.effect
		|| buff[8] != (pwmval >> 8) & 0xff || buff[9] != pwmval & 0xff)
	{	//��дһ��
		buff[0] = (info.curindex >> 8) & 0xff;
		buff[1] = info.curindex & 0xff;
		buff[2] = vsset.mvol;
		buff[3] = vsset.bflimit;
		buff[4] = vsset.bass;
		buff[5] = vsset.tflimit;
		buff[6] = vsset.treble;
		buff[7] = vsset.effect;
		buff[6] = vsset.treble;
		buff[7] = vsset.effect;
		buff[8] = (pwmval >> 8) & 0xff;
		buff[9] = pwmval & 0xff;
		SPI_Flash_Write(buff, save_bit_local, 10);
	}
	data_save_bit = 0;
}

//����У׼����										    
void TP_Save_Adjdata(void)
{
	u8 a[1];
	a[0] = 0X0A;
	SPI_Flash_Write((u8*)&tp_dev.xfac, SAVE_ADDR_BASE, 14);	//ǿ�Ʊ���&tp_dev.xfac��ַ��ʼ��14���ֽ����ݣ������浽tp_dev.touchtype
	SPI_Flash_Write(a, SAVE_ADDR_BASE + 14, 1);					//�����д0X0A���У׼����
}
//�õ�������EEPROM�����У׼ֵ
//����ֵ��1���ɹ���ȡ����
//        0����ȡʧ�ܣ�Ҫ����У׼
u8 TP_Get_Adjdata(void)
{
	u8 read[1];
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

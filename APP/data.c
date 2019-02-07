#include "data.h" 
#include "flash.h" 
#include "vs10xx.h" 
#include "mp3player.h" 
#include "touch.h" 
#include "vs10xx.h"

u8 save_bit[2]={0,0};
u8 data_save_bit=0;

//保存在EEPROM里面的地址区间基址,占用14个字节(RANGE:SAVE_ADDR_BASE~SAVE_ADDR_BASE+13)
#define SAVE_ADDR_BASE (1024*23)*1024 + 20//触摸保存的位置
#define save_bit_local (1024*23)*1024			//默认是23的地址

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
		buff[2] = vsset.mvol;
		buff[3] = vsset.bflimit;
		buff[4] = vsset.bass;
		buff[5] = vsset.tflimit;
		buff[6] = vsset.treble;
		buff[7] = vsset.effect;
		SPI_Flash_Write(buff, save_bit_local, 8);
	}
	else
	{
		info.curindex = (buff[0] << 8) | buff[1];
		save_bit[0] = buff[0];
		save_bit[1] = buff[1];
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
	u8 buff[8];
	buff[0] = save_bit[0] = (info.curindex >> 8) & 0xff;
	buff[1] = save_bit[1] = info.curindex & 0xff;
	buff[2] = vsset.mvol;
	buff[3] = vsset.bflimit;
	buff[4] = vsset.bass;
	buff[5] = vsset.tflimit;
	buff[6] = vsset.treble;
	buff[7] = vsset.effect;
	SPI_Flash_Write(buff, save_bit_local, 8);
	SPI_Flash_Read(buff, save_bit_local, 8);
	if (buff[0] != (info.curindex >> 8) & 0xff || buff[1] != info.curindex & 0xff
		|| buff[2] != vsset.mvol || buff[3] != vsset.bflimit || buff[4] != vsset.bass
		|| buff[5] != vsset.tflimit || buff[6] != vsset.treble || buff[7] != vsset.effect)
		//再写一次
		SPI_Flash_Write(buff, save_bit_local, 8);
	VS_Set_All();
	data_save_bit=0;
}

//保存校准参数										    
void TP_Save_Adjdata(void)
{
	u8 a[1] = { 0 };
	a[0] = 0X0A;
	SPI_Flash_Write((u8*)&tp_dev.xfac, SAVE_ADDR_BASE, 14);	//强制保存&tp_dev.xfac地址开始的14个字节数据，即保存到tp_dev.touchtype
	SPI_Flash_Write(a, SAVE_ADDR_BASE + 14, 1);					//在最后，写0X0A标记校准过了
}
//得到保存在EEPROM里面的校准值
//返回值：1，成功获取数据
//        0，获取失败，要重新校准
u8 TP_Get_Adjdata(void)
{
	u8 read[1] = { 0 };
	SPI_Flash_Read(read, SAVE_ADDR_BASE + 14, 1);//读取标记字,看是否校准过！ 		 
	if (read[0] == 0X0A)//触摸屏已经校准过了			   
	{
		SPI_Flash_Read((u8*)&tp_dev.xfac, SAVE_ADDR_BASE, 14);//读取之前保存的校准数据 
		if (tp_dev.touchtype)//X,Y方向与屏幕相反
		{
			CMD_RDX = 0X90;
			CMD_RDY = 0XD0;
		}
		else				   //X,Y方向与屏幕相同
		{
			CMD_RDX = 0XD0;
			CMD_RDY = 0X90;
		}
		return 1;
	}
	return 0;
}

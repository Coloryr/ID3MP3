#include "data.h" 
#include "flash.h" 
#include "vs10xx.h" 
#include "mp3player.h" 
#include "touch.h" 
#include "includes.h"

u8 save_bit[5]={0,0,0,0,0};

//保存在EEPROM里面的地址区间基址,占用14个字节(RANGE:SAVE_ADDR_BASE~SAVE_ADDR_BASE+13)
#define SAVE_ADDR_BASE (1024*23)*1024 + 20//触摸保存的位置
#define save_bit_local (1024*23)*1024			//默认是23的地址

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
	OS_CRITICAL_ENTER();	//进入临界区
	SPI_Flash_Write(save_bit, save_bit_local, 5);
	OS_CRITICAL_EXIT();
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

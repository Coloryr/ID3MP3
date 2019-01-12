#include "includes.h" 

u8 save_bit[5]={0,0,0,0,0};

void read_data(void)
{
	SPI_Flash_Read(save_bit, save_bit_local, 5);
	info.curindex = (save_bit[3] << 8) | save_bit[4];
	vsset.mvol = save_bit[2];
	if (info.curindex > info.totmp3num || save_bit[2] > 200 || save_bit[2] == 0x00)
	{
		info.curindex = 0;
		vsset.mvol = 160;
		save_bit[0] = (info.curindex >> 8) & 0xff;
		save_bit[3] = info.curindex & 0xff;
		save_bit[4] = vsset.mvol;
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


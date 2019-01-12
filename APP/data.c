#include "sys.h"
#include "flash.h"
#include "data.h"
#include "mp3player.h"
#include "VS10xx.h"

u8 save_bit[5]={0,0,0,0,0};

void read_data(void)
{
	SPI_Flash_Read(save_bit, save_bit_local, 5);
	info.curindex = (save_bit[0] << 8) | save_bit[1];
	if (info.curindex > info.totmp3num || save_bit[2] > 200 || save_bit[2] == 0x00)
	{
		info.curindex = 0;
		vsset.mvol = 160;
		save_bit[0] = (info.curindex >> 8) & 0xff;
		save_bit[1] = info.curindex & 0xff;
		save_bit[2] = vsset.mvol;
		SPI_Flash_Write(save_bit, save_bit_local, 5);
	}
}

void write_data(void)
{
	SPI_Flash_Write(save_bit, save_bit_local, 5);
}


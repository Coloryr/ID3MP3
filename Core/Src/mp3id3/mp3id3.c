#include "mp3id3.h"
#include "fatfs.h"
#include "malloc.h"
#include "../mp3player/mp3player.h"
#include "../lcd/lcd.h"
#include "../show/show.h"
#include "../font/font.h"
#include "../flash/flash.h"
#include "cmsis_os.h"

UINT br;	   //读写变量

uint16_t UNICODEtoGBK(uint16_t unicode) //???????
{
	uint16_t t[2];
	uint16_t c;
	uint32_t i, li, hi;
	uint16_t n;
	uint32_t gbk2uni_offset = 0;

	if (unicode < 0x80)
		c = unicode; //ASCII,直接不用转换.
	else
	{
		//UNICODE 2 GBK
		gbk2uni_offset = 0;
		/* Unicode to OEMCP */
		hi = ftinfo.ugbksize / 2; //对半开.
		hi = hi / 4 - 1;
		li = 0;
		for (n = 16; n; n--)
		{
			i = li + (hi - li) / 2;
			SPI_Flash_Read((uint8_t *)&t, ftinfo.ugbkaddr + i * 4 + gbk2uni_offset, 4); //读出4个字节
			if (unicode == t[0])
				break;
			if (unicode > t[0])
				li = i;
			else
				hi = i;
		}
		c = n ? t[1] : 0;
	}
	return c;
}

void ID3_UTF8(uint8_t *databuf, uint16_t tag_size, uint16_t i, uint8_t *INFO)
{
	uint8_t temp;
	uint16_t a = 0;
	for (temp = 0; temp < tag_size;)
	{
		a = (databuf[i + temp] << 8) | databuf[i + temp + 1];
		if (a < 0x80)
		{ /* 7-bit */
			info.TIT2[temp++] = (BYTE)a;
		}
		else
		{
			if (a < 0x800)
			{ /* 11-bit */
				INFO[temp++] = (BYTE)(0xC0 | a >> 6);
			}
			else
			{ /* 16-bit */
				INFO[temp++] = (BYTE)(0xE0 | a >> 12);
				INFO[temp++] = (BYTE)(0x80 | (a >> 6 & 0x3F));
			}
			INFO[temp++] = (BYTE)(0x80 | (a & 0x3F));
		}
	}
}

void ID3_UTF16BE(uint8_t *databuf, uint16_t tag_size, uint16_t i, uint8_t *INFO)
{
	uint8_t temp2 = 0;
	uint8_t temp = 0;
	uint8_t temp1 = 0;
	uint16_t a = 0;
	for (temp = 0; temp < tag_size;)
	{
		a = (databuf[i + temp] << 8) | databuf[i + temp + 1];
		a = UNICODEtoGBK(a);
		temp1 = (a >> 8) & 0xff;
		if (temp1 != 0x00)
		{
			INFO[temp2] = temp1;
			temp2++;
		}
		temp1 = a & 0xff;
		if (temp1 != 0x00)
		{
			INFO[temp2] = temp1;
			temp2++;
		}
		temp += 2;
	}
}

void ID3_UTF16LE(uint8_t *databuf, uint16_t tag_size, uint16_t i, uint8_t *INFO)
{
	uint8_t temp2 = 0;
	uint8_t temp, temp1;
	uint16_t a;
	for (temp = 0; temp < tag_size;)
	{
		a = (databuf[i + temp + 1] << 8) | databuf[i + temp];
		a = UNICODEtoGBK(a);
		temp1 = (a >> 8) & 0xff;
		if (temp1 != 0x00)
		{
			INFO[temp2] = temp1;
			temp2++;
		}
		temp1 = a & 0xff;
		if (temp1 != 0x00)
		{
			INFO[temp2] = temp1;
			temp2++;
		}
		temp += 2;
	}
}

void ID3_iso_8859_1(uint8_t *databuf, uint16_t tag_size, uint16_t i, uint8_t *INFO)
{
	uint8_t temp, temp1;
	for (temp = 0; temp < tag_size;)
	{
		temp1 = databuf[i + temp];
		if (temp1 != 0x00)
			INFO[temp++] = temp1;
		else
			temp++;
	}
}

void mp3id3(void)
{
	uint16_t i = 0;
	uint8_t res = 0;
	uint8_t *databuf = 0;
	uint8_t img = 1;
	uint8_t temp;
	uint16_t tag_size;

	databuf = (uint8_t *)malloc(READ_buff_size);

	if (info.TIT2 == NULL)
		info.TIT2 = (uint8_t *)malloc(40);
	if (info.TPE1 == NULL)
		info.TPE1 = (uint8_t *)malloc(40);
	if (info.TALB == NULL)
		info.TALB = (uint8_t *)malloc(40);
	for (temp = 0; temp < 40; temp++)
	{
		info.TIT2[temp] = 0;
		info.TPE1[temp] = 0;
		info.TALB[temp] = 0;
	}
	if (databuf == NULL) //内存申请失败.
		while (1)
		{
			Show_Str(30, 20, 160, 16, "MP3ID3：内存申请失败", 16, 0);
			osDelay(200 / portTICK_RATE_MS);
			LCD_Fill(30, 20, 160, 16, BLACK); //清除显示
			osDelay(200 / portTICK_RATE_MS);
		}
	res = f_read(info.fmp3, databuf, 10, (UINT *)&br); //读出mp3id3头
	if (res != FR_OK)
		return;
	if (databuf[0] == 0x49 && databuf[1] == 0x44 && databuf[2] == 0x33)
	{
		//计算大小
		info.size = databuf[6] << 21 | databuf[7] << 14 | databuf[8] << 7 | databuf[9];
		res = f_read(info.fmp3, databuf, READ_buff_size, (UINT *)&br);
		if (res != FR_OK)
			return;
		i = 0;
		while (img) //查找歌名
		{
			if (databuf[i] == 0x54 && databuf[i + 1] == 0x49 && databuf[i + 2] == 0x54 && databuf[i + 3] == 0x32)
			{ //找到位置
				tag_size = databuf[i + 4] << 24 | databuf[i + 5] << 16 | databuf[i + 6] << 8 | databuf[i + 7];
				if (databuf[i + 11] == 0xFE && databuf[i + 12] == 0xFF && databuf[i + 10] == 0x01)
				{ //UTF-16BE
					i += 13;
					tag_size -= 3;
					ID3_UTF16BE(databuf, tag_size, i, info.TIT2);
				}
				else if (databuf[i + 11] == 0xFF && databuf[i + 12] == 0xFE && databuf[i + 10] == 0x01)
				{ //UTF-16LE
					i += 13;
					tag_size -= 3;
					ID3_UTF16LE(databuf, tag_size, i, info.TIT2);
				}
				else if (databuf[i + 10] == 0x00) //iso-8859-1
				{
					i += 11;
					tag_size -= 1;
					ID3_iso_8859_1(databuf, tag_size, i, info.TIT2);
				}
				else if (databuf[i + 10] == 0x03) //UTF-8
				{
					i += 11;
					tag_size -= 1;
					ID3_UTF8(databuf, tag_size, i, info.TIT2);
				}
				img = 0;
			}
			else
				i++;
			if (i >= READ_buff_size - 6) //找不到位置
			{
				img = 0;
			}
		}
		img = 1;
		i = 0;
		while (img) //查找作者
		{
			if (databuf[i] == 0x54 && databuf[i + 1] == 0x50 && databuf[i + 2] == 0x45 && databuf[i + 3] == 0x31)
			{ //找到位置
				tag_size = databuf[i + 4] << 24 | databuf[i + 5] << 16 | databuf[i + 6] << 8 | databuf[i + 7];
				if (databuf[i + 11] == 0xFE && databuf[i + 12] == 0xFF && databuf[i + 10] == 0x01)
				{ //UTF-16BE
					i += 13;
					tag_size -= 3;
					ID3_UTF16BE(databuf, tag_size, i, info.TPE1);
				}
				else if (databuf[i + 11] == 0xFF && databuf[i + 12] == 0xFE && databuf[i + 10] == 0x01)
				{ //UTF-16LE
					i += 13;
					tag_size -= 3;
					ID3_UTF16LE(databuf, tag_size, i, info.TPE1);
				}
				else if (databuf[i + 10] == 0x00) //iso-8859-1
				{
					i += 11;
					tag_size -= 1;
					ID3_iso_8859_1(databuf, tag_size, i, info.TPE1);
				}
				else if (databuf[i + 10] == 0x03) //UTF-8
				{
					i += 11;
					tag_size -= 1;
					ID3_UTF8(databuf, tag_size, i, info.TPE1);
				}
				img = 0;
			}
			else
				i++;
			if (i >= READ_buff_size - 6) //找不到位置
			{
				img = 0;
			}
		}
		img = 1;
		i = 0;
		while (img) //查找专辑
		{
			if (databuf[i] == 0x54 && databuf[i + 1] == 0x41 && databuf[i + 2] == 0x4c && databuf[i + 3] == 0x42)
			{ //找到位置
				tag_size = databuf[i + 4] << 24 | databuf[i + 5] << 16 | databuf[i + 6] << 8 | databuf[i + 7];
				if (databuf[i + 11] == 0xFE && databuf[i + 12] == 0xFF && databuf[i + 10] == 0x01)
				{ //UTF-16BE
					i += 13;
					tag_size -= 3;
					ID3_UTF16BE(databuf, tag_size, i, info.TALB);
				}
				else if (databuf[i + 11] == 0xFF && databuf[i + 12] == 0xFE && databuf[i + 10] == 0x01)
				{ //UTF-16LE
					i += 13;
					tag_size -= 3;
					ID3_UTF16LE(databuf, tag_size, i, info.TALB);
				}
				else if (databuf[i + 10] == 0x00) //iso-8859-1
				{
					i += 11;
					tag_size -= 1;
					ID3_iso_8859_1(databuf, tag_size, i, info.TALB);
				}
				else if (databuf[i + 10] == 0x03) //UTF-8
				{
					i += 11;
					tag_size -= 1;
					ID3_UTF8(databuf, tag_size, i, info.TALB);
				}
				img = 0;
			}
			else
				i++;
			if (i >= READ_buff_size - 6) //找不到位置
			{
				img = 0;
			}
		}
		img = 1;
		i = 0;
		while (img)
		{
			if (databuf[i] == 0x41 && databuf[i + 1] == 0x50 && databuf[i + 2] == 0x49 && databuf[i + 3] == 0x43)
			{ //找到位置
				info.pic_size = databuf[i + 4] << 24 | databuf[i + 5] << 16 | databuf[i + 6] << 8 | databuf[i + 7];
				if (databuf[i + 24] == 0xff && databuf[i + 25] == 0xd8 && databuf[i + 26] == 0xff && databuf[i + 27] == 0xe0)
				{
					info.pic_type = 0; //JPG
				}
				if (databuf[i + 24] == 0x89 && databuf[i + 25] == 0x50 && databuf[i + 26] == 0x4e && databuf[i + 27] == 0x47)
				{
					info.pic_type = 1; //PNG
				}
				img = 0;
			}
			else
				i++;
			if (i >= READ_buff_size - 4) //找不到位置
			{
				info.pic_type = 2;
				img = 0;
			}
		}
		if (info.pic_type == 0 && info.mode == 0)
		{
			i += 14 + 20;
			info.pic_local = i;
			info.pic_show = 1;
		}
		else if (info.pic_type == 1 && info.mode == 0)
		{
			i += 14 + 20;
			info.pic_local = i;
			info.pic_show = 2;
		}
		else if (info.pic_type == 2 && info.mode == 0)
		{
			info.pic_local = 0;
			info.pic_show = 3;
		}
		f_lseek(fmp3_pic, info.pic_local); //跳过头
	}
	free(databuf); //释放内存
}

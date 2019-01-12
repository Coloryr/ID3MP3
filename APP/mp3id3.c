#include "mp3id3.h" 
#include "exfuns.h"
#include "ff.h"
#include "malloc.h" 
#include "lcd.h"
#include "text.h"
#include "delay.h"
#include "flash.h"
#include "mp3player.h"
#include "show.h"
#include "includes.h" 
#include "app_start.h" 

u16 UNICODEtoGBK(u16 unicode)  //???????
{
	u16 t[2];
	u16 c;
	u32 i, li, hi;
	u16 n;
	u32 gbk2uni_offset = 0;

	if (unicode < 0x80)c = unicode;//ASCII,直接不用转换.
	else
	{
		//UNICODE 2 GBK   
		gbk2uni_offset = 0;
		/* Unicode to OEMCP */
		hi = ftinfo.ugbksize / 2;//对半开.
		hi = hi / 4 - 1;
		li = 0;
		for (n = 16; n; n--)
		{
			i = li + (hi - li) / 2;
			SPI_Flash_Read((u8*)&t, ftinfo.ugbkaddr + i * 4 + gbk2uni_offset, 4);//读出4个字节  
			if (unicode == t[0]) break;
			if (unicode > t[0])li = i;
			else hi = i;
		}
		c = n ? t[1] : 0;
	}
	return c;
}

void mp3id3()
{
	FIL* fmp3 = 0;
	u16 i = 0;
	u8 res = 0;
	u8 *databuf = 0;
	u8 img = 1;
	u8 temp;
	u16 tag_size;
	u8 code_type;
	u16 a = 0;
	u8 temp1, temp2;
	
	databuf = (u8*)mymalloc(READ_buff_size);
	fmp3 = (FIL*)mymalloc(sizeof(FIL));
	
	if (info.TIT2 == NULL)
		info.TIT2 = (u8*)mymalloc(40);
	if (info.TPE1 == NULL)
		info.TPE1 = (u8*)mymalloc(40);
	for (temp = 0; temp < 40; temp++)
	{
		info.TIT2[temp] = 0;
		info.TPE1[temp] = 0;
	}
	if (databuf == NULL || fmp3 == NULL)//内存申请失败.
		while (1)
		{
			Show_Str(30, 20, 160, 16, "MP3ID3：内存申请失败", 16, 0);
			delay_ms(200);
			LCD_Fill(30, 20, 160, 16, BLACK);//清除显示	     
			delay_ms(200);
		}
	while (f_open(fmp3, (const TCHAR*)info.pname, FA_READ | FA_OPEN_EXISTING))
	{
		Show_Str(30, 20, 160, 16, "MP3ID3：文件读取错误", 16, 0);
		delay_ms(200);
		LCD_Fill(30, 20, 160, 16, BLACK);//清除显示	     
		delay_ms(200);
	}
	res = f_read(fmp3, databuf, 10, (UINT*)&br);//读出mp3id3头
	if(res!=FR_OK)
		return;
	if (databuf[0] == 0x49 && databuf[1] == 0x44 && databuf[2] == 0x33)
	{
		//计算大小
		info.size = databuf[6] & 0x7f | ((databuf[7] & 0x7f) << 7)
			| ((databuf[8] & 0x7f) << 14) | ((databuf[9] & 0x7f) << 21);
		res = f_read(fmp3, databuf, READ_buff_size, (UINT*)&br);
		if(res!=FR_OK)
			return;
		i = 0;
		while (img)							//查找歌名
		{
			if (databuf[i] == 0x54 && databuf[i + 1] == 0x49 && databuf[i + 2] == 0x54 && databuf[i + 3] == 0x32)
			{	//找到位置
				tag_size = databuf[i + 4] << 24
					| databuf[i + 5] << 16
					| databuf[i + 6] << 8
					| databuf[i + 7];
				if (databuf[i + 11] == 0xFE && databuf[i + 12] == 0xFF && databuf[i + 10] == 0x01)
				{
					code_type = 2;							//UTF-16BE
					i += 13;
					tag_size -= 3;
				}
				else if (databuf[i + 11] == 0xFF && databuf[i + 12] == 0xFE && databuf[i + 10] == 0x01)
				{
					code_type = 1;							//UTF-16LE
					i += 13;
					tag_size -= 3;
				}
				else if (databuf[i + 10] == 0x00)
				{
					code_type = 0;							//iso-8859-1
					i += 11;
					tag_size -= 1;
				}
				else if (databuf[i + 10] == 0x03)
				{
					code_type = 3;							//UTF-8
					i += 11;
					tag_size -= 1;
				}
				if (code_type == 3)			//UTF-8
				{
					for (temp = 0; temp < tag_size;)
					{
						a = (databuf[i + temp] << 8) | databuf[i + temp + 1];
						if (a < 0x80) {				/* 7-bit */
							info.TIT2[temp++] = (BYTE)a;
						}
						else {
							if (a < 0x800) {		/* 11-bit */
								info.TIT2[temp++] = (BYTE)(0xC0 | a >> 6);
							}
							else {				/* 16-bit */
								info.TIT2[temp++] = (BYTE)(0xE0 | a >> 12);
								info.TIT2[temp++] = (BYTE)(0x80 | (a >> 6 & 0x3F));
							}
							info.TIT2[temp++] = (BYTE)(0x80 | (a & 0x3F));
						}
					}
				}
				else if (code_type == 2)			//UTF-16BE
				{
					temp2 = 0;
					for (temp = 0; temp < tag_size;)
					{
						a = (databuf[i + temp] << 8) | databuf[i + temp + 1];
						a = UNICODEtoGBK(a);
						temp1 = (a >> 8) & 0xff;
						if (temp1 != 0x00)
						{
							info.TIT2[temp2] = temp1;
							temp2++;
						}
						temp1 = a & 0xff;
						if (temp1 != 0x00)
						{
							info.TIT2[temp2] = temp1;
							temp2++;
						}
						temp += 2;
					}
				}
				else if (code_type == 1)			//UTF-16LE
				{
					temp2 = 0;
					for (temp = 0; temp < tag_size;)
					{
						a = (databuf[i + temp + 1] << 8) | databuf[i + temp];
						a = UNICODEtoGBK(a);
						temp1 = (a >> 8) & 0xff;
						if (temp1 != 0x00)
						{
							info.TIT2[temp2] = temp1;
							temp2++;
						}
						temp1 = a & 0xff;
						if (temp1 != 0x00)
						{
							info.TIT2[temp2] = temp1;
							temp2++;
						}
						temp += 2;
					}
				}
				else if (code_type == 0)			//iso-8859-1
				{
					for (temp = 0; temp < tag_size;)
					{
						temp1 = databuf[i + temp];
						if (temp1 != 0x00)
							info.TIT2[temp++] = temp1;
						else
							temp++;
					}
				}
				img = 0;
			}
			else
				i++;
			if (i >= READ_buff_size - 4)								//找不到位置
			{
				info.TPE1 = 0;
				img = 0;
				code_type = 4;
			}
		}
		img = 1;
		i = 0;
		while (img)							//查找作者
		{
			myfree(info.TPE1);
			if (databuf[i] == 0x54 && databuf[i + 1] == 0x50 && databuf[i + 2] == 0x45 && databuf[i + 3] == 0x31)
			{	//找到位置
				tag_size = databuf[i + 4] << 24
					| databuf[i + 5] << 16
					| databuf[i + 6] << 8
					| databuf[i + 7];
				if (databuf[i + 11] == 0xFE && databuf[i + 12] == 0xFF && databuf[i + 10] == 0x01)
				{
					code_type = 2;							//UTF-16BE
					i += 13;
					tag_size -= 3;
				}
				else if (databuf[i + 11] == 0xFF && databuf[i + 12] == 0xFE && databuf[i + 10] == 0x01)
				{
					code_type = 1;							//UTF-16LE
					i += 13;
					tag_size -= 3;
				}
				else if (databuf[i + 10] == 0x00)
				{
					code_type = 0;							//iso-8859-1
					i += 11;
					tag_size -= 1;
				}
				else if (databuf[i + 10] == 0x03)
				{
					code_type = 3;							//UTF-8
					i += 11;
					tag_size -= 1;
				}
				info.TPE1 = (u8*)mymalloc(tag_size);

				if (code_type == 3)			//UTF-8
				{
					for (temp = 0; temp < tag_size;)
					{
						a = (databuf[i + temp] << 8) | databuf[i + temp + 1];
						if (a < 0x80) {				/* 7-bit */
							info.TPE1[temp++] = (BYTE)a;
						}
						else {
							if (a < 0x800) {		/* 11-bit */
								info.TPE1[temp++] = (BYTE)(0xC0 | a >> 6);
							}
							else {				/* 16-bit */
								info.TPE1[temp++] = (BYTE)(0xE0 | a >> 12);
								info.TPE1[temp++] = (BYTE)(0x80 | (a >> 6 & 0x3F));
							}
							info.TPE1[temp++] = (BYTE)(0x80 | (a & 0x3F));
						}
					}
				}
				else if (code_type == 2)			//UTF-16BE
				{
					temp2 = 0;
					for (temp = 0; temp < tag_size;)
					{
						a = (databuf[i + temp] << 8) | databuf[i + temp + 1];
						a = UNICODEtoGBK(a);
						temp1 = (a >> 8) & 0xff;
						if (temp1 != 0x00)
						{
							info.TPE1[temp2] = temp1;
							temp2++;
						}
						temp1 = a & 0xff;
						if (temp1 != 0x00)
						{
							info.TPE1[temp2] = temp1;
							temp2++;
						}
						temp += 2;
					}
				}
				else if (code_type == 1)			//UTF-16LE
				{
					temp2 = 0;
					for (temp = 0; temp < tag_size;)
					{
						a = (databuf[i + temp + 1] << 8) | databuf[i + temp];
						a = UNICODEtoGBK(a);
						temp1 = (a >> 8) & 0xff;
						if (temp1 != 0x00)
						{
							info.TPE1[temp2] = temp1;
							temp2++;
						}
						temp1 = a & 0xff;
						if (temp1 != 0x00)
						{
							info.TPE1[temp2] = temp1;
							temp2++;
						}
						temp += 2;
					}
				}
				else if (code_type == 0)			//iso-8859-1
				{
					for (temp = 0; temp < tag_size;)
					{
						temp1 = databuf[i + temp];
						if (temp1 != 0x00)
						{
							info.TPE1[temp] = temp1;
							temp++;
						}
						else
							temp++;
					}
				}
				img = 0;
			}
			else
				i++;
			if (i >= READ_buff_size - 4)								//找不到位置
			{
				info.TPE1 = 0;
				code_type = 4;
				img = 0;
			}
		}
		img = 1;
		i = 0;
		while (img)
		{
			if (databuf[i] == 0x41 && databuf[i + 1] == 0x50 && databuf[i + 2] == 0x49 && databuf[i + 3] == 0x43)
			{	//找到位置
				if (databuf[i + 24] == 0xff && databuf[i + 25] == 0xd8 && databuf[i + 26] == 0xff && databuf[i + 27] == 0xe0)
				{
					code_type = 0; //JPG
				}
				if (databuf[i + 24] == 0x89 && databuf[i + 25] == 0x50 && databuf[i + 26] == 0x4e && databuf[i + 27] == 0x47)
				{
					code_type = 1; //PNG
				}
				img = 0;
			}
			else
				i++;
			if (i >= READ_buff_size - 4)								//找不到位置
			{
				code_type = 2;
				code_type = 2;
				img = 0;
			}
		}
		
		if (lcd_bit == 1 && code_type == 0)
		{
			i += 14 + 20;
			info.pic_local = i;
			APP_pic_start();
		}
		else if (code_type == 1 && lcd_bit == 1)
		{
			LCD_Fill(pic_show_x, pic_show_y, pic_show_x + pic_show_size,
				pic_show_y + pic_show_size, BACK_COLOR);
		}
		else if (code_type == 2 && lcd_bit == 1)
		{
			LCD_Fill(pic_show_x, pic_show_y, pic_show_x + pic_show_size,
				pic_show_y + pic_show_size, BACK_COLOR);
		}		    
	}
	f_close(fmp3);
	myfree(fmp3);
	myfree(databuf);						//释放内存	
}








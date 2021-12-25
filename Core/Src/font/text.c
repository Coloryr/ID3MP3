#include "main.h"
#include "text.h"
#include "../flash/flash.h"
#include "font.h"
#include "../lcd/lcd.h"
#include "string.h"

//code 字符指针开始
//从字库中查找出字模
//code 字符串的开始地址,GBK码
//mat  数据存放地址 size*2 bytes大小
void Get_HzMat(unsigned char *code, unsigned char *mat, uint8_t size)
{
	unsigned char qh, ql;
	unsigned char i;
	unsigned long foffset;
	qh = *code;
	ql = *(++code);
	if (qh < 0x81 || ql < 0x40 || ql == 0xff || qh == 0xff) //非 常用汉字
	{
		if (size == 12 || size == 16)
		{
			for (i = 0; i < (size * 2); i++)
				*mat++ = 0x00; //填充满格
			return;			   //结束访问
		}
		else if (size == 24)
		{
			for (i = 0; i < (size * 3); i++)
				*mat++ = 0x00; //填充满格
			return;			   //结束访问
		}
		else
		{
			for (i = 0; i < (size * 4); i++)
				*mat++ = 0x00; //填充满格
			return;			   //结束访问
		}
	}
	if (ql < 0x7f)
		ql -= 0x40; //注意!
	else
		ql -= 0x41;
	qh -= 0x81;

	if (size == 12 || size == 16)
	{
		foffset = ((unsigned long)190 * qh + ql) * (size * 2); //得到字库中的字节偏移量
	}
	else if (size == 24)
	{
		foffset = ((unsigned long)190 * qh + ql) * (size * 3); //得到字库中的字节偏移量
	}
	else
	{
		foffset = ((unsigned long)190 * qh + ql) * (size * 4); //得到字库中的字节偏移量
	}

	if (size == 16)
	{
		SPI_Flash_Read(mat, foffset + ftinfo.f16addr, 32);
	}
	else if (size == 12)
	{
		SPI_Flash_Read(mat, foffset + ftinfo.f12addr, 24);
	}
	else if (size == 24)
	{
		SPI_Flash_Read(mat, foffset + ftinfo.f24addr, 72);
	}
	else
	{
		SPI_Flash_Read(mat, foffset + ftinfo.f32addr, 128);
	}
}
//显示一个指定大小的汉字
//x,y :汉字的坐标
//font:汉字GBK码
//size:字体大小
//mode:0,正常显示,1,叠加显示
void Show_Font(uint16_t x, uint16_t y, uint8_t *font, uint8_t size, uint8_t mode)
{
	uint8_t temp, t, t1, size1;
	uint16_t y0 = y;
	uint8_t dzk[128];
	uint16_t tempcolor;
	if (size != 12 && size != 16 && size != 24 && size != 32)
		return;					//不支持的size
	Get_HzMat(font, dzk, size); //得到相应大小的点阵数据
	if (mode == 0)				//正常显示
	{
		if (size == 12 || size == 16)
		{
			size1 = size * 2;
		}
		else if (size == 24)
		{
			size1 = size * 3;
		}
		else
		{
			size1 = size * 4;
		}
		for (t = 0; t < size1; t++)
		{
			temp = dzk[t]; //得到12数据
			for (t1 = 0; t1 < 8; t1++)
			{
				if (temp & 0x80)
					LCD_DrawPoint(x, y);
				else
				{
					tempcolor = POINT_COLOR;
					POINT_COLOR = BACK_COLOR;
					LCD_DrawPoint(x, y);
					POINT_COLOR = tempcolor; //还原
				}
				temp <<= 1;
				y++;
				if ((y - y0) == size)
				{
					y = y0;
					x++;
					break;
				}
			}
		}
	}
	else //叠加显示
	{
		if (size == 12 || size == 16)
		{
			size1 = size * 2;
		}
		else if (size == 24)
		{
			size1 = size * 3;
		}
		else
		{
			size1 = size * 4;
		}
		for (t = 0; t < size1; t++)
		{
			temp = dzk[t]; //得到12数据
			for (t1 = 0; t1 < 8; t1++)
			{
				if (temp & 0x80)
					LCD_DrawPoint(x, y);
				temp <<= 1;
				y++;
				if ((y - y0) == size)
				{
					y = y0;
					x++;
					break;
				}
			}
		}
	}
}
//在指定位置开始显示一个字符串
//支持自动换行
//(x,y):起始坐标
//width,height:区域
//str  :字符串
//size :字体大小
//mode:0,非叠加方式;1,叠加方式
void Show_Str(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t *str, uint8_t size, uint8_t mode)
{
	uint16_t x0 = x;
	uint16_t y0 = y;
	uint8_t bHz = 0;  //字符或者中文
	while (*str != 0) //数据未结束
	{
		if (!bHz)
		{
			if (*str > 0x80)
				bHz = 1;						 //中文
			else								 //字符
			{									 //字符暂时不管
				if (x > (x0 + width - size / 2)) //换行
				{
					y += size;
					x = x0;
				}
				if (y > (y0 + height - size))
					break;		//越界返回
				if (*str == 13) //换行符号
				{
					y += size;
					x = x0;
					str++;
				}
				else
					LCD_ShowChar(x, y, *str, size, mode); //有效部分写入
				str++;
				x += size / 2; //字符,为全字的一半
			}
		}
		else //中文
		{
			bHz = 0;					 //有汉字库
			if (x > (x0 + width - size)) //换行
			{
				y += size;
				x = x0;
			}
			if (y > (y0 + height - size))
				break;						  //越界返回
			Show_Font(x, y, str, size, mode); //显示这个汉字,空心显示
			str += 2;
			x += size; //下一个汉字偏移
		}
	}
}
//在指定宽度的中间显示字符串
//如果字符长度超过了len,则用Show_Str显示
//len:指定要显示的宽度
void Show_Str_Mid(uint16_t x, uint16_t y, uint8_t *str, uint8_t size, uint8_t len)
{
	uint16_t strlenth = 0;
	strlenth = strlen((const char *)str);
	strlenth *= size / 2;
	if (strlenth > len)
		Show_Str(x, y, 240, 320, str, size, 1);
	else
	{
		strlenth = (len - strlenth) / 2;
		Show_Str(strlenth + x, y, 240, 320, str, size, 1);
	}
}

#ifndef __IMAGE2LCD_H
#define __IMAGE2LCD_H
#include "sys.h"

//4096色/16位真彩色/18位真彩色/24位真彩色/32位真彩色
//图像数据头结构体  
__packed typedef struct _HEADCOLOR
{
	unsigned char scan;
	unsigned char gray;
	unsigned short w;
	unsigned short h;
	unsigned char is565;
	unsigned char rgb;
}HEADCOLOR;
//scan: 扫描模式 	    
//Bit7: 0:自左至右扫描，1:自右至左扫描。 
//Bit6: 0:自顶至底扫描，1:自底至顶扫描。 
//Bit5: 0:字节内象素数据从高位到低位排列，1:字节内象素数据从低位到高位排列。 
//Bit4: 0:WORD类型高低位字节顺序与PC相同，1:WORD类型高低位字节顺序与PC相反。 
//Bit3~2: 保留。 
//Bit1~0: [00]水平扫描，[01]垂直扫描，[10]数据水平,字节垂直，[11]数据垂直,字节水平。 
//gray: 灰度值 
//   灰度值，1:单色，2:四灰，4:十六灰，8:256色，12:4096色，16:16位彩色，24:24位彩色，32:32位彩色。
//w: 图像的宽度。	 
//h: 图像的高度。
//is565: 在4096色模式下为0表示使用[16bits(WORD)]格式，此时图像数据中每个WORD表示一个象素；为1表示使用[12bits(连续字节流)]格式，此时连续排列的每12Bits代表一个象素。
//在16位彩色模式下为0表示R G B颜色分量所占用的位数都为5Bits，为1表示R G B颜色分量所占用的位数分别为5Bits,6Bits,5Bits。
//在18位彩色模式下为0表示"6Bits in Low Byte"，为1表示"6Bits in High Byte"。
//在24位彩色和32位彩色模式下is565无效。	  
//rgb: 描述R G B颜色分量的排列顺序，rgb中每2Bits表示一种颜色分量，[00]表示空白，[01]表示Red，[10]表示Green，[11]表示Blue。

void image_display(u16 x,u16 y,u8 * imgx);//在指定位置显示图片
void image_show(u16 xsta,u16 ysta,u16 xend,u16 yend,u8 scan,u8 *p);//在指定区域开始显示图片
u16 image_getcolor(u8 mode,u8 *str);//获取颜色


#endif














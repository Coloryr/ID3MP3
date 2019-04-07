#include "sys.h"
#include "piclib.h"

_pic_info picinfo; //图片信息
_pic_phy pic_phy;  //图片显示物理接口
//////////////////////////////////////////////////////////////////////////
//lcd.h没有提供划横线函数,需要自己实现
void piclib_draw_hline(u16 x0, u16 y0, u16 len, u16 color)
{
	if ((len == 0) || (x0 > lcddev.width) || (y0 > lcddev.height))
		return;
	LCD_Fill(x0, y0, x0 + len - 1, y0, color);
}
//填充颜色
//x,y:起始坐标
//width，height：宽度和高度。
//*color：颜色数组
void piclib_fill_color(u16 x, u16 y, u16 width, u16 height, u16 *color)
{
	LCD_Color_Fill(x, y, x + width - 1, y + height - 1, color);
}
//////////////////////////////////////////////////////////////////////////
//画图初始化,在画图之前,必须先调用此函数
//指定画点/读点
void piclib_init(void)
{
	pic_phy.read_point = LCD_ReadPoint;		 //读点函数实现
	pic_phy.draw_point = LCD_Fast_DrawPoint; //画点函数实现
	pic_phy.fill = LCD_Fill;				 //填充函数实现
	pic_phy.draw_hline = piclib_draw_hline;  //画线函数实现
	pic_phy.fillcolor = piclib_fill_color;   //颜色填充函数实现

	picinfo.lcdwidth = lcddev.width;   //得到LCD的宽度像素
	picinfo.lcdheight = lcddev.height; //得到LCD的高度像素

	picinfo.ImgWidth = 0;  //初始化宽度为0
	picinfo.ImgHeight = 0; //初始化高度为0
	picinfo.Div_Fac = 0;   //初始化缩放系数为0
	picinfo.S_Height = 0;  //初始化设定的高度为0
	picinfo.S_Width = 0;   //初始化设定的宽度为0
	picinfo.S_XOFF = 0;	//初始化x轴的偏移量为0
	picinfo.S_YOFF = 0;	//初始化y轴的偏移量为0
	picinfo.staticx = 0;   //初始化当前显示到的x坐标为0
	picinfo.staticy = 0;   //初始化当前显示到的y坐标为0
}
//快速ALPHA BLENDING算法.
//src:源颜色
//dst:目标颜色
//alpha:透明程度(0~32)
//返回值:混合后的颜色.
u16 piclib_alpha_blend(u16 src, u16 dst, u8 alpha)
{
	u32 src2;
	u32 dst2;
	//Convert to 32bit |-----GGGGGG-----RRRRR------BBBBB|
	src2 = ((src << 16) | src) & 0x07E0F81F;
	dst2 = ((dst << 16) | dst) & 0x07E0F81F;
	//Perform blending R:G:B with alpha in range 0..32
	//Note that the reason that alpha may not exceed 32 is that there are only
	//5bits of space between each R:G:B value, any higher value will overflow
	//into the next component and deliver ugly result.
	dst2 = ((((dst2 - src2) * alpha) >> 5) + src2) & 0x07E0F81F;
	return (dst2 >> 16) | dst2;
}
//初始化智能画点
//内部调用
void ai_draw_init(void)
{
	float temp, temp1;
	temp = (float)picinfo.S_Width / picinfo.ImgWidth;
	temp1 = (float)picinfo.S_Height / picinfo.ImgHeight;
	if (temp < temp1)
		temp1 = temp; //取较小的那个
	if (temp1 > 1)
		temp1 = 1;
	//使图片处于所给区域的中间
	picinfo.S_XOFF += (picinfo.S_Width - temp1 * picinfo.ImgWidth) / 2;
	picinfo.S_YOFF += (picinfo.S_Height - temp1 * picinfo.ImgHeight) / 2;
	temp1 *= 8192; //扩大8192倍
	picinfo.Div_Fac = temp1;
	picinfo.staticx = 0xffff;
	picinfo.staticy = 0xffff; //放到一个不可能的值上面
}
//判断这个像素是否可以显示
//(x,y) :像素原始坐标
//chg   :功能变量.
//返回值:0,不需要显示.1,需要显示
u8 is_element_ok(u16 x, u16 y, u8 chg)
{
	if (x != picinfo.staticx || y != picinfo.staticy)
	{
		if (chg == 1)
		{
			picinfo.staticx = x;
			picinfo.staticy = y;
		}
		return 1;
	}
	else
		return 0;
}

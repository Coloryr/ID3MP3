#include "guix.h"
#include "font/text.h"
#include "lcd/lcd.h"
#include "touch/touch.h"

//此处必须在外部申明asc2_1206和asc2_1608;
extern const unsigned char asc2_1206[95][12];
extern const unsigned char asc2_1608[95][16];
extern const unsigned char asc2_2412[95][36];
extern const unsigned char asc2_3216[95][64];
///////////////////////////////////////////////////////////////////////////

_gui_phy gui_phy; //gui物理层接口
//////////////////////////统一标准函数//////////////////////////////
//画点函数
//x0,y0:坐标
//color:颜色
void _gui_draw_point(uint16_t x0, uint16_t y0, uint16_t color)
{
	uint16_t tempcolor = POINT_COLOR;
	POINT_COLOR = color;
	LCD_DrawPoint(x0, y0);
	POINT_COLOR = tempcolor;
}

////////////////////////////////////////////////////////////////////
//gui初始化
void gui_init(void)
{
	gui_phy.read_point = LCD_ReadPoint;
	gui_phy.draw_point = LCD_Fast_DrawPoint; //快速画点
	gui_phy.fill = LCD_Fill;
	gui_phy.colorfill = LCD_Color_Fill;
	gui_phy.back_color = BACK_COLOR;
	gui_phy.lcdwidth = lcddev.width;
	gui_phy.lcdheight = lcddev.height;
	//设置listbox/filelistbox参数
}
//颜色转换
//rgb:32位颜色
//返回值:16位色.
uint16_t gui_color_chg(uint32_t rgb)
{
	uint16_t r, g, b;
	r = rgb >> 19;
	g = (rgb >> 10) & 0x3f;
	b = (rgb >> 3) & 0x1f;
	return ((r << 11) | (g << 5) | b);
}
//将rgb332颜色转换为rgb565.
//rgb332:rgb332格式数据
//返回值:rgb565格式数据.
uint16_t gui_rgb332torgb565(uint16_t rgb332)
{
	uint16_t rgb565 = 0x18E7;
	rgb565 |= (rgb332 & 0x03) << 3;
	rgb565 |= (rgb332 & 0x1c) << 6;
	rgb565 |= (rgb332 & 0xe0) << 8;
	return rgb565;
}
//m^n函数
long long gui_pow(uint8_t m, uint8_t n)
{
	long long result = 1;
	while (n--)
		result *= m;
	return result;
}
//两个数的差的绝对值
uint32_t gui_disabs(uint32_t x1, uint32_t x2)
{
	return x1 > x2 ? x1 - x2 : x2 - x1;
}
//对指定区域进行alphablend
//x,y,width,height:区域设置
//color:alphablend的颜色
//aval:透明度(0~32)
void gui_alphablend_area(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color, uint8_t aval)
{
	uint16_t i, j;
	uint16_t tempcolor;
	for (i = 0; i < width; i++)
	{
		for (j = 0; j < height; j++)
		{
			tempcolor = gui_phy.read_point(x + i, y + j);
			tempcolor = gui_alpha_blend565(tempcolor, color, aval);
			gui_phy.draw_point(x + i, y + j, tempcolor);
		}
	}
}

//画大点函数
//x0,y0:坐标
//color:颜色
//以(x0,y0)为中心,画一个9个点的大点
void gui_draw_bigpoint(uint16_t x0, uint16_t y0, uint16_t color)
{
	uint16_t i, j;
	uint16_t x, y;
	if (x0 >= 1)
		x = x0 - 1;
	else
		x = x0;
	if (y0 >= 1)
		y = y0 - 1;
	else
		y = y0;
	for (i = y; i < y0 + 2; i++)
	{
		for (j = x; j < x0 + 2; j++)
			gui_phy.draw_point(j, i, color);
	}
}
//画任意线
//x0,y0:起点 x1,y1:终点
//color:颜色
void gui_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
	uint16_t t;
	int xerr = 0, yerr = 0, delta_x, delta_y, distance;
	int incx, incy, uRow, uCol;

	delta_x = x1 - x0; //计算坐标增量
	delta_y = y1 - y0;
	uRow = x0;
	uCol = y0;
	if (delta_x > 0)
		incx = 1; //设置单步方向
	else if (delta_x == 0)
		incx = 0; //垂直线
	else
	{
		incx = -1;
		delta_x = -delta_x;
	}
	if (delta_y > 0)
		incy = 1;
	else if (delta_y == 0)
		incy = 0; //水平线
	else
	{
		incy = -1;
		delta_y = -delta_y;
	}
	if (delta_x > delta_y)
		distance = delta_x; //选取基本增量坐标轴
	else
		distance = delta_y;
	for (t = 0; t <= distance + 1; t++) //画线输出
	{
		gui_phy.draw_point(uRow, uCol, color); //画点
		xerr += delta_x;
		yerr += delta_y;
		if (xerr > distance)
		{
			xerr -= distance;
			uRow += incx;
		}
		if (yerr > distance)
		{
			yerr -= distance;
			uCol += incy;
		}
	}
}
//画一条粗线(方法0)
//(x1,y1),(x2,y2):线条的起止坐标
//size：线条的粗细程度
//color：线条的颜色
void gui_draw_bline(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t size, uint16_t color)
{
	uint16_t t;
	int xerr = 0, yerr = 0, delta_x, delta_y, distance;
	int incx, incy, uRow, uCol;
	if (x1 < size || x2 < size || y1 < size || y2 < size)
		return;
	if (size == 0)
	{
		gui_phy.draw_point(x1, y1, color);
		return;
	}
	delta_x = x2 - x1; //计算坐标增量
	delta_y = y2 - y1;
	uRow = x1;
	uCol = y1;
	if (delta_x > 0)
		incx = 1; //设置单步方向
	else if (delta_x == 0)
		incx = 0; //垂直线
	else
	{
		incx = -1;
		delta_x = -delta_x;
	}
	if (delta_y > 0)
		incy = 1;
	else if (delta_y == 0)
		incy = 0; //水平线
	else
	{
		incy = -1;
		delta_y = -delta_y;
	}
	if (delta_x > delta_y)
		distance = delta_x; //选取基本增量坐标轴
	else
		distance = delta_y;
	for (t = 0; t <= distance + 1; t++) //画线输出
	{
		gui_fill_circle(uRow, uCol, size, color); //画点
		xerr += delta_x;
		yerr += delta_y;
		if (xerr > distance)
		{
			xerr -= distance;
			uRow += incx;
		}
		if (yerr > distance)
		{
			yerr -= distance;
			uCol += incy;
		}
	}
}

//画一条粗线(方法1)
//x0,y0:起点 x1,y1:终点
//size:线粗细,仅支持:0~2.
//color:颜色
void gui_draw_bline1(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t size, uint16_t color)
{
	uint16_t t;
	int xerr = 0, yerr = 0, delta_x, delta_y, distance;
	int incx, incy, uRow, uCol;

	delta_x = x1 - x0; //计算坐标增量
	delta_y = y1 - y0;
	uRow = x0;
	uCol = y0;
	if (delta_x > 0)
		incx = 1; //设置单步方向
	else if (delta_x == 0)
		incx = 0; //垂直线
	else
	{
		incx = -1;
		delta_x = -delta_x;
	}
	if (delta_y > 0)
		incy = 1;
	else if (delta_y == 0)
		incy = 0; //水平线
	else
	{
		incy = -1;
		delta_y = -delta_y;
	}
	if (delta_x > delta_y)
		distance = delta_x; //选取基本增量坐标轴
	else
		distance = delta_y;
	for (t = 0; t <= distance + 1; t++) //画线输出
	{
		if (size == 0)
			LCD_Fast_DrawPoint(uRow, uCol, color); //画点
		if (size == 1)
		{
			LCD_Fast_DrawPoint(uRow, uCol, color);		   //画点
			LCD_Fast_DrawPoint(uRow + 1, uCol, color);	   //画点
			LCD_Fast_DrawPoint(uRow, uCol + 1, color);	   //画点
			LCD_Fast_DrawPoint(uRow + 1, uCol + 1, color); //画点
		}
		if (size == 2)
		{
			LCD_Fast_DrawPoint(uRow, uCol, color);		   //画点
			LCD_Fast_DrawPoint(uRow + 1, uCol, color);	   //画点
			LCD_Fast_DrawPoint(uRow, uCol + 1, color);	   //画点
			LCD_Fast_DrawPoint(uRow + 1, uCol + 1, color); //画点
			LCD_Fast_DrawPoint(uRow - 1, uCol + 1, color); //画点
			LCD_Fast_DrawPoint(uRow + 1, uCol - 1, color); //画点
			LCD_Fast_DrawPoint(uRow - 1, uCol - 1, color); //画点
			LCD_Fast_DrawPoint(uRow - 1, uCol, color);	   //画点
			LCD_Fast_DrawPoint(uRow, uCol - 1, color);	   //画点
		}
		xerr += delta_x;
		yerr += delta_y;
		if (xerr > distance)
		{
			xerr -= distance;
			uRow += incx;
		}
		if (yerr > distance)
		{
			yerr -= distance;
			uCol += incy;
		}
	}
}
//画垂直线
//x0,y0:坐标
//len:线长度
//color:颜色
void gui_draw_vline(uint16_t x0, uint16_t y0, uint16_t len, uint16_t color)
{
	if ((len == 0) || (x0 > gui_phy.lcdwidth) || (y0 > gui_phy.lcdheight))
		return;
	gui_phy.fill(x0, y0, x0, y0 + len - 1, color);
}
//画水平线
//x0,y0:坐标
//len:线长度
//color:颜色
void gui_draw_hline(uint16_t x0, uint16_t y0, uint16_t len, uint16_t color)
{
	if ((len == 0) || (x0 > gui_phy.lcdwidth) || (y0 > gui_phy.lcdheight))
		return;
	gui_phy.fill(x0, y0, x0 + len - 1, y0, color);
}
//填充一个色块.
//x0,y0,width,height:坐标及填充尺寸
//ctbl:颜色表,横向填充时width,纵向填充时大小为height
//mode:填充模式(纵向:从左到右,从上到下填充;横向:从上到下,从左到右填充)
//[7:1]:保留
//[bit0]:0,纵向;1,横向
void gui_fill_colorblock(uint16_t x0, uint16_t y0, uint16_t width, uint16_t height, uint16_t *ctbl, uint8_t mode)
{
	uint16_t i;
	if (height == 0 || width == 0)
		return;
	if (mode & 0x01) //横向
	{
		for (i = 0; i < width; i++)
		{
			gui_draw_vline(x0 + i, y0, height - 1, ctbl[i]);
		}
	}
	else
	{
		for (i = 0; i < height; i++)
		{
			gui_draw_hline(x0, y0 + i, width - 1, ctbl[i]);
		}
	}
}

//将一种颜色平滑过渡到另外一种颜色
//srgb,ergb:起始颜色和终止颜色.
//cbuf:颜色缓存区(≥len)
//len:过渡级数(就是输出的颜色值总数)
void gui_smooth_color(uint32_t srgb, uint32_t ergb, uint16_t *cbuf, uint16_t len)
{
	uint8_t sr, sg, sb;
	short dr, dg, db;
	uint16_t r, g, b;
	uint16_t i = 0;
	sr = srgb >> 16;
	sg = (srgb >> 8) & 0XFF;
	sb = srgb & 0XFF;
	dr = (ergb >> 16) - sr;
	dg = ((ergb >> 8) & 0XFF) - sg;
	db = (ergb & 0XFF) - sb;
	for (i = 0; i < len; i++)
	{
		r = sr + (dr * i) / len;
		g = sg + (dg * i) / len;
		b = sb + (db * i) / len;
		r >>= 3;
		g >>= 2;
		b >>= 3;
		cbuf[i] = ((r << 11) | (g << 5) | b);
	}
}
//画一条平滑过渡的彩色矩形(或线)
//x,y,width,height:矩形的坐标尺寸
//srgb,ergb:起始和终止的颜色
void gui_draw_smooth_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t srgb, uint32_t ergb)
{
	uint16_t i, j;
	uint16_t *colortbl = NULL;
	colortbl = (uint16_t *)gui_memin_malloc(width * 2); //分配内存
	if (colortbl == NULL)
		return;									   //内存申请失败
	gui_smooth_color(srgb, ergb, colortbl, width); //获得颜色组
	for (i = 0; i < width; i++)
	{
		for (j = 0; j < height; j++)
		{
			gui_phy.draw_point(x + i, y + j, colortbl[i]); //画点
		}
	}
	gui_memin_free(colortbl);
}
//画矩形
//x0,y0:矩形的左上角坐标
//width,height:矩形的尺寸
//color:颜色
void gui_draw_rectangle(uint16_t x0, uint16_t y0, uint16_t width, uint16_t height, uint16_t color)
{
	gui_draw_hline(x0, y0, width, color);
	gui_draw_hline(x0, y0 + height - 1, width, color);
	gui_draw_vline(x0, y0, height, color);
	gui_draw_vline(x0 + width - 1, y0, height, color);
}
//画圆角矩形/填充圆角矩形
//x,y,width,height:圆角矩形的位置和尺寸
//r:圆角的半径.
//mode:0,画矩形框;1,填充矩形.
//upcolor:上半部分颜色
//downcolor:下半部分颜色
void gui_draw_arcrectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t r, uint8_t mode, uint16_t upcolor, uint16_t downcolor)
{
	uint16_t btnxh = 0;
	if (height % 2)
		btnxh = height + 1; //基偶数处理
	else
		btnxh = height;
	if (mode) //填充
	{
		gui_fill_rectangle(x + r, y, width - 2 * r, btnxh / 2, upcolor);			   //中上
		gui_fill_rectangle(x + r, y + btnxh / 2, width - 2 * r, btnxh / 2, downcolor); //中下
		gui_fill_rectangle(x, y + r, r, btnxh / 2 - r, upcolor);					   //左上
		gui_fill_rectangle(x, y + btnxh / 2, r, btnxh / 2 - r, downcolor);			   //左下
		gui_fill_rectangle(x + width - r, y + r, r, btnxh / 2 - r, upcolor);		   //右上
		gui_fill_rectangle(x + width - r, y + btnxh / 2, r, btnxh / 2 - r, downcolor); //右下
	}
	else
	{
		gui_draw_hline(x + r, y, width - 2 * r, upcolor);						//上
		gui_draw_hline(x + r, y + btnxh - 1, width - 2 * r, downcolor);			//下
		gui_draw_vline(x, y + r, btnxh / 2 - r, upcolor);						//左上
		gui_draw_vline(x, y + btnxh / 2, btnxh / 2 - r, downcolor);				//左下
		gui_draw_vline(x + width - 1, y + r, btnxh / 2 - r, upcolor);			//右上
		gui_draw_vline(x + width - 1, y + btnxh / 2, btnxh / 2 - r, downcolor); //右下
	}
	gui_draw_arc(x, y, x + r, y + r, x + r, y + r, r, upcolor, mode);																//左上
	gui_draw_arc(x, y + btnxh - r, x + r, y + btnxh - 1, x + r, y + btnxh - r - 1, r, downcolor, mode);								//左下
	gui_draw_arc(x + width - r, y, x + width, y + r, x + width - r - 1, y + r, r, upcolor, mode);									//右上
	gui_draw_arc(x + width - r, y + btnxh - r, x + width, y + btnxh - 1, x + width - r - 1, y + btnxh - r - 1, r, downcolor, mode); //右下
}

//填充矩形
//x0,y0:矩形的左上角坐标
//width,height:矩形的尺寸
//color:颜色
void gui_fill_rectangle(uint16_t x0, uint16_t y0, uint16_t width, uint16_t height, uint16_t color)
{
	if (width == 0 || height == 0)
		return; //非法.
	gui_phy.fill(x0, y0, x0 + width - 1, y0 + height - 1, color);
}
//画实心圆
//x0,y0:坐标
//r半径
//color:颜色
void gui_fill_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color)
{
	uint32_t i;
	uint32_t imax = ((uint32_t)r * 707) / 1000 + 1;
	uint32_t sqmax = (uint32_t)r * (uint32_t)r + (uint32_t)r / 2;
	uint32_t x = r;
	gui_draw_hline(x0 - r, y0, 2 * r, color);
	for (i = 1; i <= imax; i++)
	{
		if ((i * i + x * x) > sqmax)
		{
			// draw lines from outside
			if (x > imax)
			{
				gui_draw_hline(x0 - i + 1, y0 + x, 2 * (i - 1), color);
				gui_draw_hline(x0 - i + 1, y0 - x, 2 * (i - 1), color);
			}
			x--;
		}
		// draw lines from inside (center)
		gui_draw_hline(x0 - x, y0 + i, 2 * x, color);
		gui_draw_hline(x0 - x, y0 - i, 2 * x, color);
	}
}
//在区域内画点
//(sx,sy),(ex,ey):设定的显示范围
//(x,y):点坐标
//color:颜色
void gui_draw_expoint(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t x, uint16_t y, uint16_t color)
{
	uint16_t tempcolor;
	if (x <= ex && x >= sx && y <= ey && y >= sy)
	{
		tempcolor = POINT_COLOR;
		POINT_COLOR = color;
		LCD_DrawPoint(x, y);
		POINT_COLOR = tempcolor;
	}
}
//画8点(Bresenham算法)
//(sx,sy),(ex,ey):设定的显示范围
//(rx,ry,a,b):参数
//color:颜色
void gui_draw_circle8(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t rx, uint16_t ry, int a, int b, uint16_t color)
{
	gui_draw_expoint(sx, sy, ex, ey, rx + a, ry - b, color);
	gui_draw_expoint(sx, sy, ex, ey, rx + b, ry - a, color);
	gui_draw_expoint(sx, sy, ex, ey, rx + b, ry + a, color);
	gui_draw_expoint(sx, sy, ex, ey, rx + a, ry + b, color);
	gui_draw_expoint(sx, sy, ex, ey, rx - a, ry + b, color);
	gui_draw_expoint(sx, sy, ex, ey, rx - b, ry + a, color);
	gui_draw_expoint(sx, sy, ex, ey, rx - b, ry - a, color);
	gui_draw_expoint(sx, sy, ex, ey, rx - a, ry - b, color);
}
//在指定位置画一个指定大小的圆
//(rx,ry):圆心
//(sx,sy),(ex,ey):设定的显示范围
//r    :半径
//color:颜色
//mode :0,不填充;1,填充
void gui_draw_arc(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t rx, uint16_t ry, uint16_t r, uint16_t color, uint8_t mode)
{
	int a, b, c;
	int di;
	a = 0;
	b = r;
	di = 3 - (r << 1); //判断下个点位置的标志
	while (a <= b)
	{
		if (mode)
			for (c = a; c <= b; c++)
				gui_draw_circle8(sx, sy, ex, ey, rx, ry, a, c, color); //画实心圆
		else
			gui_draw_circle8(sx, sy, ex, ey, rx, ry, a, b, color); //画空心圆
		a++;
		//使用Bresenham算法画圆
		if (di < 0)
			di += 4 * a + 6;
		else
		{
			di += 10 + 4 * (a - b);
			b--;
		}
	}
}
//画椭圆
//x0,y0:坐标
//rx:x方向半径
//ry:y方向半径
//color:椭圆的颜色
void gui_draw_ellipse(uint16_t x0, uint16_t y0, uint16_t rx, uint16_t ry, uint16_t color)
{
	uint32_t OutConst, Sum, SumY;
	uint16_t x, y;
	uint16_t xOld;
	uint32_t _rx = rx;
	uint32_t _ry = ry;
	if (rx > x0 || ry > y0)
		return;												   //非法.
	OutConst = _rx * _rx * _ry * _ry + (_rx * _rx * _ry >> 1); // Constant as explaint above
	// To compensate for rounding
	xOld = x = rx;
	for (y = 0; y <= ry; y++)
	{
		if (y == ry)
			x = 0;
		else
		{
			SumY = ((uint32_t)(rx * rx)) * ((uint32_t)(y * y)); // Does not change in loop
			while (Sum = SumY + ((uint32_t)(ry * ry)) * ((uint32_t)(x * x)), (x > 0) && (Sum > OutConst))
				x--;
		}
		// Since we draw lines, we can not draw on the first iteration
		if (y)
		{
			gui_draw_line(x0 - xOld, y0 - y + 1, x0 - x, y0 - y, color);
			gui_draw_line(x0 - xOld, y0 + y - 1, x0 - x, y0 + y, color);
			gui_draw_line(x0 + xOld, y0 - y + 1, x0 + x, y0 - y, color);
			gui_draw_line(x0 + xOld, y0 + y - 1, x0 + x, y0 + y, color);
		}
		xOld = x;
	}
}
//填充椭圆
//x0,y0:坐标
//rx:x方向半径
//ry:y方向半径
//color:椭圆的颜色
void gui_fill_ellipse(uint16_t x0, uint16_t y0, uint16_t rx, uint16_t ry, uint16_t color)
{
	uint32_t OutConst, Sum, SumY;
	uint16_t x, y;
	uint32_t _rx = rx;
	uint32_t _ry = ry;
	OutConst = _rx * _rx * _ry * _ry + (_rx * _rx * _ry >> 1); // Constant as explaint above
	// To compensate for rounding
	x = rx;
	for (y = 0; y <= ry; y++)
	{
		SumY = ((uint32_t)(rx * rx)) * ((uint32_t)(y * y)); // Does not change in loop
		while (Sum = SumY + ((uint32_t)(ry * ry)) * ((uint32_t)(x * x)), (x > 0) && (Sum > OutConst))
			x--;
		gui_draw_hline(x0 - x, y0 + y, 2 * x, color);
		if (y)
			gui_draw_hline(x0 - x, y0 - y, 2 * x, color);
	}
}
//快速ALPHA BLENDING算法.
//src:源颜色
//dst:目标颜色
//alpha:透明程度(0~32)
//返回值:混合后的颜色.
uint16_t gui_alpha_blend565(uint16_t src, uint16_t dst, uint8_t alpha)
{
	uint32_t src2;
	uint32_t dst2;
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
/////////////////////////////////////////////////////////////////////////////////////////////

//在中间位置显示一个字符串
//x,y,width,height:显示区域坐标.
//color:文字颜色.
//size:文字大小
//str:字符串
void gui_show_strmid(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color, uint8_t size, uint8_t *str)
{
	uint16_t xoff = 0, yoff = 0;
	uint16_t strlenth;
	uint16_t strwidth;
	if (str == NULL)
		return;
	strlenth = strlen((const char *)str); //得到字符串长度
	strwidth = strlenth * size / 2;		  //字符串显示占用宽度
	if (height > size)
		yoff = (height - size) / 2;
	if (strwidth <= width) //字符串没超过宽度
	{
		xoff = (width - strwidth) / 2;
	}
	gui_show_ptstr(x + xoff, y + yoff, x + width - 1, y + height - 1, 0, color, size, str, 1);
}

//在指定地址开始显示一个ASCII字符
//x,y:显示开始坐标.
//xend,yend:x,y 方向的终点坐标
//offset:开始显示的偏移
//color:文字颜色.
//size:文字大小
//chr:字符
//mode:0,非叠加显示;1,叠加显示.2,大点叠加(只适用于叠加模式)
void gui_show_ptchar(uint16_t x, uint16_t y, uint16_t xend, uint16_t yend, uint16_t offset, uint16_t color, uint16_t size, uint8_t chr, uint8_t mode)
{
	uint8_t temp;
	uint8_t t1, t;
	uint16_t tempoff;
	uint16_t y0 = y;
	uint16_t csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2); //得到字体一个字符对应点阵集所占的字节数
	if (chr > ' ')
		chr = chr - ' '; //得到偏移后的值
	else
		chr = 0; //小于空格的一律用空格代替,比如TAB键(键值为9)
	for (t = 0; t < csize; t++)
	{
		switch (size)
		{
		case 12:
			temp = asc2_1206[chr][t]; //调用1206字体
			break;
		case 16:
			temp = asc2_1608[chr][t]; //调用1608字体
			break;
		case 24:
			temp = asc2_2412[chr][t]; //调用2412字体
			break;
		case 28:
			temp = asc2_3216[chr][t]; //调用2814字体
			break;
		default: //不支持的字体
			return;
		}
		tempoff = offset;
		if (x > xend)
			return;		  //超区域了
		if (tempoff == 0) //偏移地址到了
		{
			for (t1 = 0; t1 < 8; t1++)
			{
				if (y <= yend)
				{
					if (temp & 0x80)
					{
						if (mode == 0x02)
							gui_draw_bigpoint(x, y, color);
						else
							gui_phy.draw_point(x, y, color);
					}
					else if (mode == 0)
						gui_phy.draw_point(x, y, gui_phy.back_color);
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
		else
		{
			y += 8;
			if ((y - y0) >= size) //大于一个字的高度了
			{
				y = y0; //y坐标归零
				tempoff--;
			}
		}
	}
}
//显示len个数字
//x,y :起点坐标
//len :数字的位数
//color:颜色
//size:字体大小
//num:数值(0~2^64);
//mode:模式.
//[7]:0,不填充;1,填充0.
//[3:0]:0,非叠加显示;1,叠加显示.2,大点叠加(只适用于叠加模式)
void gui_show_num(uint16_t x, uint16_t y, uint8_t len, uint16_t color, uint8_t size, long long num, uint8_t mode)
{
	uint8_t t, temp;
	uint8_t enshow = 0;
	for (t = 0; t < len; t++)
	{
		temp = (num / gui_pow(10, len - t - 1)) % 10;
		if (enshow == 0 && t < (len - 1))
		{
			if (temp == 0)
			{
				if (mode & 0X80)
					gui_show_ptchar(x + (size / 2) * t, y, gui_phy.lcdwidth, gui_phy.lcdheight, 0, color, size, '0', mode & 0xf); //填充0
				else
					gui_show_ptchar(x + (size / 2) * t, y, gui_phy.lcdwidth, gui_phy.lcdheight, 0, color, size, ' ', mode & 0xf);
				continue;
			}
			else
				enshow = 1;
		}
		gui_show_ptchar(x + (size / 2) * t, y, gui_phy.lcdwidth, gui_phy.lcdheight, 0, color, size, temp + '0', mode & 0xf);
	}
}
//将数字转为字符串
//str:字符串存放地址.
//num:数字
uint8_t *gui_num2str(uint8_t *str, uint32_t num)
{
	uint8_t t, temp;
	uint8_t enrec = 0;
	for (t = 0; t < 10; t++)
	{
		temp = (num / gui_pow(10, 10 - t - 1)) % 10;
		if (enrec == 0 && t < 9)
		{
			if (temp == 0)
				continue; //还不需要记录
			else
				enrec = 1; //需要记录了.
		}
		*str = temp + '0';
		str++;
	}
	*str = '\0'; //加入结束符.
	return str;
}

//在指定地址开始显示一个汉字
//x,y:显示开始坐标.
//xend,yend:x,y 方向的终点坐标
//offset:开始显示的偏移
//color:文字颜色.
//size:文字大小
//chr:汉字内码(高字节在前,低字节在后)
//mode:0,非叠加显示;1,叠加显示,2,大点叠加(只适用于叠加模式)
void gui_show_ptfont(uint16_t x, uint16_t y, uint16_t xend, uint16_t yend, uint16_t offset, uint16_t color, uint16_t size, uint8_t *chr, uint8_t mode)
{
	uint8_t temp;
	uint8_t t1, t;
	uint16_t tempoff;
	uint16_t y0 = y;
	uint8_t dzk[72];											//24*24字体,最大要72个字节作为点阵存储
	uint8_t csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size); //得到字体一个字符对应点阵集所占的字节数
	if (size != 12 && size != 16 && size != 24)
		return;				   //不支持的size/内存分配失败
	Get_HzMat(chr, dzk, size); //得到相应大小的点阵数据
	tempoff = offset;
	for (t = 0; t < csize; t++)
	{
		if (x > xend)
			break;		  //到达终点坐标
		temp = dzk[t];	  //得到点阵数据
		if (tempoff == 0) //偏移地址到了
		{
			for (t1 = 0; t1 < 8; t1++)
			{
				if (y <= yend)
				{
					if (temp & 0x80)
					{
						if (mode == 0x02)
							gui_draw_bigpoint(x, y, color);
						else
							gui_phy.draw_point(x, y, color);
					}
					else if (mode == 0)
						gui_phy.draw_point(x, y, gui_phy.back_color);
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
		else
		{
			y += 8;
			if ((y - y0) >= size) //大于一个字的高度了
			{
				y = y0; //y坐标归零
				tempoff--;
			}
		}
	}
}
//在指定位置开始,显示指定长度范围的字符串.
//x,y:显示开始坐标.
//xend:x方向的终止坐标
//offset:开始显示的偏移
//color:文字颜色.
//size:文字大小
//str:字符串
//mode:0,非叠加显示;1,叠加显示,2,大点叠加(只适用于叠加模式)
void gui_show_ptstr(uint16_t x, uint16_t y, uint16_t xend, uint16_t yend, uint16_t offset, uint16_t color, uint8_t size, uint8_t *str, uint8_t mode)
{
	uint8_t bHz = 0;	//字符或者中文
	uint8_t EnDisp = 0; //使能显示
	while (*str != 0)	//数据未结束
	{
		if (!bHz)
		{
			if (*str > 0x80)
				bHz = 1; //中文
			else		 //字符
			{
				if (EnDisp == 0) //还未使能显示
				{
					if (offset >= size / 2) //超过了一个字符
					{
						offset -= size / 2; //减少一个字符的偏移
					}
					else //未超过一个字符的偏移
					{
						offset = offset % (size / 2); //得到字符偏移量
						EnDisp = 1;					  //可以开始显示了
					}
				}
				if (EnDisp == 1) //使能显示
				{
					gui_show_ptchar(x, y, xend, yend, offset, color, size, *str, mode); //显示一个字符
					if ((xend - x) > size / 2)
						x += size / 2; //字符,为全字的一半
					else
						x += xend - x; //未完全显示
					if (offset)
					{
						x -= offset;
						offset = 0; //清除偏移
					}
				}
				if (x >= xend)
					return; //超过了,退出
				str++;
			}
		}
		else //中文
		{
			bHz = 0;		 //有汉字库
			if (EnDisp == 0) //还未使能
			{
				if (offset >= size) //超过了一个字
				{
					offset -= size; //减少
				}
				else
				{
					offset = offset % (size); //得到字符偏移
					EnDisp = 1;				  //可以开始显示了
				}
			}
			if (EnDisp)
			{
				gui_show_ptfont(x, y, xend, yend, offset, color, size, str, mode); //显示这个汉字,空心显示
				if ((xend - x) > size)
					x += size; //字符,为全字的一半
				else
					x += xend - x; //未完全显示
				if (offset)
				{
					x -= offset;
					offset = 0; //清除偏移
				}
			}
			if (x >= xend)
				return; //超过了,退出
			str += 2;
		}
	}
}
//在指定位置开始,显示指定长度范围的字符串.(带边特效显示)
//x,y:显示开始坐标.
//width:最大显示宽度
//offset:开始显示的偏移
//color:文字颜色.
//rimcolor:文字边的颜色
//size:文字大小
//str:字符串
void gui_show_ptstrwhiterim(uint16_t x, uint16_t y, uint16_t xend, uint16_t yend, uint16_t offset, uint16_t color, uint16_t rimcolor, uint8_t size, uint8_t *str)
{
	gui_show_ptstr(x, y, xend, yend, offset, rimcolor, size, str, 2); //先画边(用大号点画)
	gui_show_ptstr(x, y, xend, yend, offset, color, size, str, 1);	  //画字(用小点画)
}

//把name加入到path后面.自动添加\和结束符.但是要确保pname的内存空间足够,否则可能导致死机.
//pname:带路径的文件名.path的内存大小至少为:strlen(path)+strlen(name)+2!
//path:路径
//name:名字
//返回值:带路径的文件名
uint8_t *gui_path_name(uint8_t *pname, uint8_t *path, uint8_t *name)
{
	const uint8_t chgchar[2] = {0X5C, 0X00};	  //转义符 等效"\"
	strcpy((char *)pname, (const char *)path);	  //拷贝path到pname里面
	strcat((char *)pname, (const char *)chgchar); //添加转义符
	strcat((char *)pname, (const char *)name);	  //添加新增的名字
	return pname;
}
/////////////////////////////////////////////////////////////////////////////////////////
//得到字符串所占的行数
//str:字符串指针;
//linelenth:每行的长度
//font:字体大小
//返回值:字符串所占的行数
uint32_t gui_get_stringline(uint8_t *str, uint16_t linelenth, uint8_t font)
{
	uint16_t xpos = 0;
	uint32_t lincnt = 1; //最少就是1行.
	if (linelenth < font / 2)
		return 0XFFFFFFFF; //无法统计完成
	while (*str != '\0' && lincnt != 0xffffffff)
	{
		if ((*str == 0x0D && (*(str + 1) == 0X0A)) || (*str == 0X0A)) //是回车/换行符
		{
			if (*str == 0X0D)
				str += 2;
			else
				str += 1;
			lincnt++; //行数加1
			xpos = 0;
		}
		else if (*str >= 0X81 && (*(str + 1) >= 0X40)) //是gbk汉字
		{
			xpos += font;
			str += 2;
			if (xpos > linelenth) //已经不在本行之内
			{
				xpos = font;
				lincnt++; //行数加1
			}
		}
		else //是字符
		{
			xpos += font / 2;
			str += 1;
			if (xpos > linelenth) //已经不在本行之内
			{
				xpos = font / 2;
				lincnt++; //行数加1
			}
		}
	}
	return lincnt;
}
//在设定区域内显示字符串,超过区域后不再显示
//str:字符串指针;
//x,y,width,height:显示区域
//font:字体大小
//fcolor:字体颜色,以叠加方式写字.
void gui_show_string(uint8_t *str, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t font, uint16_t fcolor)
{
	uint16_t xpos = x;
	uint16_t ypos = y;
	uint16_t endx = x + width - 1;
	uint16_t endy = y + height - 1;
	if (width < font / 2)
		return;			 //无法显示完成
	while (*str != '\0') //未结束
	{
		if ((*str == 0x0D && (*(str + 1) == 0X0A)) || (*str == 0X0A)) //是回车/换行符
		{
			if (*str == 0X0D)
				str += 2;
			else
				str += 1;
			xpos = x;
			ypos += font; //y增加16
		}
		else if (*str >= 0X81 && (*(str + 1) >= 0X40)) //是gbk汉字
		{
			if ((xpos + font) > (endx + 1)) //已经不在本行之内
			{
				xpos = x;
				ypos += font;
			}
			gui_show_ptfont(xpos, ypos, endx, endy, 0, fcolor, font, str, 1);
			xpos += font; //偏移
			str += 2;
		}
		else //是字符
		{
			if ((xpos + font / 2) > (endx + 1))
			{
				xpos = x;
				ypos += font;
			}
			gui_show_ptchar(xpos, ypos, endx, endy, 0, fcolor, font, *str, 1);
			xpos += font / 2;
			str += 1;
		}
		if (ypos > endy)
			break; //超过了显示区域了.
	}
}
//由此处开始向前寻找gbk码(即大于0x80的字节)的个数
//str:字符串
//pos:开始查找的地址
//返回值:gbk码个数。
uint16_t gui_string_forwardgbk_count(uint8_t *str, uint16_t pos)
{
	uint16_t t = 0;
	while (str[pos] > 0x80)
	{
		t++;
		if (pos == 0)
			break;
		pos--;
	}
	return t;
}
/////////////////////////////////////////////////////////////////////////////////////////
//内存复制函数(因为用系统memset函数会导致莫名的死机,故全部换成这个函数)
//*p:内存首地址
//c:要统一设置的值
//len:设置长度
void gui_memset(void *p, uint8_t c, uint32_t len)
{
	uint8_t *pt = (uint8_t *)p;
	while (len)
	{
		*pt = c;
		pt++;
		len--;
	}
}
//内部内存分配
//size:要分配的内存大小(uint8_t为单位)
//返回值:NULL:分配失败;其他,内存地址
void *gui_memin_malloc(uint32_t size)
{
	return (void *)malloc(size);
}
//内部内存释放
void gui_memin_free(void *ptr)
{
	if (ptr)
		free(ptr);
}
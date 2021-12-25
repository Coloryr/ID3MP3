#ifndef __GUIX_H__
#define __GUIX_H__
#include "main.h"
#include "malloc.h"
#include "../lcd/lcd.h"
#include "string.h"

//用到ucos的滴答时钟
typedef unsigned int INT32U;
extern volatile INT32U OSTime;
//gui时间计数器,需要在外部实现
//gui定时器,每10ms增加1次
#define GUI_TIMER_10MS OSTime / 2 //ucos的调度时间设置为5ms

#define GUI_VERSION 200 //gui版本(扩大了100倍),100表示V1.00

////////////////////////////////////////////////////////////////////////////////////////////////////////
//gui颜色定义
#define GUI_COLOR_BLACK 0X0000
#define GUI_COLOR_WHITE 0XFFFF

//输入事件类型
#define IN_TYPE_TOUCH 0X00
#define IN_TYPE_KEY 0X01
#define IN_TYPE_JOYPAD 0X02
#define IN_TYPE_MOUSE 0X03
#define IN_TYPE_ERR 0XFF

//gui底层接口
//也就是外部需要实现的函数!
//这里我们需要外部提供4个函数
//1,读点函数;
//2,画点函数;
//3,单色填充函数;
//4,彩色填充函数
typedef struct
{
	uint16_t back_color;												   //背景色
	uint16_t (*read_point)(uint16_t, uint16_t);							   //读点函数
	void (*draw_point)(uint16_t, uint16_t, uint16_t);					   //画点函数
	void (*fill)(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);		   //单色填充函数
	void (*colorfill)(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t *); //彩色填充函数
	uint16_t lcdwidth;													   //LCD的宽度
	uint16_t lcdheight;													   //LCD的高度
} _gui_phy;
extern _gui_phy gui_phy;

uint16_t gui_alpha_blend565(uint16_t src, uint16_t dst, uint8_t alpha); //alpha blending
uint16_t gui_color_chg(uint32_t rgb);									//颜色转换.将32位色转为16位色
uint16_t gui_rgb332torgb565(uint16_t rgb332);							//将rgb332转为rgb565
long long gui_pow(uint8_t m, uint8_t n);								//得到m^n
uint8_t *gui_path_name(uint8_t *pname, uint8_t *path, uint8_t *name);

void gui_memset(void *p, uint8_t c, uint32_t len);
void *gui_memin_malloc(uint32_t size);
void gui_memin_free(void *ptr);
void *gui_memex_malloc(uint32_t size);
void gui_memex_free(void *ptr);
void *gui_memin_realloc(void *ptr, uint32_t size);

void gui_init(void);
void gui_get_key(void *obj, uint8_t type);
uint32_t gui_disabs(uint32_t x1, uint32_t x2); //差的绝对值
void gui_alphablend_area(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color, uint8_t aval);
void gui_draw_bigpoint(uint16_t x0, uint16_t y0, uint16_t color);																					//画大点
void gui_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);																//画任意线
void gui_draw_bline(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t size, uint16_t color);												//画一条粗线
void gui_draw_bline1(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t size, uint16_t color);												//画一条粗线,方法1
void gui_draw_rectangle(uint16_t x0, uint16_t y0, uint16_t width, uint16_t height, uint16_t color);													//画矩形
void gui_draw_arcrectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t r, uint8_t mode, uint16_t upcolor, uint16_t downcolor); //画圆角矩形
void gui_draw_vline(uint16_t x0, uint16_t y0, uint16_t len, uint16_t color);																		//画垂直线
void gui_draw_hline(uint16_t x0, uint16_t y0, uint16_t len, uint16_t color);																		//画水平线
void gui_fill_colorblock(uint16_t x0, uint16_t y0, uint16_t width, uint16_t height, uint16_t *ctbl, uint8_t mode);									//填充颜色块
void gui_smooth_color(uint32_t srgb, uint32_t ergb, uint16_t *cbuf, uint16_t len);																	//颜色平滑过渡
void gui_draw_smooth_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t srgb, uint32_t ergb);								//画颜色平滑过渡的矩形/线条
void gui_fill_rectangle(uint16_t x0, uint16_t y0, uint16_t width, uint16_t height, uint16_t color);													//填充区域
void gui_fill_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
void gui_draw_ellipse(uint16_t x0, uint16_t y0, uint16_t rx, uint16_t ry, uint16_t color);
void gui_fill_ellipse(uint16_t x0, uint16_t y0, uint16_t rx, uint16_t ry, uint16_t color);
void gui_draw_argrec(uint16_t x0, uint16_t y0, uint16_t width, uint16_t height, uint16_t color);
void gui_show_strmid(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color, uint8_t size, uint8_t *str);
void gui_show_ptchar(uint16_t x, uint16_t y, uint16_t xend, uint16_t yend, uint16_t offset, uint16_t color, uint16_t size, uint8_t chr, uint8_t mode);
void gui_show_ptfont(uint16_t x, uint16_t y, uint16_t xend, uint16_t yend, uint16_t offset, uint16_t color, uint16_t size, uint8_t *chr, uint8_t mode);
void gui_show_ptstr(uint16_t x, uint16_t y, uint16_t xend, uint16_t yend, uint16_t offset, uint16_t color, uint8_t size, uint8_t *str, uint8_t mode);
void gui_show_ptstrwhiterim(uint16_t x, uint16_t y, uint16_t xend, uint16_t yend, uint16_t offset, uint16_t color, uint16_t rimcolor, uint8_t size, uint8_t *str); //特效显示字符串
void gui_draw_icos(uint16_t x, uint16_t y, uint8_t size, uint8_t index);
void gui_draw_icosalpha(uint16_t x, uint16_t y, uint8_t size, uint8_t index);
void gui_show_num(uint16_t x, uint16_t y, uint8_t len, uint16_t color, uint8_t size, long long num, uint8_t mode);
uint8_t *gui_num2str(uint8_t *str, uint32_t num); //将数字转为字符串
void gui_draw_arc(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t rx, uint16_t ry, uint16_t r, uint16_t color, uint8_t mode);

uint32_t gui_get_stringline(uint8_t *str, uint16_t linelenth, uint8_t font);
void gui_show_string(uint8_t *str, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t font, uint16_t fcolor);
uint16_t gui_string_forwardgbk_count(uint8_t *str, uint16_t pos);

#endif

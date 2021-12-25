#ifndef __LCD_H__
#define __LCD_H__
#include "main.h"
#include "stdlib.h"
#include "../bit.h"

//LCD重要参数集
typedef struct
{
	uint16_t width;	  //LCD 宽度
	uint16_t height;  //LCD 高度
	uint16_t id;	  //LCD ID
	uint8_t dir;	  //横屏还是竖屏控制：0，竖屏；1，横屏。
	uint16_t wramcmd; //开始写gram指令
	uint16_t setxcmd; //设置x坐标指令
	uint16_t setycmd; //设置y坐标指令
} _lcd_dev;

//LCD参数
extern _lcd_dev lcddev; //管理LCD重要参数
//LCD的画笔颜色和背景色
extern uint16_t POINT_COLOR; //默认红色
extern uint16_t BACK_COLOR;	 //背景颜色.默认为白色

//////////////////////////////////////////////////////////////////////////////////
//-----------------LCD端口定义----------------
#define LCD_LED PBout(0) //LCD背光    		 PB0
//LCD地址结构体
typedef struct
{
	uint16_t LCD_REG;
	uint16_t LCD_RAM;
} LCD_TypeDef;

#define LCD_BASE ((uint32_t)(0x60000000 | 0x0001FFFE))
#define LCD ((LCD_TypeDef *)LCD_BASE)
//////////////////////////////////////////////////////////////////////////////////

//扫描方向定义
#define L2R_U2D 0 //从左到右,从上到下
#define L2R_D2U 1 //从左到右,从下到上
#define R2L_U2D 2 //从右到左,从上到下
#define R2L_D2U 3 //从右到左,从下到上

#define U2D_L2R 4 //从上到下,从左到右
#define U2D_R2L 5 //从上到下,从右到左
#define D2U_L2R 6 //从下到上,从左到右
#define D2U_R2L 7 //从下到上,从右到左

#define DFT_SCAN_DIR L2R_U2D //默认的扫描方向

//画笔颜色
#define WHITE 0xFFFF
#define BLACK 0x0000
#define BLUE 0x001F
#define BRED 0XF81F
#define GRED 0XFFE0
#define GBLUE 0X07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define GREEN 0x07E0
#define CYAN 0x7FFF
#define YELLOW 0xFFE0
#define BROWN 0XBC40 //棕色
#define BRRED 0XFC07 //棕红色
#define GRAY 0X8430	 //灰色
//GUI颜色

#define DARKBLUE 0X01CF	 //深蓝色
#define LIGHTBLUE 0X7D7C //浅蓝色
#define GRAYBLUE 0X5458	 //灰蓝色
//以上三色为PANEL的颜色

#define LIGHTGREEN 0X841F //浅绿色
//#define LIGHTGRAY        0XEF5B //浅灰色(PANNEL)
#define LGRAY 0XC618 //浅灰色(PANNEL),窗体背景色

#define LGRAYBLUE 0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE 0X2B12	 //浅棕蓝色(选择条目的反色)

void LCD_Init(void);																					//初始化
void LCD_DisplayOn(void);																				//开显示
void LCD_DisplayOff(void);																				//关显示
void LCD_Clear(uint16_t Color);																			//清屏
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);														//设置光标
void LCD_DrawPoint(uint16_t x, uint16_t y);																//画点
void LCD_Fast_DrawPoint(uint16_t x, uint16_t y, uint16_t color);										//快速画点
uint16_t LCD_ReadPoint(uint16_t x, uint16_t y);															//读点
void LCD_Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r);												//画圆
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);									//画线
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);								//画矩形
void LCD_Fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t color);						//填充单色
void LCD_Color_Fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color);				//填充指定颜色
void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t num, uint8_t size, uint8_t mode);						//显示一个字符
void LCD_ShowNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size);						//显示一个数字
void LCD_ShowxNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint8_t mode);		//显示 数字
void LCD_ShowString(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, uint8_t *p); //显示一个字符串,12/16字体

void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue);
uint16_t LCD_ReadReg(uint16_t LCD_Reg);
void LCD_WriteRAM_Prepare(void);
void LCD_WriteRAM(uint16_t RGB_Code);
void LCD_SSD_BackLightSet(uint8_t pwm);											//SSD1963 背光控制
void LCD_Scan_Dir(uint8_t dir);													//设置屏扫描方向
void LCD_Display_Dir(uint8_t dir);												//设置屏幕显示方向
void LCD_Set_Window(uint16_t sx, uint16_t sy, uint16_t width, uint16_t height); //设置窗口

void LCD_WR_DATA(uint16_t data);

#endif

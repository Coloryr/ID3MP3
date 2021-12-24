#ifndef ILI9486_H
#define ILI9486_H

#include "lcd_io_gpio8.h"

/* Orientation
   - 0: 240x320 portrait 0'
   - 1: 320x240 landscape 90'
   - 2: 240x320 portrait 180'
   - 3: 320x240 landscape 270'
*/
#define  ILI9486_ORIENTATION      1

/* Color mode
   - 0: RGB565 (R:bit15..11, G:bit10..5, B:bit4..0)
   - 1: BRG565 (B:bit15..11, G:bit10..5, R:bit4..0)
*/
#define  ILI9486_COLORMODE        0

/* Analog touchscreen
   - 0: touchscreen disabled
   - 1: touchscreen enabled
*/
#define  ILI9486_TOUCH            0

/* For multi-threaded or interrupt use, Lcd and Touchscreen simultaneous use can cause confusion (since it uses common I/O resources)
   If enabled, the Lcd functions wait until the touchscreen functions are run. The touchscreen query is not executed when Lcd is busy.
   - 0: multi-threaded protection disabled
   - 1: multi-threaded protection enabled
*/
#define  ILI9486_MULTITASK_MUTEX   1

//-----------------------------------------------------------------------------
// ILI9486 physic resolution (in 0 orientation)
#define  ILI9486_LCD_PIXEL_WIDTH  320
#define  ILI9486_LCD_PIXEL_HEIGHT 480


#define ILI9486_NOP            0x00
#define ILI9486_SWRESET        0x01

#define ILI9486_RDDID          0x04
#define ILI9486_RDDST          0x09
#define ILI9486_RDMODE         0x0A
#define ILI9486_RDMADCTL       0x0B
#define ILI9486_RDPIXFMT       0x0C
#define ILI9486_RDIMGFMT       0x0D
#define ILI9486_RDSELFDIAG     0x0F

#define ILI9486_SLPIN          0x10
#define ILI9486_SLPOUT         0x11
#define ILI9486_PTLON          0x12
#define ILI9486_NORON          0x13

#define ILI9486_INVOFF         0x20
#define ILI9486_INVON          0x21
#define ILI9486_GAMMASET       0x26
#define ILI9486_DISPOFF        0x28
#define ILI9486_DISPON         0x29

#define ILI9486_CASET          0x2A
#define ILI9486_PASET          0x2B
#define ILI9486_RAMWR          0x2C
#define ILI9486_RAMRD          0x2E

#define ILI9486_PTLAR          0x30
#define ILI9486_VSCRDEF        0x33
#define ILI9486_MADCTL         0x36
#define ILI9486_VSCRSADD       0x37     /* Vertical Scrolling Start Address */
#define ILI9486_PIXFMT         0x3A     /* COLMOD: Pixel Format Set */

#define ILI9486_RGB_INTERFACE  0xB0     /* RGB Interface Signal Control */
#define ILI9486_FRMCTR1        0xB1
#define ILI9486_FRMCTR2        0xB2
#define ILI9486_FRMCTR3        0xB3
#define ILI9486_INVCTR         0xB4
#define ILI9486_DFUNCTR        0xB6     /* Display Function Control */

#define ILI9486_PWCTR1         0xC0
#define ILI9486_PWCTR2         0xC1
#define ILI9486_PWCTR3         0xC2
#define ILI9486_PWCTR4         0xC3
#define ILI9486_PWCTR5         0xC4
#define ILI9486_VMCTR1         0xC5
#define ILI9486_VMCTR2         0xC7

#define ILI9486_RDID1          0xDA
#define ILI9486_RDID2          0xDB
#define ILI9486_RDID3          0xDC
#define ILI9486_RDID4          0xDD

#define ILI9486_GMCTRP1        0xE0
#define ILI9486_GMCTRN1        0xE1
#define ILI9486_DGCTR1         0xE2
#define ILI9486_DGCTR2         0xE3

//-----------------------------------------------------------------------------
#define ILI9486_MAD_RGB        0x08
#define ILI9486_MAD_BGR        0x00

#define ILI9486_MAD_VERTICAL   0x20
#define ILI9486_MAD_X_LEFT     0x00
#define ILI9486_MAD_X_RIGHT    0x40
#define ILI9486_MAD_Y_UP       0x80
#define ILI9486_MAD_Y_DOWN     0x00

void ili9486_Init(void);

uint16_t ili9486_ReadID(void);

void ili9486_DisplayOn(void);

void ili9486_DisplayOff(void);

void ili9486_SetCursor(uint16_t Xpos, uint16_t Ypos);

void ili9486_WritePixel(uint16_t Xpos, uint16_t Ypos, uint32_t RGB_Code);

uint32_t ili9486_ReadPixel(uint16_t Xpos, uint16_t Ypos);

void ili9486_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height);

void ili9486_DrawHLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length);

void ili9486_DrawVLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length);

void ili9486_FillRect(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint32_t RGBCode);

uint16_t ili9486_GetLcdPixelWidth(void);

uint16_t ili9486_GetLcdPixelHeight(void);

void ili9486_DrawBitmap(uint16_t Xpos, uint16_t Ypos, uint8_t *pbmp);

void ili9486_DrawRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pData);

void ili9486_ReadRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pData);

void ili9486_Scroll(int16_t Scroll, uint16_t TopFix, uint16_t BottonFix);

#endif
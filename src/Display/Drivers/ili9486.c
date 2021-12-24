/*
 * ILI9486 LCD driver (optional builtin touchscreen driver)
 * 2021.05
*/

#include <string.h>
#include "main.h"
#include "lcd.h"
#include "bmp.h"
#include "ili9486.h"
#include "lcd_io_gpio8.h"

#if ILI9486_COLORMODE == 0
#define ILI9486_MAD_COLORMODE  ILI9486_MAD_RGB
#else
#define ILI9486_MAD_COLORMODE  ILI9486_MAD_BGR
#endif

#if (ILI9486_ORIENTATION == 0)
#define ILI9486_SIZE_X                     ILI9486_LCD_PIXEL_WIDTH
#define ILI9486_SIZE_Y                     ILI9486_LCD_PIXEL_HEIGHT
#define ILI9486_MAD_DATA_RIGHT_THEN_UP     ILI9486_MAD_COLORMODE | ILI9486_MAD_X_RIGHT | ILI9486_MAD_Y_UP
#define ILI9486_MAD_DATA_RIGHT_THEN_DOWN   ILI9486_MAD_COLORMODE | ILI9486_MAD_X_RIGHT | ILI9486_MAD_Y_DOWN
#define ILI9486_MAD_DATA_RGBMODE           ILI9486_MAD_COLORMODE | ILI9486_MAD_X_LEFT  | ILI9486_MAD_Y_DOWN
#elif (ILI9486_ORIENTATION == 1)
#define ILI9486_SIZE_X                     ILI9486_LCD_PIXEL_HEIGHT
#define ILI9486_SIZE_Y                     ILI9486_LCD_PIXEL_WIDTH
#define ILI9486_MAD_DATA_RIGHT_THEN_UP     ILI9486_MAD_COLORMODE | ILI9486_MAD_X_RIGHT | ILI9486_MAD_Y_DOWN | ILI9486_MAD_VERTICAL
#define ILI9486_MAD_DATA_RIGHT_THEN_DOWN   ILI9486_MAD_COLORMODE | ILI9486_MAD_X_LEFT  | ILI9486_MAD_Y_DOWN | ILI9486_MAD_VERTICAL
#define ILI9486_MAD_DATA_RGBMODE           ILI9486_MAD_COLORMODE | ILI9486_MAD_X_RIGHT | ILI9486_MAD_Y_DOWN
#elif (ILI9486_ORIENTATION == 2)
#define ILI9486_SIZE_X                     ILI9486_LCD_PIXEL_WIDTH
#define ILI9486_SIZE_Y                     ILI9486_LCD_PIXEL_HEIGHT
#define ILI9486_MAD_DATA_RIGHT_THEN_UP     ILI9486_MAD_COLORMODE | ILI9486_MAD_X_LEFT  | ILI9486_MAD_Y_DOWN
#define ILI9486_MAD_DATA_RIGHT_THEN_DOWN   ILI9486_MAD_COLORMODE | ILI9486_MAD_X_LEFT  | ILI9486_MAD_Y_UP
#define ILI9486_MAD_DATA_RGBMODE           ILI9486_MAD_COLORMODE | ILI9486_MAD_X_RIGHT | ILI9486_MAD_Y_UP
#elif (ILI9486_ORIENTATION == 3)
#define ILI9486_SIZE_X                     ILI9486_LCD_PIXEL_HEIGHT
#define ILI9486_SIZE_Y                     ILI9486_LCD_PIXEL_WIDTH
#define ILI9486_MAD_DATA_RIGHT_THEN_UP     ILI9486_MAD_COLORMODE | ILI9486_MAD_X_LEFT  | ILI9486_MAD_Y_UP   | ILI9486_MAD_VERTICAL
#define ILI9486_MAD_DATA_RIGHT_THEN_DOWN   ILI9486_MAD_COLORMODE | ILI9486_MAD_X_RIGHT | ILI9486_MAD_Y_UP   | ILI9486_MAD_VERTICAL
#define ILI9486_MAD_DATA_RGBMODE           ILI9486_MAD_COLORMODE | ILI9486_MAD_X_LEFT  | ILI9486_MAD_Y_UP
#endif

#define ILI9486_SETCURSOR(x, y)            {LCD_IO_WriteCmd8(ILI9486_CASET); LCD_IO_WriteData16_to_2x8(x); LCD_IO_WriteData16_to_2x8(x); \
                                            LCD_IO_WriteCmd8(ILI9486_PASET); LCD_IO_WriteData16_to_2x8(y); LCD_IO_WriteData16_to_2x8(y); }

//-----------------------------------------------------------------------------
#define ILI9486_LCD_INITIALIZED    0x01
#define ILI9486_IO_INITIALIZED     0x02
static uint8_t Is_ili9486_Initialized = 0;

#if      ILI9486_MULTITASK_MUTEX == 1 && ILI9486_TOUCH == 1
volatile uint8_t io_lcd_busy = 0;
volatile uint8_t io_ts_busy = 0;
#define  ILI9486_LCDMUTEX_PUSH()    while(io_ts_busy); io_lcd_busy++;
#define  ILI9486_LCDMUTEX_POP()     io_lcd_busy--
#else
#define  ILI9486_LCDMUTEX_PUSH()
#define  ILI9486_LCDMUTEX_POP()
#endif

#define  LCD_IO_WriteData16_to_2x8(dt)    {LCD_IO_WriteData8((dt) >> 8); LCD_IO_WriteData8(dt); }

//-----------------------------------------------------------------------------

static uint16_t yStart, yEnd;

//-----------------------------------------------------------------------------
/**
  * @brief  Enables the Display.
  * @param  None
  * @retval None
  */
void ili9486_DisplayOn(void) {
    ILI9486_LCDMUTEX_PUSH();
    LCD_IO_WriteCmd8(ILI9486_SLPOUT);    // Exit Sleep
    ILI9486_LCDMUTEX_POP();
    LCD_IO_Bl_OnOff(1);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Disables the Display.
  * @param  None
  * @retval None
  */
void ili9486_DisplayOff(void) {
    LCD_IO_Bl_OnOff(0);
    ILI9486_LCDMUTEX_PUSH();
    LCD_IO_WriteCmd8(ILI9486_SLPIN);    // Sleep
    ILI9486_LCDMUTEX_POP();
}

//-----------------------------------------------------------------------------
/**
  * @brief  Get the LCD pixel Width.
  * @param  None
  * @retval The Lcd Pixel Width
  */
uint16_t ili9486_GetLcdPixelWidth(void) {
    return ILI9486_SIZE_X;
}

//-----------------------------------------------------------------------------
/**
  * @brief  Get the LCD pixel Height.
  * @param  None
  * @retval The Lcd Pixel Height
  */
uint16_t ili9486_GetLcdPixelHeight(void) {
    return ILI9486_SIZE_Y;
}

//-----------------------------------------------------------------------------
/**
  * @brief  Get the ILI9486 ID.
  * @param  None
  * @retval The ILI9486 ID
  */
uint16_t ili9486_ReadID(void) {
    uint32_t id = 0;
    ILI9486_LCDMUTEX_PUSH();
    LCD_IO_ReadCmd8MultipleData8(0xD3, (uint8_t *) &id, 3, 1);
    ILI9486_LCDMUTEX_POP();
    if (id == 0x869400)
        return 0x9486;
    else
        return 0;
}

//-----------------------------------------------------------------------------
/**
  * @brief  ILI9486 initialization
  * @param  None
  * @retval None
  */
void ili9486_Init(void) {
    if ((Is_ili9486_Initialized & ILI9486_LCD_INITIALIZED) == 0) {
        Is_ili9486_Initialized |= ILI9486_LCD_INITIALIZED;
        if ((Is_ili9486_Initialized & ILI9486_IO_INITIALIZED) == 0)
            LCD_IO_Init();
        Is_ili9486_Initialized |= ILI9486_IO_INITIALIZED;
    }

    LCD_Delay(10);
    LCD_IO_WriteCmd8(ILI9486_SWRESET);
    LCD_Delay(100);

    LCD_IO_WriteCmd8MultipleData8(ILI9486_RGB_INTERFACE, (uint8_t *) "\x00", 1); // RGB mode off (0xB0)
    LCD_IO_WriteCmd8(ILI9486_SLPOUT);    // Exit Sleep (0x11)
    LCD_Delay(10);

    LCD_IO_WriteCmd8MultipleData8(ILI9486_PIXFMT, (uint8_t *) "\x55", 1); // interface format (0x3A)

    LCD_IO_WriteCmd8(ILI9486_MADCTL);
    LCD_IO_WriteData8(ILI9486_MAD_DATA_RIGHT_THEN_DOWN);

    LCD_IO_WriteCmd8MultipleData8(ILI9486_PWCTR3, (uint8_t *) "\x44", 1); // 0xC2
    LCD_IO_WriteCmd8MultipleData8(ILI9486_VMCTR1, (uint8_t *) "\x00\x00\x00\x00", 4); // 0xC5

    // positive gamma control (0xE0)
    LCD_IO_WriteCmd8MultipleData8(ILI9486_GMCTRP1,
                                  (uint8_t *) "\x0F\x1F\x1C\x0C\x0F\x08\x48\x98\x37\x0A\x13\x04\x11\x0D\x00", 15);

    // negative gamma control (0xE1)
    LCD_IO_WriteCmd8MultipleData8(ILI9486_GMCTRN1,
                                  (uint8_t *) "\x0F\x32\x2E\x0B\x0D\x05\x47\x75\x37\x06\x10\x03\x24\x20\x00", 15);

    // Digital gamma control1 (0xE2)
    LCD_IO_WriteCmd8MultipleData8(ILI9486_DGCTR1,
                                  (uint8_t *) "\x0F\x32\x2E\x0B\x0D\x05\x47\x75\x37\x06\x10\x03\x24\x20\x00", 15);

    LCD_IO_WriteCmd8(ILI9486_NORON);     // Normal display on (0x13)
    LCD_IO_WriteCmd8(ILI9486_INVOFF);    // Display inversion off (0x20)
    LCD_IO_WriteCmd8(ILI9486_SLPOUT);    // Exit Sleep (0x11)
    LCD_Delay(200);
    LCD_IO_WriteCmd8(ILI9486_DISPON);    // Display on (0x29)
    LCD_Delay(10);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Set Cursor position.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @retval None
  */
void ili9486_SetCursor(uint16_t Xpos, uint16_t Ypos) {
    ILI9486_LCDMUTEX_PUSH();
    ILI9486_SETCURSOR(Xpos, Ypos);
    ILI9486_LCDMUTEX_POP();
}

//-----------------------------------------------------------------------------
/**
  * @brief  Write pixel.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @param  RGBCode: the RGB pixel color
  * @retval None
  */
void ili9486_WritePixel(uint16_t Xpos, uint16_t Ypos, uint32_t RGBCode) {
    ILI9486_LCDMUTEX_PUSH();
    ILI9486_SETCURSOR(Xpos, Ypos);
    LCD_IO_WriteCmd8(ILI9486_RAMWR);
    LCD_IO_WriteData16(RGBCode);
    ILI9486_LCDMUTEX_POP();
}


//-----------------------------------------------------------------------------
/**
  * @brief  Read pixel.
  * @param  None
  * @retval the RGB pixel color
  */
uint32_t ili9486_ReadPixel(uint16_t Xpos, uint16_t Ypos) {
    uint16_t ret;
    ILI9486_LCDMUTEX_PUSH();
    LCD_IO_WriteCmd8MultipleData8(ILI9486_PIXFMT, (uint8_t *) "\x66", 1); // Read: only 24bit pixel mode
    ILI9486_SETCURSOR(Xpos, Ypos);
    LCD_IO_ReadCmd8MultipleData24to16(ILI9486_RAMRD, (uint16_t *) &ret, 1, 1);
    LCD_IO_WriteCmd8MultipleData8(ILI9486_PIXFMT, (uint8_t *) "\x55", 1); // Return to 16bit pixel mode
    ILI9486_LCDMUTEX_POP();
    return (ret);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Sets a display window
  * @param  Xpos:   specifies the X bottom left position.
  * @param  Ypos:   specifies the Y bottom left position.
  * @param  Height: display window height.
  * @param  Width:  display window width.
  * @retval None
  */
void ili9486_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height) {
    yStart = Ypos;
    yEnd = Ypos + Height - 1;
    ILI9486_LCDMUTEX_PUSH();
    LCD_IO_WriteCmd8(ILI9486_CASET);
    LCD_IO_WriteData16_to_2x8(Xpos);
    LCD_IO_WriteData16_to_2x8(Xpos + Width - 1);
    LCD_IO_WriteCmd8(ILI9486_PASET);
    LCD_IO_WriteData16_to_2x8(Ypos);
    LCD_IO_WriteData16_to_2x8(Ypos + Height - 1);
    ILI9486_LCDMUTEX_POP();
}

//-----------------------------------------------------------------------------
/**
  * @brief  Draw vertical line.
  * @param  RGBCode: Specifies the RGB color
  * @param  Xpos:     specifies the X position.
  * @param  Ypos:     specifies the Y position.
  * @param  Length:   specifies the Line length.
  * @retval None
  */
void ili9486_DrawHLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length) {
    ILI9486_LCDMUTEX_PUSH();
    LCD_IO_WriteCmd8(ILI9486_CASET);
    LCD_IO_WriteData16_to_2x8(Xpos);
    LCD_IO_WriteData16_to_2x8(Xpos + Length - 1);
    LCD_IO_WriteCmd8(ILI9486_PASET);
    LCD_IO_WriteData16_to_2x8(Ypos);
    LCD_IO_WriteData16_to_2x8(Ypos);
    LCD_IO_WriteCmd8DataFill16(ILI9486_RAMWR, RGBCode, Length);
    ILI9486_LCDMUTEX_POP();
}

//-----------------------------------------------------------------------------
/**
  * @brief  Draw vertical line.
  * @param  RGBCode: Specifies the RGB color
  * @param  Xpos:     specifies the X position.
  * @param  Ypos:     specifies the Y position.
  * @param  Length:   specifies the Line length.
  * @retval None
  */
void ili9486_DrawVLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length) {
    ILI9486_LCDMUTEX_PUSH();
    LCD_IO_WriteCmd8(ILI9486_CASET);
    LCD_IO_WriteData16_to_2x8(Xpos);
    LCD_IO_WriteData16_to_2x8(Xpos);
    LCD_IO_WriteCmd8(ILI9486_PASET);
    LCD_IO_WriteData16_to_2x8(Ypos);
    LCD_IO_WriteData16_to_2x8(Ypos + Length - 1);
    LCD_IO_WriteCmd8DataFill16(ILI9486_RAMWR, RGBCode, Length);
    ILI9486_LCDMUTEX_POP();
}

//-----------------------------------------------------------------------------
/**
  * @brief  Draw Filled rectangle
  * @param  Xpos:     specifies the X position.
  * @param  Ypos:     specifies the Y position.
  * @param  Xsize:    specifies the X size
  * @param  Ysize:    specifies the Y size
  * @param  RGBCode:  specifies the RGB color
  * @retval None
  */
void ili9486_FillRect(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint32_t RGBCode) {
    ILI9486_LCDMUTEX_PUSH();
    LCD_IO_WriteCmd8(ILI9486_CASET);
    LCD_IO_WriteData16_to_2x8(Xpos);
    LCD_IO_WriteData16_to_2x8(Xpos + Xsize - 1);
    LCD_IO_WriteCmd8(ILI9486_PASET);
    LCD_IO_WriteData16_to_2x8(Ypos);
    LCD_IO_WriteData16_to_2x8(Ypos + Ysize - 1);
    LCD_IO_WriteCmd8DataFill16(ILI9486_RAMWR, RGBCode, Xsize * Ysize);
    ILI9486_LCDMUTEX_POP();
}

//-----------------------------------------------------------------------------
/**
  * @brief  Displays a 16bit bitmap picture..
  * @param  BmpAddress: Bmp picture address.
  * @param  Xpos:  Bmp X position in the LCD
  * @param  Ypos:  Bmp Y position in the LCD
  * @retval None
  * @brief  Draw direction: right then up
  */
void ili9486_DrawBitmap(uint16_t Xpos, uint16_t Ypos, uint8_t *pbmp) {
    uint32_t index, size;
    /* Read bitmap size */
    size = ((BITMAPSTRUCT *) pbmp)->fileHeader.bfSize;
    /* Get bitmap data address offset */
    index = ((BITMAPSTRUCT *) pbmp)->fileHeader.bfOffBits;
    size = (size - index) / 2;
    pbmp += index;

    ILI9486_LCDMUTEX_PUSH();
    LCD_IO_WriteCmd8(ILI9486_MADCTL);
    LCD_IO_WriteData8(ILI9486_MAD_DATA_RIGHT_THEN_UP);
    LCD_IO_WriteCmd8(ILI9486_PASET);
    LCD_IO_WriteData16_to_2x8(ILI9486_SIZE_Y - 1 - yEnd);
    LCD_IO_WriteData16_to_2x8(ILI9486_SIZE_Y - 1 - yStart);
    LCD_IO_WriteCmd8MultipleData16(ILI9486_RAMWR, (uint16_t *) pbmp, size);
    LCD_IO_WriteCmd8(ILI9486_MADCTL);
    LCD_IO_WriteData8(ILI9486_MAD_DATA_RIGHT_THEN_DOWN);
    ILI9486_LCDMUTEX_POP();
}

//-----------------------------------------------------------------------------
/**
  * @brief  Displays 16bit/pixel picture..
  * @param  pdata: picture address.
  * @param  Xpos:  Image X position in the LCD
  * @param  Ypos:  Image Y position in the LCD
  * @param  Xsize: Image X size in the LCD
  * @param  Ysize: Image Y size in the LCD
  * @retval None
  * @brief  Draw direction: right then down
  */
void ili9486_DrawRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pData) {
    ili9486_SetDisplayWindow(Xpos, Ypos, Xsize, Ysize);
    ILI9486_LCDMUTEX_PUSH();
    LCD_IO_WriteCmd8MultipleData16(ILI9486_RAMWR, pData, Xsize * Ysize);
    ILI9486_LCDMUTEX_POP();
}

//-----------------------------------------------------------------------------
/**
  * @brief  Read 16bit/pixel picture from Lcd and store to RAM
  * @param  pdata: picture address.
  * @param  Xpos:  Image X position in the LCD
  * @param  Ypos:  Image Y position in the LCD
  * @param  Xsize: Image X size in the LCD
  * @param  Ysize: Image Y size in the LCD
  * @retval None
  * @brief  Draw direction: right then down
  */
void ili9486_ReadRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pData) {
    ili9486_SetDisplayWindow(Xpos, Ypos, Xsize, Ysize);
    ILI9486_LCDMUTEX_PUSH();
    LCD_IO_WriteCmd8MultipleData8(ILI9486_PIXFMT, (uint8_t *) "\x66", 1); // Read: only 24bit pixel mode
    LCD_IO_ReadCmd8MultipleData24to16(ILI9486_RAMRD, pData, Xsize * Ysize, 1);
    LCD_IO_WriteCmd8MultipleData8(ILI9486_PIXFMT, (uint8_t *) "\x55", 1); // Return to 16bit pixel mode
    ILI9486_LCDMUTEX_POP();
}

//-----------------------------------------------------------------------------
/**
  * @brief  Set display scroll parameters
  * @param  Scroll    : Scroll size [pixel]
  * @param  TopFix    : Top fix size [pixel]
  * @param  BottonFix : Botton fix size [pixel]
  * @retval None
  */
void ili9486_Scroll(int16_t Scroll, uint16_t TopFix, uint16_t BottonFix) {
    static uint16_t scrparam[4] = {0, 0, 0, 0};
    ILI9486_LCDMUTEX_PUSH();
#if (ILI9486_ORIENTATION == 0)
    if ((TopFix != scrparam[1]) || (BottonFix != scrparam[3])) {
        scrparam[1] = TopFix;
        scrparam[3] = BottonFix;
        scrparam[2] = ILI9486_LCD_PIXEL_HEIGHT - TopFix - BottonFix;
        LCD_IO_WriteCmd8MultipleData16(ILI9486_VSCRDEF, &scrparam[1], 3);
    }
    Scroll = (0 - Scroll) % scrparam[2];
    if (Scroll < 0)
        Scroll = scrparam[2] + Scroll + scrparam[1];
    else
        Scroll = Scroll + scrparam[1];
#elif (ILI9486_ORIENTATION == 1)
    if((TopFix != scrparam[1]) || (BottonFix != scrparam[3]))
    {
      scrparam[1] = TopFix;
      scrparam[3] = BottonFix;
      scrparam[2] = ILI9486_LCD_PIXEL_HEIGHT - TopFix - BottonFix;
      LCD_IO_WriteCmd8MultipleData16(ILI9486_VSCRDEF, &scrparam[1], 3);
    }
    Scroll = (0 - Scroll) % scrparam[2];
    if(Scroll < 0)
      Scroll = scrparam[2] + Scroll + scrparam[1];
    else
      Scroll = Scroll + scrparam[1];
#elif (ILI9486_ORIENTATION == 2)
    if((TopFix != scrparam[3]) || (BottonFix != scrparam[1]))
    {
      scrparam[3] = TopFix;
      scrparam[1] = BottonFix;
      scrparam[2] = ILI9486_LCD_PIXEL_HEIGHT - TopFix - BottonFix;
      LCD_IO_WriteCmd8MultipleData16(ILI9486_VSCRDEF, &scrparam[1], 3);
    }
    Scroll %= scrparam[2];
    if(Scroll < 0)
      Scroll = scrparam[2] + Scroll + scrparam[1];
    else
      Scroll = Scroll + scrparam[1];
#elif (ILI9486_ORIENTATION == 3)
    if((TopFix != scrparam[3]) || (BottonFix != scrparam[1]))
    {
      scrparam[3] = TopFix;
      scrparam[1] = BottonFix;
      scrparam[2] = ILI9486_LCD_PIXEL_HEIGHT - TopFix - BottonFix;
      LCD_IO_WriteCmd8MultipleData16(ILI9486_VSCRDEF, &scrparam[1], 3);
    }
    Scroll %= scrparam[2];
    if(Scroll < 0)
      Scroll = scrparam[2] + Scroll + scrparam[1];
    else
      Scroll = Scroll + scrparam[1];
#endif
    if (Scroll != scrparam[0]) {
        scrparam[0] = Scroll;
        LCD_IO_WriteCmd8DataFill16(ILI9486_VSCRSADD, scrparam[0], 1);
    }
    ILI9486_LCDMUTEX_POP();
}
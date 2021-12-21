#include "lcd.h"
#include "stm32_adafruit_lcd.h"

/* @defgroup STM32_ADAFRUIT_LCD_Private_Defines */
#define POLY_X(Z)             ((int32_t)((Points + (Z))->X))
#define POLY_Y(Z)             ((int32_t)((Points + (Z))->Y))
#define NULL                  (void *)0

/* @defgroup STM32_ADAFRUIT_LCD_Private_Macros */
#define ABS(X) ((X) > 0 ? (X) : -(X))
#define SWAP16(a, b) {uint16_t t = a; a = b; b = t;}

/**
  * @brief  Initializes the LCD.
  * @param  None
  * @retval LCD state
  */
uint8_t BSP_LCD_Init(void) {
    uint8_t ret = LCD_ERROR;

    /* LCD Init */
    lcd_drv->Init();
    ret = LCD_OK;
    return ret;
}

/**
  * @brief  Draws a pixel on LCD.
  * @param  Xpos: X position 
  * @param  Ypos: Y position
  * @param  RGB_Code: Pixel color in RGB mode (5-6-5)  
  * @retval None
  */
void BSP_LCD_DrawPixel(uint16_t Xpos, uint16_t Ypos, uint16_t RGB_Code) {
    if (lcd_drv->WritePixel != NULL) {
        lcd_drv->WritePixel(Xpos, Ypos, RGB_Code);
    }
}
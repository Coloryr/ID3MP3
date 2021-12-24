/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32_ADAFRUIT_LCD_H
#define __STM32_ADAFRUIT_LCD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "lcd.h"

#include "ili9486.h"

/** 
  * @brief  Point structures definition
  */
typedef struct {
    int16_t X;
    int16_t Y;
} Point, *pPoint;

/** 
  * @brief  LCD status structure definition  
  */
#define LCD_OK         0x00
#define LCD_ERROR      0x01
#define LCD_TIMEOUT    0x02

#if LCD_REVERSE == 1
#define  RC(a)   ((((a) & 0xFF) << 8) | (((a) & 0xFF00) >> 8))
#else
#define  RC(a)   a
#endif

/** 
  * @brief  LCD color  
  */
#define LCD_COLOR(r, g, b)      RC((r & 0xF8) << 8 | (g & 0xFC) << 3 | (b & 0xF8) >> 3)

/** @defgroup STM32_ADAFRUIT_LCD_Exported_Functions
  * @{
  */
uint8_t BSP_LCD_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32_ADAFRUIT_LCD_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

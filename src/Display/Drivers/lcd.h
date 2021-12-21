#ifndef __LCD_H
#define __LCD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* The 16 bits values (color codes, bitmap) byte order
 * - 0: ne reverse order
 * - 1: reverse order
 *   note: Using reverse order is only recommended in 8 bit fsmc dma mode so that dma can be turned on. 
           In all other cases it is disadvantageous.
 */
#define  LCD_REVERSE16   0

/** @defgroup LCD_Driver_structure  LCD Driver structure
  * @{
  */
typedef struct {
    void (*Init)(void);

    uint16_t (*ReadID)(void);

    void (*DisplayOn)(void);

    void (*DisplayOff)(void);

    void (*SetCursor)(uint16_t, uint16_t);

    void (*WritePixel)(uint16_t, uint16_t, uint32_t);

    uint32_t (*readPixel)(uint16_t, uint16_t);

    void (*SetDisplayWindow)(uint16_t, uint16_t, uint16_t, uint16_t);

    void (*DrawHLine)(uint16_t, uint16_t, uint16_t, uint16_t);

    void (*DrawVLine)(uint16_t, uint16_t, uint16_t, uint16_t);

    uint16_t (*getLcdPixelWidth)(void);

    uint16_t (*getLcdPixelHeight)(void);

    void (*DrawBitmap)(uint16_t, uint16_t, uint8_t *);

    void (*DrawRGBImage)(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t *);

    void (*FillRect)(uint16_t, uint16_t, uint16_t, uint16_t, uint32_t);

    void (*ReadRGBImage)(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t *);

    void (*Scroll)(int16_t, uint16_t, uint16_t);
} LCD_DrvTypeDef;

#ifdef __cplusplus
}
#endif

#endif /* __LCD_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

#ifndef __TS_H
#define __TS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/** @defgroup TS_Driver_structure  Touch Sensor Driver structure
  * @{
  */
typedef struct {
    void (*Init)();
    uint8_t (*DetectTouch)();
    void (*GetXY)(uint16_t, uint16_t *, uint16_t *);
} TS_DrvTypeDef;


#ifdef __cplusplus
}
#endif

#endif /* __TS_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

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
    void (*Init)(uint16_t);

    uint16_t (*ReadID)(uint16_t);

    void (*Reset)(uint16_t);

    void (*Start)(uint16_t);

    uint8_t (*DetectTouch)(uint16_t);

    void (*GetXY)(uint16_t, uint16_t *, uint16_t *);

    void (*EnableIT)(uint16_t);

    void (*ClearIT)(uint16_t);

    uint8_t (*GetITStatus)(uint16_t);

    void (*DisableIT)(uint16_t);
} TS_DrvTypeDef;


#ifdef __cplusplus
}
#endif

#endif /* __TS_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

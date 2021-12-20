/* 
 * 8 bit paralell LCD/TOUCH FSMC driver for STM32F4
 * 5 controll pins (CS, RS, WR, RD, RST) + 8 data pins + 1 backlight pin

 * note: whereas the LCD and Touchscreen are on the same pins,
 * therefore, the simultaneous use of Lcd and Touchscreen must be excluded!
 * In a multithreaded / intermittent environment, care must be taken to prevent collisions!

 * caution: if there are other devices on the fsmc bus (eg memory), reading the analog touch may cause a collision!
 */

/* 
 * Author: Roberto Benjami
 * version:  2020.01
 */


/* ADC sample time (0:3cycles, 1:15c, 2:28c, 3:55c, 4:84c, 5:112c, 6:144c, 7:480cycles) */
#define TS_SAMPLETIME         2

#include "main.h"
#include "lcd.h"
#include "lcdts_io_fsmc8.h"

/* Change the byte order of 16bits data? */
#if LCD_REVERSE16 == 0
#define RD(a)                 __REVSH(a)
#endif
#if LCD_REVERSE16 == 1
#define RD(a)                 a
#endif

#define LCD_ADDR_DATA         (LCD_ADDR_BASE + (1 << (LCD_REGSELECT_BIT + 2)) - 2)

#define DMA_MAXSIZE           0xFFFE

/* Link function for LCD peripheral */
void     LCD_Delay (uint32_t delay);
void     LCD_IO_Init(void);
void     LCD_IO_Bl_OnOff(uint8_t Bl);

void     LCD_IO_WriteCmd8(uint8_t Cmd);
void     LCD_IO_WriteCmd16(uint16_t Cmd);
void     LCD_IO_WriteData8(uint8_t Data);
void     LCD_IO_WriteData16(uint16_t Data);

void     LCD_IO_WriteCmd8DataFill16(uint8_t Cmd, uint16_t Data, uint32_t Size);
void     LCD_IO_WriteCmd8MultipleData8(uint8_t Cmd, uint8_t *pData, uint32_t Size);
void     LCD_IO_WriteCmd8MultipleData16(uint8_t Cmd, uint16_t *pData, uint32_t Size);
void     LCD_IO_WriteCmd16DataFill16(uint16_t Cmd, uint16_t Data, uint32_t Size);
void     LCD_IO_WriteCmd16MultipleData8(uint16_t Cmd, uint8_t *pData, uint32_t Size);
void     LCD_IO_WriteCmd16MultipleData16(uint16_t Cmd, uint16_t *pData, uint32_t Size);

void     LCD_IO_ReadCmd8MultipleData8(uint8_t Cmd, uint8_t *pData, uint32_t Size, uint32_t DummySize);
void     LCD_IO_ReadCmd8MultipleData16(uint8_t Cmd, uint16_t *pData, uint32_t Size, uint32_t DummySize);
void     LCD_IO_ReadCmd8MultipleData24to16(uint8_t Cmd, uint16_t *pData, uint32_t Size, uint32_t DummySize);
void     LCD_IO_ReadCmd16MultipleData8(uint16_t Cmd, uint8_t *pData, uint32_t Size, uint32_t DummySize);
void     LCD_IO_ReadCmd16MultipleData16(uint16_t Cmd, uint16_t *pData, uint32_t Size, uint32_t DummySize);
void     LCD_IO_ReadCmd16MultipleData24to16(uint16_t Cmd, uint16_t *pData, uint32_t Size, uint32_t DummySize);

/* Link function for Touchscreen */
uint8_t  TS_IO_DetectToch(void);
uint16_t TS_IO_GetX(void);
uint16_t TS_IO_GetY(void);
uint16_t TS_IO_GetZ1(void);
uint16_t TS_IO_GetZ2(void);

//-----------------------------------------------------------------------------
#define BITBAND_ACCESS(a, b)  *(volatile uint32_t*)(((uint32_t)&a & 0xF0000000) + 0x2000000 + (((uint32_t)&a & 0x000FFFFF) << 5) + (b << 2))

/* GPIO mode */
#define MODE_DIGITAL_INPUT    0x0
#define MODE_OUT              0x1
#define MODE_ALTER            0x2
#define MODE_ANALOG_INPUT     0x3

#define MODE_SPD_LOW          0x0
#define MODE_SPD_MEDIUM       0x1
#define MODE_SPD_HIGH         0x2
#define MODE_SPD_VHIGH        0x3

#define MODE_PU_NONE          0x0
#define MODE_PU_UP            0x1
#define MODE_PU_DOWN          0x2

#define GPIOX_PORT_(a, b)     GPIO ## a
#define GPIOX_PORT(a)         GPIOX_PORT_(a)

#define GPIOX_PIN_(a, b)      b
#define GPIOX_PIN(a)          GPIOX_PIN_(a)

#define GPIOX_AFR_(a,b,c)     GPIO ## b->AFR[c >> 3] = (GPIO ## b->AFR[c >> 3] & ~(0x0F << (4 * (c & 7)))) | (a << (4 * (c & 7)));
#define GPIOX_AFR(a, b)       GPIOX_AFR_(a, b)

#define GPIOX_MODER_(a,b,c)   GPIO ## b->MODER = (GPIO ## b->MODER & ~(3 << (2 * c))) | (a << (2 * c));
#define GPIOX_MODER(a, b)     GPIOX_MODER_(a, b)

#define GPIOX_OSPEEDR_(a,b,c) GPIO ## b->OSPEEDR = (GPIO ## b->OSPEEDR & ~(3 << (2 * c))) | (a << (2 * c));
#define GPIOX_OSPEEDR(a, b)   GPIOX_OSPEEDR_(a, b)

#define GPIOX_PUPDR_(a,b,c)   GPIO ## b->PUPDR = (GPIO ## b->PUPDR & ~(3 << (2 * c))) | (a << (2 * c));
#define GPIOX_PUPDR(a, b)     GPIOX_PUPDR_(a, b)

#define GPIOX_ODR_(a, b)      BITBAND_ACCESS(GPIO ## a ->ODR, b)
#define GPIOX_ODR(a)          GPIOX_ODR_(a)

#define GPIOX_IDR_(a, b)      BITBAND_ACCESS(GPIO ## a ->IDR, b)
#define GPIOX_IDR(a)          GPIOX_IDR_(a)

#define GPIOX_LINE_(a, b)     EXTI_Line ## b
#define GPIOX_LINE(a)         GPIOX_LINE_(a)

#define GPIOX_CLOCK_(a, b)    RCC_AHB1ENR_GPIO ## a ## EN
#define GPIOX_CLOCK(a)        GPIOX_CLOCK_(a)

#define GPIOX_PORTNUM_A       1
#define GPIOX_PORTNUM_B       2
#define GPIOX_PORTNUM_C       3
#define GPIOX_PORTNUM_D       4
#define GPIOX_PORTNUM_E       5
#define GPIOX_PORTNUM_F       6
#define GPIOX_PORTNUM_G       7
#define GPIOX_PORTNUM_H       8
#define GPIOX_PORTNUM_I       9
#define GPIOX_PORTNUM_J       10
#define GPIOX_PORTNUM_K       11
#define GPIOX_PORTNUM_(a, b)  GPIOX_PORTNUM_ ## a
#define GPIOX_PORTNUM(a)      GPIOX_PORTNUM_(a)

#define GPIOX_PORTNAME_(a, b) a
#define GPIOX_PORTNAME(a)     GPIOX_PORTNAME_(a)

//-----------------------------------------------------------------------------
/* DMA mode */
#define DMA_ISR_TCIF0_Pos       (5U)
#define DMA_ISR_TCIF0           (0x1U << DMA_ISR_TCIF0_Pos)                  /*!< 0x00000020 */
#define DMA_ISR_HTIF0_Pos       (4U)
#define DMA_ISR_HTIF0           (0x1U << DMA_ISR_HTIF0_Pos)                  /*!< 0x00000010 */
#define DMA_ISR_TEIF0_Pos       (3U)
#define DMA_ISR_TEIF0           (0x1U << DMA_ISR_TEIF0_Pos)                  /*!< 0x00000008 */
#define DMA_ISR_DMEIF0_Pos      (2U)
#define DMA_ISR_DMEIF0          (0x1U << DMA_ISR_DMEIF0_Pos)                 /*!< 0x00000004 */
#define DMA_ISR_FEIF0_Pos       (0U)
#define DMA_ISR_FEIF0           (0x1U << DMA_ISR_FEIF0_Pos)                  /*!< 0x00000001 */
#define DMA_ISR_TCIF1_Pos       (11U)
#define DMA_ISR_TCIF1           (0x1U << DMA_ISR_TCIF1_Pos)                  /*!< 0x00000800 */
#define DMA_ISR_HTIF1_Pos       (10U)
#define DMA_ISR_HTIF1           (0x1U << DMA_ISR_HTIF1_Pos)                  /*!< 0x00000400 */
#define DMA_ISR_TEIF1_Pos       (9U)
#define DMA_ISR_TEIF1           (0x1U << DMA_ISR_TEIF1_Pos)                  /*!< 0x00000200 */
#define DMA_ISR_DMEIF1_Pos      (8U)
#define DMA_ISR_DMEIF1          (0x1U << DMA_ISR_DMEIF1_Pos)                 /*!< 0x00000100 */
#define DMA_ISR_FEIF1_Pos       (6U)
#define DMA_ISR_FEIF1           (0x1U << DMA_ISR_FEIF1_Pos)                  /*!< 0x00000040 */
#define DMA_ISR_TCIF2_Pos       (21U)
#define DMA_ISR_TCIF2           (0x1U << DMA_ISR_TCIF2_Pos)                  /*!< 0x00200000 */
#define DMA_ISR_HTIF2_Pos       (20U)
#define DMA_ISR_HTIF2           (0x1U << DMA_ISR_HTIF2_Pos)                  /*!< 0x00100000 */
#define DMA_ISR_TEIF2_Pos       (19U)
#define DMA_ISR_TEIF2           (0x1U << DMA_ISR_TEIF2_Pos)                  /*!< 0x00080000 */
#define DMA_ISR_DMEIF2_Pos      (18U)
#define DMA_ISR_DMEIF2          (0x1U << DMA_ISR_DMEIF2_Pos)                 /*!< 0x00040000 */
#define DMA_ISR_FEIF2_Pos       (16U)
#define DMA_ISR_FEIF2           (0x1U << DMA_ISR_FEIF2_Pos)                  /*!< 0x00010000 */
#define DMA_ISR_TCIF3_Pos       (27U)
#define DMA_ISR_TCIF3           (0x1U << DMA_ISR_TCIF3_Pos)                  /*!< 0x08000000 */
#define DMA_ISR_HTIF3_Pos       (26U)
#define DMA_ISR_HTIF3           (0x1U << DMA_ISR_HTIF3_Pos)                  /*!< 0x04000000 */
#define DMA_ISR_TEIF3_Pos       (25U)
#define DMA_ISR_TEIF3           (0x1U << DMA_ISR_TEIF3_Pos)                  /*!< 0x02000000 */
#define DMA_ISR_DMEIF3_Pos      (24U)
#define DMA_ISR_DMEIF3          (0x1U << DMA_ISR_DMEIF3_Pos)                 /*!< 0x01000000 */
#define DMA_ISR_FEIF3_Pos       (22U)
#define DMA_ISR_FEIF3           (0x1U << DMA_ISR_FEIF3_Pos)                  /*!< 0x00400000 */
#define DMA_ISR_TCIF4_Pos       (5U)
#define DMA_ISR_TCIF4           (0x1U << DMA_ISR_TCIF4_Pos)                  /*!< 0x00000020 */
#define DMA_ISR_HTIF4_Pos       (4U)
#define DMA_ISR_HTIF4           (0x1U << DMA_ISR_HTIF4_Pos)                  /*!< 0x00000010 */
#define DMA_ISR_TEIF4_Pos       (3U)
#define DMA_ISR_TEIF4           (0x1U << DMA_ISR_TEIF4_Pos)                  /*!< 0x00000008 */
#define DMA_ISR_DMEIF4_Pos      (2U)
#define DMA_ISR_DMEIF4          (0x1U << DMA_ISR_DMEIF4_Pos)                 /*!< 0x00000004 */
#define DMA_ISR_FEIF4_Pos       (0U)
#define DMA_ISR_FEIF4           (0x1U << DMA_ISR_FEIF4_Pos)                  /*!< 0x00000001 */
#define DMA_ISR_TCIF5_Pos       (11U)
#define DMA_ISR_TCIF5           (0x1U << DMA_ISR_TCIF5_Pos)                  /*!< 0x00000800 */
#define DMA_ISR_HTIF5_Pos       (10U)
#define DMA_ISR_HTIF5           (0x1U << DMA_ISR_HTIF5_Pos)                  /*!< 0x00000400 */
#define DMA_ISR_TEIF5_Pos       (9U)
#define DMA_ISR_TEIF5           (0x1U << DMA_ISR_TEIF5_Pos)                  /*!< 0x00000200 */
#define DMA_ISR_DMEIF5_Pos      (8U)
#define DMA_ISR_DMEIF5          (0x1U << DMA_ISR_DMEIF5_Pos)                 /*!< 0x00000100 */
#define DMA_ISR_FEIF5_Pos       (6U)
#define DMA_ISR_FEIF5           (0x1U << DMA_ISR_FEIF5_Pos)                  /*!< 0x00000040 */
#define DMA_ISR_TCIF6_Pos       (21U)
#define DMA_ISR_TCIF6           (0x1U << DMA_ISR_TCIF6_Pos)                  /*!< 0x00200000 */
#define DMA_ISR_HTIF6_Pos       (20U)
#define DMA_ISR_HTIF6           (0x1U << DMA_ISR_HTIF6_Pos)                  /*!< 0x00100000 */
#define DMA_ISR_TEIF6_Pos       (19U)
#define DMA_ISR_TEIF6           (0x1U << DMA_ISR_TEIF6_Pos)                  /*!< 0x00080000 */
#define DMA_ISR_DMEIF6_Pos      (18U)
#define DMA_ISR_DMEIF6          (0x1U << DMA_ISR_DMEIF6_Pos)                 /*!< 0x00040000 */
#define DMA_ISR_FEIF6_Pos       (16U)
#define DMA_ISR_FEIF6           (0x1U << DMA_ISR_FEIF6_Pos)                  /*!< 0x00010000 */
#define DMA_ISR_TCIF7_Pos       (27U)
#define DMA_ISR_TCIF7           (0x1U << DMA_ISR_TCIF7_Pos)                  /*!< 0x08000000 */
#define DMA_ISR_HTIF7_Pos       (26U)
#define DMA_ISR_HTIF7           (0x1U << DMA_ISR_HTIF7_Pos)                  /*!< 0x04000000 */
#define DMA_ISR_TEIF7_Pos       (25U)
#define DMA_ISR_TEIF7           (0x1U << DMA_ISR_TEIF7_Pos)                  /*!< 0x02000000 */
#define DMA_ISR_DMEIF7_Pos      (24U)
#define DMA_ISR_DMEIF7          (0x1U << DMA_ISR_DMEIF7_Pos)                 /*!< 0x01000000 */
#define DMA_ISR_FEIF7_Pos       (22U)
#define DMA_ISR_FEIF7           (0x1U << DMA_ISR_FEIF7_Pos)                  /*!< 0x00400000 */

#define DMA_IFCR_CTCIF0_Pos     (5U)
#define DMA_IFCR_CTCIF0         (0x1U << DMA_IFCR_CTCIF0_Pos)                /*!< 0x00000020 */
#define DMA_IFCR_CHTIF0_Pos     (4U)
#define DMA_IFCR_CHTIF0         (0x1U << DMA_IFCR_CHTIF0_Pos)                /*!< 0x00000010 */
#define DMA_IFCR_CTEIF0_Pos     (3U)
#define DMA_IFCR_CTEIF0         (0x1U << DMA_IFCR_CTEIF0_Pos)                /*!< 0x00000008 */
#define DMA_IFCR_CDMEIF0_Pos    (2U)
#define DMA_IFCR_CDMEIF0        (0x1U << DMA_IFCR_CDMEIF0_Pos)               /*!< 0x00000004 */
#define DMA_IFCR_CFEIF0_Pos     (0U)
#define DMA_IFCR_CFEIF0         (0x1U << DMA_IFCR_CFEIF0_Pos)                /*!< 0x00000001 */
#define DMA_IFCR_CTCIF1_Pos     (11U)
#define DMA_IFCR_CTCIF1         (0x1U << DMA_IFCR_CTCIF1_Pos)                /*!< 0x00000800 */
#define DMA_IFCR_CHTIF1_Pos     (10U)
#define DMA_IFCR_CHTIF1         (0x1U << DMA_IFCR_CHTIF1_Pos)                /*!< 0x00000400 */
#define DMA_IFCR_CTEIF1_Pos     (9U)
#define DMA_IFCR_CTEIF1         (0x1U << DMA_IFCR_CTEIF1_Pos)                /*!< 0x00000200 */
#define DMA_IFCR_CDMEIF1_Pos    (8U)
#define DMA_IFCR_CDMEIF1        (0x1U << DMA_IFCR_CDMEIF1_Pos)               /*!< 0x00000100 */
#define DMA_IFCR_CFEIF1_Pos     (6U)
#define DMA_IFCR_CFEIF1         (0x1U << DMA_IFCR_CFEIF1_Pos)                /*!< 0x00000040 */
#define DMA_IFCR_CTCIF2_Pos     (21U)
#define DMA_IFCR_CTCIF2         (0x1U << DMA_IFCR_CTCIF2_Pos)                /*!< 0x00200000 */
#define DMA_IFCR_CHTIF2_Pos     (20U)
#define DMA_IFCR_CHTIF2         (0x1U << DMA_IFCR_CHTIF2_Pos)                /*!< 0x00100000 */
#define DMA_IFCR_CTEIF2_Pos     (19U)
#define DMA_IFCR_CTEIF2         (0x1U << DMA_IFCR_CTEIF2_Pos)                /*!< 0x00080000 */
#define DMA_IFCR_CDMEIF2_Pos    (18U)
#define DMA_IFCR_CDMEIF2        (0x1U << DMA_IFCR_CDMEIF2_Pos)               /*!< 0x00040000 */
#define DMA_IFCR_CFEIF2_Pos     (16U)
#define DMA_IFCR_CFEIF2         (0x1U << DMA_IFCR_CFEIF2_Pos)                /*!< 0x00010000 */
#define DMA_IFCR_CTCIF3_Pos     (27U)
#define DMA_IFCR_CTCIF3         (0x1U << DMA_IFCR_CTCIF3_Pos)                /*!< 0x08000000 */
#define DMA_IFCR_CHTIF3_Pos     (26U)
#define DMA_IFCR_CHTIF3         (0x1U << DMA_IFCR_CHTIF3_Pos)                /*!< 0x04000000 */
#define DMA_IFCR_CTEIF3_Pos     (25U)
#define DMA_IFCR_CTEIF3         (0x1U << DMA_IFCR_CTEIF3_Pos)                /*!< 0x02000000 */
#define DMA_IFCR_CDMEIF3_Pos    (24U)
#define DMA_IFCR_CDMEIF3        (0x1U << DMA_IFCR_CDMEIF3_Pos)               /*!< 0x01000000 */
#define DMA_IFCR_CFEIF3_Pos     (22U)
#define DMA_IFCR_CFEIF3         (0x1U << DMA_IFCR_CFEIF3_Pos)                /*!< 0x00400000 */
#define DMA_IFCR_CTCIF4_Pos     (5U)
#define DMA_IFCR_CTCIF4         (0x1U << DMA_IFCR_CTCIF4_Pos)                /*!< 0x00000020 */
#define DMA_IFCR_CHTIF4_Pos     (4U)
#define DMA_IFCR_CHTIF4         (0x1U << DMA_IFCR_CHTIF4_Pos)                /*!< 0x00000010 */
#define DMA_IFCR_CTEIF4_Pos     (3U)
#define DMA_IFCR_CTEIF4         (0x1U << DMA_IFCR_CTEIF4_Pos)                /*!< 0x00000008 */
#define DMA_IFCR_CDMEIF4_Pos    (2U)
#define DMA_IFCR_CDMEIF4        (0x1U << DMA_IFCR_CDMEIF4_Pos)               /*!< 0x00000004 */
#define DMA_IFCR_CFEIF4_Pos     (0U)
#define DMA_IFCR_CFEIF4         (0x1U << DMA_IFCR_CFEIF4_Pos)                /*!< 0x00000001 */
#define DMA_IFCR_CTCIF5_Pos     (11U)
#define DMA_IFCR_CTCIF5         (0x1U << DMA_IFCR_CTCIF5_Pos)                /*!< 0x00000800 */
#define DMA_IFCR_CHTIF5_Pos     (10U)
#define DMA_IFCR_CHTIF5         (0x1U << DMA_IFCR_CHTIF5_Pos)                /*!< 0x00000400 */
#define DMA_IFCR_CTEIF5_Pos     (9U)
#define DMA_IFCR_CTEIF5         (0x1U << DMA_IFCR_CTEIF5_Pos)                /*!< 0x00000200 */
#define DMA_IFCR_CDMEIF5_Pos    (8U)
#define DMA_IFCR_CDMEIF5        (0x1U << DMA_IFCR_CDMEIF5_Pos)               /*!< 0x00000100 */
#define DMA_IFCR_CFEIF5_Pos     (6U)
#define DMA_IFCR_CFEIF5         (0x1U << DMA_IFCR_CFEIF5_Pos)                /*!< 0x00000040 */
#define DMA_IFCR_CTCIF6_Pos     (21U)
#define DMA_IFCR_CTCIF6         (0x1U << DMA_IFCR_CTCIF6_Pos)                /*!< 0x00200000 */
#define DMA_IFCR_CHTIF6_Pos     (20U)
#define DMA_IFCR_CHTIF6         (0x1U << DMA_IFCR_CHTIF6_Pos)                /*!< 0x00100000 */
#define DMA_IFCR_CTEIF6_Pos     (19U)
#define DMA_IFCR_CTEIF6         (0x1U << DMA_IFCR_CTEIF6_Pos)                /*!< 0x00080000 */
#define DMA_IFCR_CDMEIF6_Pos    (18U)
#define DMA_IFCR_CDMEIF6        (0x1U << DMA_IFCR_CDMEIF6_Pos)               /*!< 0x00040000 */
#define DMA_IFCR_CFEIF6_Pos     (16U)
#define DMA_IFCR_CFEIF6         (0x1U << DMA_IFCR_CFEIF6_Pos)                /*!< 0x00010000 */
#define DMA_IFCR_CTCIF7_Pos     (27U)
#define DMA_IFCR_CTCIF7         (0x1U << DMA_IFCR_CTCIF7_Pos)                /*!< 0x08000000 */
#define DMA_IFCR_CHTIF7_Pos     (26U)
#define DMA_IFCR_CHTIF7         (0x1U << DMA_IFCR_CHTIF7_Pos)                /*!< 0x04000000 */
#define DMA_IFCR_CTEIF7_Pos     (25U)
#define DMA_IFCR_CTEIF7         (0x1U << DMA_IFCR_CTEIF7_Pos)                /*!< 0x02000000 */
#define DMA_IFCR_CDMEIF7_Pos    (24U)
#define DMA_IFCR_CDMEIF7        (0x1U << DMA_IFCR_CDMEIF7_Pos)               /*!< 0x01000000 */
#define DMA_IFCR_CFEIF7_Pos     (22U)
#define DMA_IFCR_CFEIF7         (0x1U << DMA_IFCR_CFEIF7_Pos)                /*!< 0x00400000 */

typedef struct
{
  __IO uint32_t ISR[2];  /*!< DMA interrupt status register,      Address offset: 0x00 */
  __IO uint32_t IFCR[2]; /*!< DMA interrupt flag clear register,  Address offset: 0x08 */
} DMA_TypeDef_Array;

#define DMANUM_(a, b, c, d)             a
#define DMANUM(a)                       DMANUM_(a)

#define DMACHN_(a, b, c, d)             b
#define DMACHN(a)                       DMACHN_(a)

#define DMASTREAM_(a, b, c, d)          c
#define DMASTREAM(a)                    DMASTREAM_(a)

#define DMAPRIORITY_(a, b, c, d)        d
#define DMAPRIORITY(a)                  DMAPRIORITY_(a)

#define DMAX_STREAMX_(a, b, c, d)       DMA ## a ## _Stream ## c
#define DMAX_STREAMX(a)                 DMAX_STREAMX_(a)

#define DMAX_STREAMX_IRQ_(a, b, c, d)   DMA ## a ## _Stream ## c ## _IRQn
#define DMAX_STREAMX_IRQ(a)             DMAX_STREAMX_IRQ_(a)

#define DMAX_STREAMX_IRQHANDLER_(a, b, c, d) DMA ## a ## _Stream ## c ## _IRQHandler
#define DMAX_STREAMX_IRQHANDLER(a)      DMAX_STREAMX_IRQHANDLER_(a)

/* Interrupt event pl: if(DMAX_ISR(LCD_DMA_TX) & DMAX_ISR_TCIF(LCD_DMA_TX))... */
#define DMAX_ISR_(a, b, c, d)           ((DMA_TypeDef_Array*) + DMA ## a ## _BASE)->ISR[c >> 2]
#define DMAX_ISR(a)                     DMAX_ISR_(a)

#define DMAX_ISR_TCIF_(a, b, c, d)      DMA_ISR_TCIF ## c
#define DMAX_ISR_TCIF(a)                DMAX_ISR_TCIF_(a)

#define DMAX_ISR_HTIF_(a, b, c, d)      DMA_ISR_HTIF ## c
#define DMAX_ISR_HTIF(a)                DMAX_ISR_HTIF_(a)

#define DMAX_ISR_TEIF_(a, b, c, d)      DMA_ISR_TEIF ## c
#define DMAX_ISR_TEIF(a)                DMAX_ISR_TEIF_(a)

#define DMAX_ISR_DMEIF_(a, b, c, d)     DMA_ISR_DMEIF ## c
#define DMAX_ISR_DMEIF(a)               DMAX_ISR_DMEIF_(a)

#define DMAX_ISR_FEIF_(a, b, c, d)      DMA_ISR_FEIF ## c
#define DMAX_ISR_FEIF(a)                DMAX_ISR_FEIF_(a)

/* Interrupt clear pl: DMAX_IFCR(LCD_DMA_TX) = DMAX_IFCR_CTCIF(LCD_DMA_TX) | DMAX_IFCR_CFEIF(LCD_DMA_TX); */
#define DMAX_IFCR_(a, b, c, d)          ((DMA_TypeDef_Array*) + DMA ## a ## _BASE)->IFCR[c >> 2]
#define DMAX_IFCR(a)                    DMAX_IFCR_(a)

#define DMAX_IFCR_CTCIF_(a, b, c, d)    DMA_IFCR_CTCIF ## c
#define DMAX_IFCR_CTCIF(a)              DMAX_IFCR_CTCIF_(a)

#define DMAX_IFCR_CHTIF_(a, b, c, d)    DMA_IFCR_CHTIF ## c
#define DMAX_IFCR_CHTIF(a)              DMAX_IFCR_CHTIF_(a)

#define DMAX_IFCR_CTEIF_(a, b, c, d)    DMA_IFCR_CTEIF ## c
#define DMAX_IFCR_CTEIF(a)              DMAX_IFCR_CTEIF_(a)

#define DMAX_IFCR_CDMEIF_(a, b, c, d)   DMA_IFCR_CDMEIF ## c
#define DMAX_IFCR_CDMEIF(a)             DMAX_IFCR_CDMEIF_(a)

#define DMAX_IFCR_CFEIF_(a, b, c, d)    DMA_IFCR_CFEIF ## c
#define DMAX_IFCR_CFEIF(a)              DMAX_IFCR_CFEIF_(a)

//-----------------------------------------------------------------------------
#if DMANUM(LCD_DMA) > 0
/* SPI DMA WRITE(a: src data pointer, b: target data pointer, c: source increment, d: target increment,
                 e: number of data, f: 0=8 bit, 1=16bit */
#define LCD_FSMC_DMA(a, b, c, d, e, f) {                                        \
  while(e)                                                                      \
  {                                                                             \
    DMAX_STREAMX(LCD_DMA)->CR = 0;                                              \
    DMAX_IFCR(LCD_DMA) = DMAX_IFCR_CTCIF(LCD_DMA);                              \
    DMAX_STREAMX(LCD_DMA)->PAR = (uint32_t)a;                                   \
    DMAX_STREAMX(LCD_DMA)->M0AR = (uint32_t)b;                                  \
    if(e > DMA_MAXSIZE)                                                         \
    {                                                                           \
      DMAX_STREAMX(LCD_DMA)->NDTR = DMA_MAXSIZE;                                \
      e -= DMA_MAXSIZE;                                                         \
    }                                                                           \
    else                                                                        \
    {                                                                           \
      DMAX_STREAMX(LCD_DMA)->NDTR = e;                                          \
      e = 0;                                                                    \
    }                                                                           \
    DMAX_STREAMX(LCD_DMA)->CR = TCIE |                                          \
      (c << DMA_SxCR_PINC_Pos) | (d << DMA_SxCR_MINC_Pos) |                     \
      (f << DMA_SxCR_PSIZE_Pos) | (f << DMA_SxCR_MSIZE_Pos) |                   \
      (2 << DMA_SxCR_DIR_Pos) | (DMACHN(LCD_DMA) << DMA_SxCR_CHSEL_Pos) |       \
      (DMAPRIORITY(LCD_DMA) << DMA_SxCR_PL_Pos);                                \
    DMAX_STREAMX(LCD_DMA)->CR |= DMA_SxCR_EN;                                   \
    WAIT_FOR_DMA_END;                                                           \
  }                                                                             }

#ifdef  osFeature_Semaphore
#define WAIT_FOR_DMA_END      osSemaphoreWait(BinarySemDmaHandle, osWaitForever)
#define TCIE                  DMA_SxCR_TCIE
#define LCD_DMA_IRQ
#else   /* #ifdef  osFeature_Semaphore */
#define WAIT_FOR_DMA_END      while(!(DMAX_ISR(LCD_DMA) & DMAX_ISR_TCIF(LCD_DMA)));
#define TCIE                  0
#endif  /* #else  osFeature_Semaphore */

#endif  /* #if LCD_DMA > 0 */

//-----------------------------------------------------------------------------
/* reset pin setting */
#define LCD_RST_ON            GPIOX_ODR(LCD_RST) = 0
#define LCD_RST_OFF           GPIOX_ODR(LCD_RST) = 1

//-----------------------------------------------------------------------------
/* LCD_CS pin mode setting when change the mode from LCD to TOUCH and back */
#define LCD_TS_ON             GPIOX_MODER(MODE_OUT, LCD_CS)
#define LCD_TS_OFF            GPIOX_MODER(MODE_ALTER, LCD_CS)

#if TS_ADC == 1
#define RCC_APB2ENR_ADCXEN    RCC_APB2ENR_ADC1EN
#define ADCX                  ADC1
#endif

#if TS_ADC == 2
#define RCC_APB2ENR_ADCXEN    RCC_APB2ENR_ADC2EN
#define ADCX                  ADC2
#endif

#if TS_ADC == 3
#define RCC_APB2ENR_ADCXEN    RCC_APB2ENR_ADC3EN
#define ADCX                  ADC3
#endif

#ifndef TS_XM_AN
#define TS_XM_AN              TS_XM
#endif

#ifndef TS_YP_AN
#define TS_YP_AN              TS_YP
#endif

/* if the touch AD pins differ from RS and WR pins, and them paralell linked */
#ifdef  ADCX
#if (GPIOX_PORTNUM(TS_XM) != GPIOX_PORTNUM(TS_XM_AN)) || (GPIOX_PIN(TS_XM) != GPIOX_PIN(TS_XM_AN)) || \
    (GPIOX_PORTNUM(TS_YP) != GPIOX_PORTNUM(TS_YP_AN)) || (GPIOX_PIN(TS_YP) != GPIOX_PIN(TS_YP_AN))
#define TS_AD_PIN_PARALELL
#endif
#endif  /* #ifdef  ADCX */

//-----------------------------------------------------------------------------
#ifdef LCD_DMA_IRQ
osSemaphoreId BinarySemDmaHandle;
void DMAX_STREAMX_IRQHANDLER(LCD_DMA)(void)
{
  if(DMAX_ISR(LCD_DMA) & DMAX_ISR_TCIF(LCD_DMA))
  {
    DMAX_IFCR(LCD_DMA) = DMAX_IFCR_CTCIF(LCD_DMA);
    osSemaphoreRelease(BinarySemDmaHandle);
  }
}
#endif

//-----------------------------------------------------------------------------
#ifdef  __GNUC__
#pragma GCC push_options
#pragma GCC optimize("O0")
#elif   defined(__CC_ARM)
#pragma push
#pragma O0
#endif
void LCD_IO_Delay(uint32_t c)
{
  while(c--);
}
#ifdef  __GNUC__
#pragma GCC pop_options
#elif   defined(__CC_ARM)
#pragma pop
#endif

//-----------------------------------------------------------------------------
void LCD_Delay(uint32_t Delay)
{
  HAL_Delay(Delay);
}

//-----------------------------------------------------------------------------
void LCD_IO_Bl_OnOff(uint8_t Bl)
{
  #if GPIOX_PORTNUM(LCD_BL) >= GPIOX_PORTNUM_A
  if(Bl)
    GPIOX_ODR(LCD_BL) = LCD_BLON;
  else
    GPIOX_ODR(LCD_BL) = 1 - LCD_BLON;
  #endif
}

//-----------------------------------------------------------------------------
void LCD_IO_Init(void)
{
  #ifdef ADCX
  RCC->AHB1ENR |= GPIOX_CLOCK(TS_XM) | GPIOX_CLOCK(TS_XP) | GPIOX_CLOCK(TS_YM) | GPIOX_CLOCK(TS_YP);
  #endif

  #if GPIOX_PORTNUM(LCD_RST) >= GPIOX_PORTNUM_A
  RCC->AHB1ENR |= GPIOX_CLOCK(LCD_RST);
  GPIOX_MODER(MODE_OUT, LCD_RST);       /* RST = GPIO OUT */
  GPIOX_ODR(LCD_RST) = 1;               /* RST = 1 */
  #endif

  #if GPIOX_PORTNUM(LCD_BL) >= GPIOX_PORTNUM_A
  RCC->APB1ENR |= GPIOX_CLOCK(LCD_BL);
  GPIOX_ODR(LCD_BL) = LCD_BLON;
  GPIOX_MODER(MODE_OUT, LCD_BL);
  #endif

  /* Set or Reset the control line */
  #if GPIOX_PORTNUM(LCD_RST) >= GPIOX_PORTNUM_A
  LCD_Delay(1);
  LCD_RST_ON;
  LCD_Delay(1);
  LCD_RST_OFF;
  #endif
  LCD_Delay(1);

  GPIOX_ODR(LCD_CS) = 1;

  #ifdef ADCX

  #ifdef TS_AD_PIN_PARALELL
  RCC->AHB1ENR |= GPIOX_CLOCK(TS_XM_AN) | GPIOX_CLOCK(TS_YP_AN);
  GPIOX_MODER(MODE_ANALOG_INPUT, TS_XM_AN); /* XM = AN_INPUT */
  GPIOX_MODER(MODE_ANALOG_INPUT, TS_YP_AN); /* YP = AN_INPUT */
  #endif

  RCC->APB2ENR |= RCC_APB2ENR_ADCXEN;
  ADCX->CR1 = ADC_CR1_DISCEN;
  ADCX->CR2 = ADC_CR2_ADON;

  #if TS_XM_ADCCH >= 10
  ADCX->SMPR1 |= TS_SAMPLETIME << (3 * (TS_XM_ADCCH - 10));
  #else
  ADCX->SMPR2 |= TS_SAMPLETIME << (3 * (TS_XM_ADCCH));
  #endif

  #if TS_YP_ADCCH >= 10
  ADCX->SMPR1 |= TS_SAMPLETIME << (3 * (TS_YP_ADCCH - 10));
  #else
  ADCX->SMPR2 |= TS_SAMPLETIME << (3 * (TS_YP_ADCCH));
  #endif

  #endif /* #ifdef ADCX */

  #if DMANUM(LCD_DMA) == 1
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
  #elif DMANUM(LCD_DMA) == 2
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
  #endif

  #ifdef LCD_DMA_IRQ
  osSemaphoreDef(myBinarySem01);
  BinarySemDmaHandle = osSemaphoreCreate(osSemaphore(myBinarySem01), 1);
  HAL_NVIC_SetPriority(DMAX_STREAMX_IRQ(LCD_DMA), configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY, 0);
  HAL_NVIC_EnableIRQ(DMAX_STREAMX_IRQ(LCD_DMA));
  osSemaphoreWait(BinarySemDmaHandle, 1);
  #endif
}

//-----------------------------------------------------------------------------
void LCD_IO_WriteCmd8(uint8_t Cmd)
{
  *(uint8_t *)LCD_ADDR_BASE = Cmd;
}

//-----------------------------------------------------------------------------
void LCD_IO_WriteCmd16(uint16_t Cmd)
{
  *(volatile uint16_t *)LCD_ADDR_BASE = RD(Cmd);
}

//-----------------------------------------------------------------------------
void LCD_IO_WriteData8(uint8_t Data)
{
  *(volatile uint8_t *)LCD_ADDR_DATA = Data;
}

//-----------------------------------------------------------------------------
void LCD_IO_WriteData16(uint16_t Data)
{
  *(volatile uint16_t *)LCD_ADDR_DATA = RD(Data);
}

//-----------------------------------------------------------------------------
void LCD_IO_WriteCmd8DataFill16(uint8_t Cmd, uint16_t Data, uint32_t Size)
{
  uint16_t d = RD(Data);
  *(volatile uint8_t *)LCD_ADDR_BASE = Cmd;

  #if DMANUM(LCD_DMA) == 0
  while(Size--)
    *(volatile uint16_t *)LCD_ADDR_DATA = d;

  #else
  LCD_FSMC_DMA(&d, LCD_ADDR_DATA, 0, 0, Size, 1);
  #endif
}

//-----------------------------------------------------------------------------
void LCD_IO_WriteCmd8MultipleData8(uint8_t Cmd, uint8_t *pData, uint32_t Size)
{
  *(volatile uint8_t *)LCD_ADDR_BASE = Cmd;

  #if DMANUM(LCD_DMA) == 0
  while(Size--)
  {
    *(volatile uint8_t *)LCD_ADDR_DATA =*pData;
    pData++;
  }

  #else
  LCD_FSMC_DMA(pData, LCD_ADDR_DATA, 1, 0, Size, 0);
  #endif
}

//-----------------------------------------------------------------------------
void LCD_IO_WriteCmd8MultipleData16(uint8_t Cmd, uint16_t *pData, uint32_t Size)
{
  *(volatile uint8_t *)LCD_ADDR_BASE = Cmd;

  #if DMANUM(LCD_DMA) == 0 || LCD_REVERSE16 == 0
  while(Size--)
  {
    *(volatile uint16_t *)LCD_ADDR_DATA = RD(*pData);
    pData++;
  }

  #else
  LCD_FSMC_DMA(pData, LCD_ADDR_DATA, 1, 0, Size, 1);
  #endif
}

//-----------------------------------------------------------------------------
void LCD_IO_WriteCmd16DataFill16(uint16_t Cmd, uint16_t Data, uint32_t Size)
{
  *(volatile uint16_t *)LCD_ADDR_BASE = RD(Cmd);
  uint16_t d = RD(Data);

  #if DMANUM(LCD_DMA) == 0
  while(Size--)
    *(volatile uint16_t *)LCD_ADDR_DATA = d;

  #else
  LCD_FSMC_DMA(&d, LCD_ADDR_DATA, 0, 0, Size, 1);
  #endif
}

//-----------------------------------------------------------------------------
void LCD_IO_WriteCmd16MultipleData8(uint16_t Cmd, uint8_t *pData, uint32_t Size)
{
  *(volatile uint16_t *)LCD_ADDR_BASE = RD(Cmd);

  #if DMANUM(LCD_DMA) == 0
  while(Size--)
  {
    *(volatile uint8_t *)LCD_ADDR_DATA =*pData;
    pData++;
  }

  #else
  LCD_FSMC_DMA(pData, LCD_ADDR_DATA, 1, 0, Size, 0);
  #endif
}

//-----------------------------------------------------------------------------
void LCD_IO_WriteCmd16MultipleData16(uint16_t Cmd, uint16_t *pData, uint32_t Size)
{
  *(volatile uint16_t *)LCD_ADDR_BASE = RD(Cmd);

  #if DMANUM(LCD_DMA) == 0 || LCD_REVERSE16 == 0
  while(Size--)
  {
    *(volatile uint16_t *)LCD_ADDR_DATA = RD(*pData);
    pData++;
  }

  #else
  LCD_FSMC_DMA(pData, LCD_ADDR_DATA, 1, 0, Size, 1);
  #endif
}

//-----------------------------------------------------------------------------
void LCD_IO_ReadCmd8MultipleData8(uint8_t Cmd, uint8_t *pData, uint32_t Size, uint32_t DummySize)
{
  #ifdef  __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-but-set-variable"
  uint8_t DummyData;
  #pragma GCC diagnostic pop
  #elif   defined(__CC_ARM)
  uint8_t DummyData;
  #endif

  *(volatile uint8_t *)LCD_ADDR_BASE = Cmd;

  while(DummySize--)
    DummyData = *(volatile uint8_t *)LCD_ADDR_DATA;

  #if DMANUM(LCD_DMA) == 0
  while(Size--)
  {
    *pData = *(volatile uint8_t *)LCD_ADDR_DATA;
    pData++;
  }

  #else
  LCD_FSMC_DMA(LCD_ADDR_DATA, pData, 0, 1, Size, 0);
  #endif
}

//-----------------------------------------------------------------------------
void LCD_IO_ReadCmd8MultipleData16(uint8_t Cmd, uint16_t *pData, uint32_t Size, uint32_t DummySize)
{
  #ifdef  __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-but-set-variable"
  uint8_t DummyData;
  #pragma GCC diagnostic pop
  #elif   defined(__CC_ARM)
  uint8_t DummyData;
  #endif

  *(volatile uint8_t *)LCD_ADDR_BASE = Cmd;

  while(DummySize)
  {
    DummyData = *(volatile uint8_t *)LCD_ADDR_DATA;
    DummySize--;
  }

  #if DMANUM(LCD_DMA) == 0 || LCD_REVERSE16 == 0
  while(Size--)
  {
    *pData = RD(*(volatile uint16_t *)LCD_ADDR_DATA);
    pData++;
  }

  #else
  LCD_FSMC_DMA(LCD_ADDR_DATA, pData, 0, 1, Size, 1);
  #endif
}

//-----------------------------------------------------------------------------
void LCD_IO_ReadCmd8MultipleData24to16(uint8_t Cmd, uint16_t *pData, uint32_t Size, uint32_t DummySize)
{
  uint8_t rgb888[3];

  #ifdef  __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-but-set-variable"
  uint8_t DummyData;
  #pragma GCC diagnostic pop
  #elif   defined(__CC_ARM)
  uint8_t DummyData;
  #endif

  *(volatile uint8_t *)LCD_ADDR_BASE = Cmd;

  while(DummySize--)
    DummyData = *(volatile uint8_t *)LCD_ADDR_DATA;

  while(Size--)
  {
    rgb888[0] = *(volatile uint8_t*)LCD_ADDR_DATA;
    rgb888[1] = *(volatile uint8_t*)LCD_ADDR_DATA;
    rgb888[2] = *(volatile uint8_t*)LCD_ADDR_DATA;
    #if LCD_REVERSE16 == 0
    *pData = ((rgb888[0] & 0xF8) << 8 | (rgb888[1] & 0xFC) << 3 | rgb888[2] >> 3);
    #else
    *pData = __REVSH((rgb888[0] & 0xF8) << 8 | (rgb888[1] & 0xFC) << 3 | rgb888[2] >> 3);
    #endif
    pData++;
  }
}

//-----------------------------------------------------------------------------
void LCD_IO_ReadCmd16MultipleData8(uint16_t Cmd, uint8_t *pData, uint32_t Size, uint32_t DummySize)
{
  #ifdef  __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-but-set-variable"
  uint8_t DummyData;
  #pragma GCC diagnostic pop
  #elif   defined(__CC_ARM)
  uint8_t DummyData;
  #endif

  *(volatile uint16_t *)LCD_ADDR_BASE = RD(Cmd);

  while(DummySize--)
    DummyData = *(volatile uint8_t *)LCD_ADDR_DATA;

  #if DMANUM(LCD_DMA) == 0
  while(Size--)
  {
    *pData = *(volatile uint8_t *)LCD_ADDR_DATA;
    pData++;
  }

  #else
  LCD_FSMC_DMA(LCD_ADDR_DATA, pData, 0, 1, Size, 0);
  #endif
}

//-----------------------------------------------------------------------------
void LCD_IO_ReadCmd16MultipleData16(uint16_t Cmd, uint16_t *pData, uint32_t Size, uint32_t DummySize)
{
  #ifdef  __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-but-set-variable"
  uint8_t DummyData;
  #pragma GCC diagnostic pop
  #elif   defined(__CC_ARM)
  uint8_t DummyData;
  #endif

  *(volatile uint16_t *)LCD_ADDR_BASE = RD(Cmd);

  while(DummySize--)
    DummyData = *(volatile uint8_t *)LCD_ADDR_DATA;

  #if DMANUM(LCD_DMA) == 0 || LCD_REVERSE16 == 0
  while(Size)
  {
    *pData = RD(*(volatile uint16_t *)LCD_ADDR_DATA);
    pData++;
    Size--;
  }

  #else
  LCD_FSMC_DMA(LCD_ADDR_DATA, pData, 0, 1, Size, 1);
  #endif
}

//-----------------------------------------------------------------------------
void LCD_IO_ReadCmd16MultipleData24to16(uint16_t Cmd, uint16_t *pData, uint32_t Size, uint32_t DummySize)
{
  uint8_t rgb888[3];

  #ifdef  __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-but-set-variable"
  uint8_t DummyData;
  #pragma GCC diagnostic pop
  #elif   defined(__CC_ARM)
  uint8_t DummyData;
  #endif

  *(volatile uint16_t *)LCD_ADDR_BASE = RD(Cmd);

  while(DummySize--)
    DummyData = *(volatile uint8_t *)LCD_ADDR_DATA;

  while(Size--)
  {
    rgb888[0] = *(volatile uint8_t*)LCD_ADDR_DATA;
    rgb888[1] = *(volatile uint8_t*)LCD_ADDR_DATA;
    rgb888[2] = *(volatile uint8_t*)LCD_ADDR_DATA;
    #if LCD_REVERSE16 == 0
    *pData = ((rgb888[0] & 0xF8) << 8 | (rgb888[1] & 0xFC) << 3 | rgb888[2] >> 3);
    #else
    *pData = __REVSH((rgb888[0] & 0xF8) << 8 | (rgb888[1] & 0xFC) << 3 | rgb888[2] >> 3);
    #endif
    pData++;
  }
}

//=============================================================================
#ifdef ADCX
/* CS = 1, X+ = 0, X- = 0; Y+ = in PU, Y- = in PU */
uint8_t TS_IO_DetectToch(void)
{
  uint8_t  ret;
  LCD_TS_ON;

  /* TS_XM<-LCD_RS, TS_XP<-LCD_D6, TS_YM<-LCD_D7, TS_YP<-LCD_WR */
  GPIOX_MODER(MODE_OUT, TS_XM);
  GPIOX_MODER(MODE_OUT, TS_XP);
  GPIOX_MODER(MODE_DIGITAL_INPUT, TS_YP);/* YP = D_INPUT */
  GPIOX_MODER(MODE_DIGITAL_INPUT, TS_YM);/* YM = D_INPUT */
  GPIOX_PUPDR(MODE_PU_UP, TS_YP);       /* pullup resistor on */

  GPIOX_ODR(TS_XP) = 0;                 /* XP = 0 */
  GPIOX_ODR(TS_XM) = 0;                 /* XM = 0 */

  LCD_IO_Delay(TS_AD_DELAY);

  if(GPIOX_IDR(TS_YP))
    ret = 0;                            /* Touchscreen is not touch */
  else
    ret = 1;                            /* Touchscreen is touch */

  /* pullup resistor off */
  GPIOX_PUPDR(MODE_PU_NONE, TS_YP);

  GPIOX_MODER(MODE_ALTER, TS_XM);
  GPIOX_MODER(MODE_ALTER, TS_XP);
  GPIOX_MODER(MODE_ALTER, TS_YM);
  GPIOX_MODER(MODE_ALTER, TS_YP);

  LCD_TS_OFF;
  return ret;
}

//-----------------------------------------------------------------------------
/* read the X position */
uint16_t TS_IO_GetX(void)
{
  uint16_t ret;
  LCD_TS_ON;

  /* TS_XM<-LCD_RS, TS_XP<-LCD_D6, TS_YM<-LCD_D7, TS_YP<-LCD_WR */
  GPIOX_MODER(MODE_DIGITAL_INPUT, TS_YM);/* YM = D_INPUT */
  GPIOX_MODER(MODE_ANALOG_INPUT, TS_YP); /* YP = AN_INPUT */ 
  GPIOX_MODER(MODE_OUT, TS_XM);
  GPIOX_MODER(MODE_OUT, TS_XP);

  GPIOX_ODR(TS_XP) = 0;                 /* XP = 0 */
  GPIOX_ODR(TS_XM) = 1;                 /* XM = 1 */

  ADCX->SQR3 = TS_YP_ADCCH;
  LCD_IO_Delay(TS_AD_DELAY);
  ADCX->CR2 |= ADC_CR2_SWSTART;
  while(!(ADCX->SR & ADC_SR_EOC));
  ret = ADCX->DR;

  GPIOX_MODER(MODE_ALTER, TS_XM);
  GPIOX_MODER(MODE_ALTER, TS_XP);
  GPIOX_MODER(MODE_ALTER, TS_YM);
  GPIOX_MODER(MODE_ALTER, TS_YP);

  LCD_TS_OFF;
  return ret;
}

//-----------------------------------------------------------------------------
/* read the Y position */
uint16_t TS_IO_GetY(void)
{
  uint16_t ret;
  LCD_TS_ON;

  /* TS_XM<-LCD_RS, TS_XP<-LCD_D6, TS_YM<-LCD_D7, TS_YP<-LCD_WR */
  GPIOX_MODER(MODE_DIGITAL_INPUT, TS_XP);/* XP = D_INPUT */
  GPIOX_MODER(MODE_ANALOG_INPUT, TS_XM); /* XM = AN_INPUT */
  GPIOX_MODER(MODE_OUT, TS_YM);
  GPIOX_MODER(MODE_OUT, TS_YP);

  GPIOX_ODR(TS_YM) = 0;                 /* YM = 0 */
  GPIOX_ODR(TS_YP) = 1;                 /* YP = 1 */

  ADCX->SQR3 = TS_XM_ADCCH;
  LCD_IO_Delay(TS_AD_DELAY);
  ADCX->CR2 |= ADC_CR2_SWSTART;
  while(!(ADCX->SR & ADC_SR_EOC));
  ret = ADCX->DR;

  GPIOX_MODER(MODE_ALTER, TS_XM);
  GPIOX_MODER(MODE_ALTER, TS_XP);
  GPIOX_MODER(MODE_ALTER, TS_YM);
  GPIOX_MODER(MODE_ALTER, TS_YP);

  LCD_TS_OFF;
  return ret;
}

//-----------------------------------------------------------------------------
/* read the Z1 position */
uint16_t TS_IO_GetZ1(void)
{
  uint16_t ret;
  LCD_TS_ON;

  /* TS_XM<-LCD_RS, TS_XP<-LCD_D6, TS_YM<-LCD_D7, TS_YP<-LCD_WR */
  GPIOX_MODER(MODE_ANALOG_INPUT, TS_XM); /* XM = AN_INPUT */
  GPIOX_MODER(MODE_ANALOG_INPUT, TS_YP); /* YP = AN_INPUT */
  GPIOX_MODER(MODE_OUT, TS_XP);
  GPIOX_MODER(MODE_OUT, TS_YM);

  GPIOX_ODR(TS_XP) = 0;                 /* XP = 0 */
  GPIOX_ODR(TS_YM) = 1;                 /* YM = 1 */

  ADCX->SQR3 = TS_YP_ADCCH;
  LCD_IO_Delay(TS_AD_DELAY);
  ADCX->CR2 |= ADC_CR2_SWSTART;
  while(!(ADCX->SR & ADC_SR_EOC));
  ret = ADCX->DR;

  GPIOX_MODER(MODE_ALTER, TS_XM);
  GPIOX_MODER(MODE_ALTER, TS_XP);
  GPIOX_MODER(MODE_ALTER, TS_YM);
  GPIOX_MODER(MODE_ALTER, TS_YP);

  LCD_TS_OFF;
  return ret;
}

//-----------------------------------------------------------------------------
/* read the Z2 position */
uint16_t TS_IO_GetZ2(void)
{
  uint16_t ret;
  LCD_TS_ON;

  /* TS_XM<-LCD_RS, TS_XP<-LCD_D6, TS_YM<-LCD_D7, TS_YP<-LCD_WR */
  GPIOX_MODER(MODE_ANALOG_INPUT, TS_XM); /* XM = AN_INPUT */
  GPIOX_MODER(MODE_ANALOG_INPUT, TS_YP); /* YP = AN_INPUT */
  GPIOX_MODER(MODE_OUT, TS_XP);
  GPIOX_MODER(MODE_OUT, TS_YM);

  GPIOX_ODR(TS_XP) = 0;                 /* XP = 0 */
  GPIOX_ODR(TS_YM) = 1;                 /* YM = 1 */

  ADCX->SQR3 = TS_XM_ADCCH;
  LCD_IO_Delay(TS_AD_DELAY);
  ADCX->CR2 |= ADC_CR2_SWSTART;
  while(!(ADCX->SR & ADC_SR_EOC));
  ret = ADCX->DR;

  GPIOX_MODER(MODE_ALTER, TS_XM);
  GPIOX_MODER(MODE_ALTER, TS_XP);
  GPIOX_MODER(MODE_ALTER, TS_YM);
  GPIOX_MODER(MODE_ALTER, TS_YP);

  LCD_TS_OFF;
  return ret;
}

//-----------------------------------------------------------------------------
#else  /* #ifdef ADCX */
__weak uint8_t   TS_IO_DetectToch(void) { return 0;}
__weak uint16_t  TS_IO_GetX(void)       { return 0;}
__weak uint16_t  TS_IO_GetY(void)       { return 0;}
__weak uint16_t  TS_IO_GetZ1(void)      { return 0;}
__weak uint16_t  TS_IO_GetZ2(void)      { return 0;}
#endif /* #else ADCX */

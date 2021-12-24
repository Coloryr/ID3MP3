/*
 * 8 bit paralell LCD GPIO driver for STM32H7
 * 5 controll pins (CS, RS, WR, RD, RST) + 8 data pins + backlight pin
 */

/* 
 * Author: Roberto Benjami
 * version:  2020.12.22
 */

#include "main.h"
#include "lcd.h"
#include "lcd_io_gpio8.h"

/* Change the byte order of 16bits data? */
#if LCD_REVERSE16 == 0
#define RD(a)                 __REVSH(a)
#endif
#if LCD_REVERSE16 == 1
#define RD(a)                 a
#endif

#define LCD_ADDR_DATA         (LCD_ADDR_BASE + (1 << (LCD_REGSELECT_BIT + 2)) - 2)

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

/*----------------------------------------------------------------------------
   CMSIS compatible chapter
----------------------------------------------------------------------------*/
#if     __STM32H7xx_CMSIS_DEVICE_VERSION == 0
#undef  __STM32H7xx_CMSIS_DEVICE_VERSION
#define __STM32H7xx_CMSIS_DEVICE_VERSION  ((__STM32H7xx_CMSIS_DEVICE_VERSION_MAIN << 24)\
                                          |(__STM32H7xx_CMSIS_DEVICE_VERSION_SUB1 << 16)\
                                          |(__STM32H7xx_CMSIS_DEVICE_VERSION_SUB2 << 8 )\
                                          |(__STM32H7xx_CMSIS_DEVICE_VERSION_RC))
#endif
//-----------------------------------------------------------------------------
/* GPIO mode */

/* values for GPIOX_MODER (io mode) */
#define MODE_DIGITAL_INPUT    0x0
#define MODE_OUT              0x1
#define MODE_ALTER            0x2
#define MODE_ANALOG_INPUT     0x3

/* values for GPIOX_OSPEEDR (output speed) */
#define MODE_SPD_LOW          0x0
#define MODE_SPD_MEDIUM       0x1
#define MODE_SPD_HIGH         0x2
#define MODE_SPD_VHIGH        0x3

/* values for GPIOX_OTYPER (output type: PP = push-pull, OD = open-drain) */
#define MODE_OT_PP            0x0
#define MODE_OT_OD            0x1

/* values for GPIOX_PUPDR (push up and down resistor) */
#define MODE_PU_NONE          0x0
#define MODE_PU_UP            0x1
#define MODE_PU_DOWN          0x2

#define GPIOX_(a, b)          GPIO ## a
#define GPIOX(a)              GPIOX_(a)

#define GPIOX_PIN_(a, b)      b
#define GPIOX_PIN(a)          GPIOX_PIN_(a)

#define GPIOX_AFR_(a,b,c)     GPIO ## b->AFR[c >> 3] = (GPIO ## b->AFR[c >> 3] & ~(0x0F << (4 * (c & 7)))) | (a << (4 * (c & 7)));
#define GPIOX_AFR(a, b)       GPIOX_AFR_(a, b)

#define GPIOX_MODER_(a,b,c)   GPIO ## b->MODER = (GPIO ## b->MODER & ~(3 << (2 * c))) | (a << (2 * c));
#define GPIOX_MODER(a, b)     GPIOX_MODER_(a, b)

#define GPIOX_OTYPER_(a,b,c)  GPIO ## b->OTYPER = (GPIO ## b->OTYPER & ~(1 << c)) | (a << c);
#define GPIOX_OTYPER(a, b)    GPIOX_OTYPER_(a, b)

#define GPIOX_OSPEEDR_(a,b,c) GPIO ## b->OSPEEDR = (GPIO ## b->OSPEEDR & ~(3 << (2 * c))) | (a << (2 * c));
#define GPIOX_OSPEEDR(a, b)   GPIOX_OSPEEDR_(a, b)

#define GPIOX_PUPDR_(a,b,c)   GPIO ## b->PUPDR = (GPIO ## b->PUPDR & ~(3 << (2 * c))) | (a << (2 * c));
#define GPIOX_PUPDR(a, b)     GPIOX_PUPDR_(a, b)

#define GPIOX_ODR_(a, b)      BITBAND_ACCESS(GPIO ## a ->ODR, b)
#define GPIOX_ODR(a)          GPIOX_ODR_(a)

#if __STM32H7xx_CMSIS_DEVICE_VERSION >= 0x01040000
#define GPIOX_SET_(a, b)      GPIO ## a ->BSRR = 1 << b
#define GPIOX_SET(a)          GPIOX_SET_(a)

#define GPIOX_CLR_(a, b)      GPIO ## a ->BSRR = 1 << (b + 16)
#define GPIOX_CLR(a)          GPIOX_CLR_(a)
#else
#define GPIOX_SET_(a, b)      GPIO ## a ->BSRRL = 1 << b
#define GPIOX_SET(a)          GPIOX_SET_(a)

#define GPIOX_CLR_(a, b)      GPIO ## a ->BSRRH = 1 << b
#define GPIOX_CLR(a)          GPIOX_CLR_(a)
#endif

#define GPIOX_IDR_(a, b)      (GPIO ## a ->IDR & (1 << b))
#define GPIOX_IDR(a)          GPIOX_IDR_(a)

#define GPIOX_LINE_(a, b)     EXTI_Line ## b
#define GPIOX_LINE(a)         GPIOX_LINE_(a)

#define GPIOX_CLOCK_(a, b)    RCC_AHB4ENR_GPIO ## a ## EN
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

/* reset pin setting */
#define LCD_RST_ON            GPIOX_CLR(LCD_RST)
#define LCD_RST_OFF           GPIOX_SET(LCD_RST)

//-----------------------------------------------------------------------------
void LCD_Delay(uint32_t Delay)
{
  HAL_Delay(Delay);
}

//-----------------------------------------------------------------------------
void LCD_IO_Bl_OnOff(uint8_t Bl)
{

}

//-----------------------------------------------------------------------------
void LCD_IO_Init(void)
{
  RCC->AHB4ENR |= GPIOX_CLOCK(LCD_RST);

  LCD_RST_OFF;                          /* RST = 1 */
  GPIOX_MODER(MODE_OUT, LCD_RST);

  /* Reset the LCD */
  LCD_Delay(1);
  LCD_RST_ON;                           /* RST = 0 */
  LCD_Delay(1);
  LCD_RST_OFF;                          /* RST = 1 */
  LCD_Delay(1);
}

//-----------------------------------------------------------------------------
void LCD_IO_WriteCmd8(uint8_t Cmd)
{
    *(volatile uint8_t *)LCD_ADDR_BASE = Cmd;
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

    while(Size--)
        *(volatile uint16_t *)LCD_ADDR_DATA = d;
}

//-----------------------------------------------------------------------------
void LCD_IO_WriteCmd8MultipleData8(uint8_t Cmd, uint8_t *pData, uint32_t Size)
{
    *(volatile uint8_t *)LCD_ADDR_BASE = Cmd;

    while(Size--)
    {
        *(volatile uint8_t *)LCD_ADDR_DATA =*pData;
        pData++;
    }
}

//-----------------------------------------------------------------------------
void LCD_IO_WriteCmd8MultipleData16(uint8_t Cmd, uint16_t *pData, uint32_t Size)
{
    *(volatile uint8_t *)LCD_ADDR_BASE = Cmd;

    while(Size--) {
        *(volatile uint16_t *) LCD_ADDR_DATA = RD(*pData);
        pData++;
    }
}

//-----------------------------------------------------------------------------
void LCD_IO_WriteCmd16DataFill16(uint16_t Cmd, uint16_t Data, uint32_t Size)
{
    *(volatile uint16_t *)LCD_ADDR_BASE = RD(Cmd);
    uint16_t d = RD(Data);

    while(Size--)
    *(volatile uint16_t *)LCD_ADDR_DATA = d;
}

//-----------------------------------------------------------------------------
void LCD_IO_WriteCmd16MultipleData8(uint16_t Cmd, uint8_t *pData, uint32_t Size) {
    *(volatile uint16_t *) LCD_ADDR_BASE = RD(Cmd);

    while (Size--) {
        *(volatile uint8_t *) LCD_ADDR_DATA = *pData;
        pData++;
    }
}

//-----------------------------------------------------------------------------
void LCD_IO_WriteCmd16MultipleData16(uint16_t Cmd, uint16_t *pData, uint32_t Size)
{
    *(volatile uint16_t *)LCD_ADDR_BASE = RD(Cmd);

    while(Size--)
    {
        *(volatile uint16_t *)LCD_ADDR_DATA = RD(*pData);
        pData++;
    }
}

//-----------------------------------------------------------------------------
void LCD_IO_ReadCmd8MultipleData8(uint8_t Cmd, uint8_t *pData, uint32_t Size, uint32_t DummySize)
{
    uint8_t DummyData;

    *(volatile uint8_t *)LCD_ADDR_BASE = Cmd;

    while(DummySize--)
        DummyData = *(volatile uint8_t *)LCD_ADDR_DATA;

    while(Size--)
  {
    *pData = *(volatile uint8_t *)LCD_ADDR_DATA;
    pData++;
  }
}

//-----------------------------------------------------------------------------
void LCD_IO_ReadCmd8MultipleData16(uint8_t Cmd, uint16_t *pData, uint32_t Size, uint32_t DummySize)
{
    uint8_t DummyData;

    *(volatile uint8_t *)LCD_ADDR_BASE = Cmd;

    while(DummySize)
    {
        DummyData = *(volatile uint8_t *)LCD_ADDR_DATA;
        DummySize--;
    }

    while(Size--)
    {
        *pData = RD(*(volatile uint16_t *)LCD_ADDR_DATA);
        pData++;
    }
}

//-----------------------------------------------------------------------------
void LCD_IO_ReadCmd8MultipleData24to16(uint8_t Cmd, uint16_t *pData, uint32_t Size, uint32_t DummySize)
{
    uint8_t rgb888[3];
    uint8_t DummyData;

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
    uint8_t DummyData;

    *(volatile uint16_t *)LCD_ADDR_BASE = RD(Cmd);

    while(DummySize--)
        DummyData = *(volatile uint8_t *)LCD_ADDR_DATA;

    while(Size--)
  {
    *pData = *(volatile uint8_t *)LCD_ADDR_DATA;
    pData++;
  }
}

//-----------------------------------------------------------------------------
void LCD_IO_ReadCmd16MultipleData16(uint16_t Cmd, uint16_t *pData, uint32_t Size, uint32_t DummySize)
{
    uint8_t DummyData;

    *(volatile uint16_t *)LCD_ADDR_BASE = RD(Cmd);

    while(DummySize--)
        DummyData = *(volatile uint8_t *)LCD_ADDR_DATA;

    while(Size)
    {
        *pData = RD(*(volatile uint16_t *)LCD_ADDR_DATA);
        pData++;
        Size--;
    }
}

//-----------------------------------------------------------------------------
void LCD_IO_ReadCmd16MultipleData24to16(uint16_t Cmd, uint16_t *pData, uint32_t Size, uint32_t DummySize)
{
    uint8_t rgb888[3];
    uint8_t DummyData;

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

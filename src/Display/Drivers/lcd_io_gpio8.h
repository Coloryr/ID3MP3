#ifndef LCD_IO_H
#define LCD_IO_H

/*
 * 8 bit paralell LCD GPIO driver for STM32H7
 * 5 controll pins (CS, RS, WR, RD, RST) + 8 data pins + backlight pin
 */

//=============================================================================
/* Lcd controll pins assign (A..K, 0..15) */
#define LCD_CS            C, 7
#define LCD_RS            E, 3
#define LCD_WR            D, 5
#define LCD_RD            D, 4  /* If not used leave it that way */
#define LCD_RST           C, 6  /* If not used leave it that way */

/* Lcd data pins assign (A..K, 0..15) */
#define LCD_D0            D, 14
#define LCD_D1            D, 15
#define LCD_D2            D, 0
#define LCD_D3            D, 1
#define LCD_D4            E, 7
#define LCD_D5            E, 8
#define LCD_D6            E, 9
#define LCD_D7            E, 10

//=============================================================================
/* Memory address
  - Bank1 (NE1) 0x60000000
  - Bank2 (NE2) 0x64000000
  - Bank3 (NE3) 0x68000000
  - Bank4 (NE4) 0x6C000000
  - REGSELECT_BIT: if example A18 pin -> 18 */
#define LCD_ADDR_BASE     0x60000000
#define LCD_REGSELECT_BIT 19

void LCD_Delay(uint32_t delay);

void LCD_IO_Init(void);

void LCD_IO_Bl_OnOff(uint8_t Bl);

void LCD_IO_WriteCmd8(uint8_t Cmd);

void LCD_IO_WriteCmd16(uint16_t Cmd);

void LCD_IO_WriteData8(uint8_t Data);

void LCD_IO_WriteData16(uint16_t Data);

void LCD_IO_WriteCmd8DataFill16(uint8_t Cmd, uint16_t Data, uint32_t Size);

void LCD_IO_WriteCmd8MultipleData8(uint8_t Cmd, uint8_t *pData, uint32_t Size);

void LCD_IO_WriteCmd8MultipleData16(uint8_t Cmd, uint16_t *pData, uint32_t Size);

void LCD_IO_WriteCmd16DataFill16(uint16_t Cmd, uint16_t Data, uint32_t Size);

void LCD_IO_WriteCmd16MultipleData8(uint16_t Cmd, uint8_t *pData, uint32_t Size);

void LCD_IO_WriteCmd16MultipleData16(uint16_t Cmd, uint16_t *pData, uint32_t Size);

void LCD_IO_ReadCmd8MultipleData8(uint8_t Cmd, uint8_t *pData, uint32_t Size, uint32_t DummySize);

void LCD_IO_ReadCmd8MultipleData16(uint8_t Cmd, uint16_t *pData, uint32_t Size, uint32_t DummySize);

void LCD_IO_ReadCmd8MultipleData24to16(uint8_t Cmd, uint16_t *pData, uint32_t Size, uint32_t DummySize);

void LCD_IO_ReadCmd16MultipleData8(uint16_t Cmd, uint8_t *pData, uint32_t Size, uint32_t DummySize);

void LCD_IO_ReadCmd16MultipleData16(uint16_t Cmd, uint16_t *pData, uint32_t Size, uint32_t DummySize);

void LCD_IO_ReadCmd16MultipleData24to16(uint16_t Cmd, uint16_t *pData, uint32_t Size, uint32_t DummySize);

#endif
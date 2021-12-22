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

/* Backlight control
   - BL: A..K, 0..15 (if not used -> X, 0)
   - BL_ON: 0 = active low level, 1 = active high level */
#define LCD_BL            X, 0  /* If not used leave it that way */
#define LCD_BLON          0

/* ADC converter number (1, 2, 3, or 0 if not used)
   - 0: analog touchscreen driver not used
   - 1..3: A/D converter number */
#define TS_ADC            1

/* If you use different pins for touch AD conversion than for lcd control, specify it
   (When using the same pins or not using a touchscreen -> X, 0
    in this case, LCD_RS and LCD_WR will be the selected AD pin on the touchscreen) */
#define TS_XM_AN          E, 3  /* If not used leave it that way */
#define TS_YP_AN          D, 5  /* If not used leave it that way */

/* Select the AD channels */
#define TS_XM_ADCCH       0
#define TS_YP_ADCCH       0

/* wait time for LCD write and read pulse
   - First, give 10, 20 values, then lower them to speed up the program.
     (values also depend on processor speed and LCD display speed) */
#define LCD_WRITE_DELAY  10
#define LCD_READ_DELAY   20
#define TS_AD_DELAY     500

//=============================================================================
/* Memory address
  - Bank1 (NE1) 0x60000000
  - Bank2 (NE2) 0x64000000
  - Bank3 (NE3) 0x68000000
  - Bank4 (NE4) 0x6C000000
  - REGSELECT_BIT: if example A18 pin -> 18 */
#define LCD_ADDR_BASE     0x60000000
#define LCD_REGSELECT_BIT 19

/*=============================================================================
I/O group optimization so that GPIO operations are not performed bit by bit:
Note: If the pins are in order, they will automatically optimize.
The example belongs to the following pins:
      LCD_D0<-D14, LCD_D1<-D15, LCD_D2<-D0, LCD_D3<-D1
      LCD_D4<-E7,  LCD_D5<-E8,  LCD_D6<-E9, LCD_D7<-E10 */
#if 1
/* datapins setting to output (data direction: STM32 -> LCD) */
#define LCD_DIRWRITE { /* D0..D1, D14..D15, E7..E10 <- 0b01 */ \
GPIOD->MODER = (GPIOD->MODER & ~0b11110000000000000000000000001111) | 0b01010000000000000000000000000101; \
GPIOE->MODER = (GPIOE->MODER & ~0b00000000001111111100000000000000) | 0b00000000000101010100000000000000; }
/* datapins setting to input (data direction: STM32 <- LCD) */
#define LCD_DIRREAD { /* D0..D1, D14..D15, E7..E10 <- 0b00 */ \
GPIOD->MODER = (GPIOD->MODER & ~0b11110000000000000000000000001111); \
GPIOE->MODER = (GPIOE->MODER & ~0b00000000001111111100000000000000); }
/* datapins write, STM32 -> LCD (write I/O pins from dt data) */
#define LCD_WRITE(dt) { /* D14..15 <- dt0..1, D0..1 <- dt2..3, E7..10 <- dt4..7 */ \
GPIOD->ODR = (GPIOD->ODR & ~0b1100000000000011) | (((dt & 0b00000011) << 14) | ((dt & 0b00001100) >> 2)); \
GPIOE->ODR = (GPIOE->ODR & ~0b0000011110000000) | ((dt & 0b11110000) << 3); }
/* datapins read, STM32 <- LCD (read from I/O pins and store to dt data) */
#define LCD_READ(dt) { /* dt0..1 <- D14..15, dt2..3 <- D0..1, dt4..7 <- E7..10 */ \
dt = ((GPIOD->IDR & 0b1100000000000000) >> 14) | ((GPIOD->IDR & 0b0000000000000011) << 2) | \
     ((GPIOE->IDR & 0b0000011110000000) >> 3); }
/* Note: the keil compiler cannot use binary numbers, convert it to hexadecimal */	 
#endif

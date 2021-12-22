#include "ts.h"
#include "main.h"
#include "stm32_adafruit_ts.h"
#include "ili9486.h"
#include "lcd_io_gpio8.h"

/* In the LCD board are multifunction pins: */
#define  TS_XP                LCD_D0
#define  TS_XM                LCD_RS
#define  TS_YP                LCD_CS
#define  TS_YM                LCD_D1

/* Touchscreen calibration data for 4 orientations */
#define  TS_CINDEX_0        {3385020, 333702, -667424, 1243070964, -458484, -13002, 1806391572}
#define  TS_CINDEX_1        {3385020, -458484, -13002, 1806391572, -333702, 667424, -163249584}
#define  TS_CINDEX_2        {3385020, -333702, 667424, -163249584, 458484, 13002, -184966992}
#define  TS_CINDEX_3        {3385020, 458484, 13002, -184966992, 333702, -667424, 1243070964}

#if (ILI9486_ORIENTATION == 0)
int32_t ts_cindex[] = TS_CINDEX_0;
#elif (ILI9486_ORIENTATION == 1)
int32_t  ts_cindex[] = TS_CINDEX_1;
#elif (ILI9486_ORIENTATION == 2)
int32_t  ts_cindex[] = TS_CINDEX_2;
#elif (ILI9486_ORIENTATION == 3)
int32_t  ts_cindex[] = TS_CINDEX_3;
#endif

void ts_init();

TS_DrvTypeDef my_ts = {
    ts_init,

};

TS_DrvTypeDef *ts_drv = &my_ts;

void ts_init() {

}
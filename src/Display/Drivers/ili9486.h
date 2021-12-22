/* Orientation
   - 0: 240x320 portrait 0'
   - 1: 320x240 landscape 90'
   - 2: 240x320 portrait 180'
   - 3: 320x240 landscape 270'
*/
#define  ILI9486_ORIENTATION      1

/* Color mode
   - 0: RGB565 (R:bit15..11, G:bit10..5, B:bit4..0)
   - 1: BRG565 (B:bit15..11, G:bit10..5, R:bit4..0)
*/
#define  ILI9486_COLORMODE        0

/* Analog touchscreen
   - 0: touchscreen disabled
   - 1: touchscreen enabled
*/
#define  ILI9486_TOUCH            0

/* For multi-threaded or interrupt use, Lcd and Touchscreen simultaneous use can cause confusion (since it uses common I/O resources)
   If enabled, the Lcd functions wait until the touchscreen functions are run. The touchscreen query is not executed when Lcd is busy.
   - 0: multi-threaded protection disabled
   - 1: multi-threaded protection enabled
*/
#define  ILI9486_MULTITASK_MUTEX   1

//-----------------------------------------------------------------------------
// ILI9486 physic resolution (in 0 orientation)
#define  ILI9486_LCD_PIXEL_WIDTH  320
#define  ILI9486_LCD_PIXEL_HEIGHT 480

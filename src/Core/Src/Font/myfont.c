#include "lvgl.h"
#include "main.h"

lv_font_t * sy16;
lv_font_t * sy24;
lv_font_t * sy32;

void load_font(){
    sy16 = lv_font_load("0:/font/sy16.font");
    sy24 = lv_font_load("0:/font/sy24.font");
    sy32 = lv_font_load("0:/font/sy32.font");
}
#ifndef ID3MP3_SHOW_H
#define ID3MP3_SHOW_H

#include "main.h"
#include "lvgl.h"

extern ramfast lv_obj_t *info;

#ifdef __cplusplus
extern "C" {
#endif
void info_init();
void info_close();
void info_show();

#ifdef __cplusplus
}
#endif

#endif //ID3MP3_SHOW_H

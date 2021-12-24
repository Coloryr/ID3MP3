#include "show.h"

ramfast lv_obj_t *info;

void info_init(){
    info = lv_label_create(lv_scr_act());
    lv_label_set_text(info, "init...");
    lv_obj_align(info, LV_ALIGN_CENTER, 0, 0);
}

void info_close(){
    lv_obj_add_flag(info, LV_OBJ_FLAG_HIDDEN);
}

void info_show(){
    lv_obj_clear_flag(info, LV_OBJ_FLAG_HIDDEN);
}



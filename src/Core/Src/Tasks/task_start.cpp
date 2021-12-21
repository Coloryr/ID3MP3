#include "task_start.h"
#include "task_led.h"
#include "lvgl.h"

#include "fatfs.h"
#include "lvgl.h"

/* BSP LCD driver */
#include "stm32_adafruit_lcd.h"
/* BSP TS driver */
#include "stm32_adafruit_ts.h"

 osThreadId_t ledTask;
const osThreadAttr_t ledTask_attributes = {
        .name = "ledTask",
        .stack_size = 512,
        .priority = (osPriority_t) osPriorityNormal,
};

 osThreadId_t lcdTask;
const osThreadAttr_t lcdTask_attributes = {
        .name = "lcdTask",
        .stack_size = 2048,
        .priority = (osPriority_t) osPriorityNormal,
};

 osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
        .name = "defaultTask",
        .stack_size = 512 * 4,
        .priority = (osPriority_t) osPriorityNormal,
};

void LCD_LOOP(void *argument) {
    for(;;){
        lv_tick_inc(1);
        lv_task_handler();
        osDelay(1);
    }
}

FRESULT res;

#define DISP_HOR_RES 320
#define DISP_VER_RES 480

ramfast static lv_disp_draw_buf_t draw_buf;
ramfast static lv_color_t buf1[DISP_HOR_RES * DISP_VER_RES / 10];                        /*Declare a buffer for 1/10 screen size*/

void my_disp_flush(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p)
{
    int32_t x, y;
    /*It's a very slow but simple implementation.
     *`set_pixel` needs to be written by you to a set pixel on the screen*/
    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
            BSP_LCD_DrawPixel(x, y, color_p->full);
            color_p++;
        }
    }

    lv_disp_flush_ready(disp);         /* Indicate you are ready with the flushing*/
}

//void my_touchpad_read(lv_indev_t * indev, lv_indev_data_t * data)
//{
//    /*`touchpad_is_pressed` and `touchpad_get_xy` needs to be implemented by you*/
//    if(touchpad_is_pressed()) {
//        data->state = LV_INDEV_STATE_PRESSED;
//        touchpad_get_xy(&data->point.x, &data->point.y);
//    } else {
//        data->state = LV_INDEV_STATE_RELEASED;
//    }
//
//}

static void set_angle(void * img, int32_t v)
{
    lv_img_set_angle((lv_obj_t*)img, v);
}

static void set_zoom(void * img, int32_t v)
{
    lv_img_set_zoom((lv_obj_t*)img, v);
}

void Lvgl_Config(){
    lv_init();
    lv_disp_draw_buf_init(&draw_buf, buf1, NULL, DISP_HOR_RES * DISP_VER_RES / 10);  /*Initialize the display buffer.*/

    static lv_disp_drv_t disp_drv;        /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);          /*Basic initialization*/
    disp_drv.flush_cb = my_disp_flush;    /*Set your driver function*/
    disp_drv.draw_buf = &draw_buf;        /*Assign the buffer to the display*/
    disp_drv.hor_res = DISP_HOR_RES;   /*Set the horizontal resolution of the display*/
    disp_drv.ver_res = DISP_VER_RES;   /*Set the vertical resolution of the display*/
    lv_disp_drv_register(&disp_drv);      /*Finally register the driver*/

//    static lv_indev_drv_t indev_drv;           /*Descriptor of a input device driver*/
//    lv_indev_drv_init(&indev_drv);             /*Basic initialization*/
//    indev_drv.type = LV_INDEV_TYPE_POINTER;    /*Touch pad is a pointer-like device*/
//    indev_drv.read_cb = my_touchpad_read;      /*Set your driver function*/
//    lv_indev_drv_register(&indev_drv);         /*Finally register the driver*/

    if(res!= FR_OK)
    {
        lv_obj_t * label1 = lv_label_create(lv_scr_act());
        lv_label_set_long_mode(label1, LV_LABEL_LONG_WRAP);     /*Break the long lines*/
        lv_label_set_recolor(label1, true);                      /*Enable re-coloring by commands in the text*/
        lv_label_set_text(label1, "#0000ff Re-color# #ff00ff words# #ff0000 of a# label, align the lines to the center "
                                  "and wrap long text automatically.");
        lv_obj_set_width(label1, 150);  /*Set smaller width to make the lines wrap*/
        lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(label1, LV_ALIGN_CENTER, 0, -40);


        lv_obj_t * label2 = lv_label_create(lv_scr_act());
        lv_label_set_long_mode(label2, LV_LABEL_LONG_SCROLL_CIRCULAR);     /*Circular scroll*/
        lv_obj_set_width(label2, 150);
        lv_label_set_text(label2, "Fatfs Error...");
        lv_obj_align(label2, LV_ALIGN_CENTER, 0, 40);
    }
    else
    {
        FIL fp;
        res=f_open(&fp,"0:test.jpg",FA_OPEN_EXISTING);
        if(res!= FR_OK)
        {
            lv_obj_t * label1 = lv_label_create(lv_scr_act());
            lv_label_set_long_mode(label1, LV_LABEL_LONG_WRAP);     /*Break the long lines*/
            lv_label_set_recolor(label1, true);                      /*Enable re-coloring by commands in the text*/
            lv_label_set_text(label1, "#0000ff Re-color# #ff00ff words# #ff0000 of a# label, align the lines to the center "
                                      "and wrap long text automatically.");
            lv_obj_set_width(label1, 150);  /*Set smaller width to make the lines wrap*/
            lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_align(label1, LV_ALIGN_CENTER, 0, -40);


            lv_obj_t * label2 = lv_label_create(lv_scr_act());
            lv_label_set_long_mode(label2, LV_LABEL_LONG_SCROLL_CIRCULAR);     /*Circular scroll*/
            lv_obj_set_width(label2, 150);
            lv_label_set_text(label2, "File"
                                      " Error...");
            lv_obj_align(label2, LV_ALIGN_CENTER, 0, 40);
        }

        else
        {
            /*Now create the actual image*/
            lv_obj_t * img = lv_img_create(lv_scr_act());
            lv_img_set_src(img, "0:test1.jpg");
            lv_img_set_size_mode(img, LV_IMG_SIZE_MODE_VIRTUAL);
            lv_img_set_pivot(img, 0, 0);    /*Rotate around the top left corner*/



        }
    }

}

void Fatfs_Config(){
    if ((res = f_mount(&SDFatFS, SDPath, 1)) != FR_OK) {
        while (1);
    }
}

void StartDefaultTask(void *argument) {
    Fatfs_Config();
    Lvgl_Config();

    ledTask = osThreadNew(task_led, nullptr, &ledTask_attributes);
    lcdTask = osThreadNew(LCD_LOOP, nullptr, &lcdTask_attributes);

    osThreadExit();
}

void task_init() {
    defaultTaskHandle = osThreadNew(StartDefaultTask, nullptr, &defaultTask_attributes);

}
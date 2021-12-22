#include "task_start.h"
#include "task_led.h"
#include "lvgl.h"

#include "fatfs.h"
#include "Pic/piclib.h"
#include "Font/myfont.h"
#include "Flash/w25q64.h"

/* BSP LCD driver */
#include "stm32_adafruit_lcd.h"
/* BSP TS driver */
#include "stm32_adafruit_ts.h"

#include "Font/myfont.h"

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

#define DISP_HOR_RES 480
#define DISP_VER_RES 320

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

void Lvgl_Config() {
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

    load_font();

    if (res != FR_OK) {
        lv_obj_t *label4 = lv_label_create(lv_scr_act());
        lv_label_set_long_mode(label4, LV_LABEL_LONG_SCROLL_CIRCULAR);     /*Circular scroll*/
        lv_obj_set_width(label4, 150);
        lv_label_set_text(label4, "Fatfs Error...");
        lv_obj_align(label4, LV_ALIGN_CENTER, 0, 0);
    } else {
        lv_obj_t *label1 = lv_label_create(lv_scr_act());
        lv_label_set_text(label1, "ノンフィクション!!\n测试\nabc");
        lv_obj_set_style_text_font(label1, sy16, 0);
        lv_obj_set_width(label1, 310);
        lv_obj_align(label1, LV_ALIGN_TOP_LEFT, 5, 0);

        lv_obj_t *label2 = lv_label_create(lv_scr_act());
        lv_label_set_text(label2, "ノンフィクション!!\n测试\nabc");
        lv_obj_set_style_text_font(label2, sy24, 0);
        lv_obj_set_width(label2, 310);
        lv_obj_align(label2, LV_ALIGN_LEFT_MID, 5, 0);

        lv_obj_t *label3 = lv_label_create(lv_scr_act());
        lv_label_set_text(label3, "ノンフィクション!!\n测试\nabc");
        lv_obj_set_style_text_font(label3, sy32, 0);
        lv_obj_set_width(label3, 310);
        lv_obj_align(label3, LV_ALIGN_BOTTOM_LEFT, 5, 0);
    }
}

void Fatfs_Config() {
    res = f_mount(&SDFatFS, SDPath, 1);
}

void flash_config() {
    uint8_t datatemp[32];
    W25QXX_Write((uint8_t *) "test", 0x0, 4);

    W25QXX_Read(datatemp,  0x0, 4);
}

void StartDefaultTask(void *argument) {
    piclib_init();
    Fatfs_Config();
    Lvgl_Config();
    flash_config();

    ledTask = osThreadNew(task_led, nullptr, &ledTask_attributes);
    lcdTask = osThreadNew(LCD_LOOP, nullptr, &lcdTask_attributes);

    osDelay(2000);
//    ai_load_picfile((const uint8_t *) "0:test.jpg", 0, 0, lcd_drv->getLcdPixelWidth(),
//                    lcd_drv->getLcdPixelHeight());

    osThreadExit();
}

void task_init() {
    defaultTaskHandle = osThreadNew(StartDefaultTask, nullptr, &defaultTask_attributes);

}
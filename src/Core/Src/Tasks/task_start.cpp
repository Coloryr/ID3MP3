#include "task_start.h"
#include "task_led.h"
#include "lvgl.h"

#include "fatfs.h"
#include "Pic/piclib.h"
#include "Font/myfont.h"
#include "Flash/w25q64.h"
#include "Show/show.h"

/* BSP LCD driver */
#include "stm32_adafruit_lcd.h"
/* BSP TS driver */
#include "stm32_adafruit_ts.h"

ramfast osThreadId_t ledTask;
const osThreadAttr_t ledTask_attributes = {
        .name = "ledTask",
        .stack_size = 512,
        .priority = (osPriority_t) osPriorityNormal,
};

ramfast osThreadId_t lcdTask;
const osThreadAttr_t lcdTask_attributes = {
        .name = "lcdTask",
        .stack_size = 2048,
        .priority = (osPriority_t) osPriorityNormal,
};

ramfast osThreadId_t defaultTaskHandle;
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
ram2 static lv_color_t buf1[DISP_HOR_RES * DISP_VER_RES / 2];                        /*Declare a buffer for 1/10 screen size*/

void my_disp_flush(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p) {
    uint16_t Xpos, Ypos, Xsize, Ysize, count;
    Xpos = area->x1;
    Ypos = area->y1;
    Xsize = area->x2 - area->x1 + 1;
    Ysize = area->y2 - area->y1 + 1;
    count = Xsize * Ysize;
    ili9486_SetDisplayWindow(Xpos, Ypos, Xsize, Ysize);
    LCD_IO_WriteCmd8(ILI9486_RAMWR);
    while (count--) {
        LCD_IO_WriteData16(color_p->full);
        color_p++;
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
    ledTask = osThreadNew(task_led, nullptr, &ledTask_attributes);
    lcdTask = osThreadNew(LCD_LOOP, nullptr, &lcdTask_attributes);

    Lvgl_Config();
    info_init();

    lv_label_set_text(info, "init piclib");
    piclib_init();
    lv_label_set_text(info, "init fatfs");
    Fatfs_Config();
    lv_label_set_text(info, "init flash");
    flash_config();
    lv_label_set_text(info, "init font");
    load_font();

    info_close();

    lv_obj_t *label1 = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(label1, &font_16, 0);
    lv_label_set_text(label1, "ノンフィクション!!");
    lv_obj_set_width(label1, 310);
    lv_obj_align(label1, LV_ALIGN_TOP_LEFT, 0, 0);

//    lv_obj_t *label2 = lv_label_create(lv_scr_act());
//    lv_label_set_text(label2, "ノンフィクション!!\n测试\nabc");
//    lv_obj_set_style_text_font(label2, &font_24, 0);
//    lv_obj_set_width(label2, 310);
//    lv_obj_align(label2, LV_ALIGN_LEFT_MID, 5, 0);
//
//    lv_obj_t *label3 = lv_label_create(lv_scr_act());
//    lv_label_set_text(label3, "ノンフィクション!!\n测试\nabc");
//    lv_obj_set_style_text_font(label3, &font_32, 0);
//    lv_obj_set_width(label3, 310);
//    lv_obj_align(label3, LV_ALIGN_BOTTOM_LEFT, 5, 0);

    osDelay(2000);
//    ai_load_picfile((const uint8_t *) "0:test.jpg", 0, 0, lcd_drv->getLcdPixelWidth(),
//                    lcd_drv->getLcdPixelHeight());

    osThreadExit();
}

void task_init() {
    defaultTaskHandle = osThreadNew(StartDefaultTask, nullptr, &defaultTask_attributes);

}
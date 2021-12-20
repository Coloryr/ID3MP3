#include "task_start.h"
#include "task_led.h"
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
        lv_tick_inc(10);
        lv_task_handler();
        osDelay(10);
    }
}

void StartDefaultTask(void *argument) {
    ledTask = osThreadNew(task_led, nullptr, &ledTask_attributes);
    lcdTask = osThreadNew(LCD_LOOP, nullptr, &lcdTask_attributes);
//    BSP_LCD_SetFont(&Font24);
//    BSP_LCD_DisplayStringAt(0, 0,(uint8_t*)"Hello world", LEFT_MODE);

    osThreadExit();
}

void task_init() {
    defaultTaskHandle = osThreadNew(StartDefaultTask, nullptr, &defaultTask_attributes);

}
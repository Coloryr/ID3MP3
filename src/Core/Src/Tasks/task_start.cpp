#include "task_start.h"
#include "task_led.h"

osThreadId_t ledTask;
const osThreadAttr_t ledTask_attributes = {
        .name = "ledTask",
        .stack_size = 512,
        .priority = (osPriority_t) osPriorityNormal,
};

//osThreadId_t defaultTaskHandle;
//const osThreadAttr_t defaultTask_attributes = {
//        .name = "defaultTask",
//        .stack_size = 128 * 4,
//        .priority = (osPriority_t) osPriorityNormal,
//};
//
//[[noreturn]] void StartDefaultTask(void *argument) {
//
//    osThreadNew(StartDefaultTask, nullptr, &defaultTask_attributes);
//
//    osThreadDetach(defaultTaskHandle);
//}

void task_init() {
//    defaultTaskHandle = osThreadNew(StartDefaultTask, nullptr, &defaultTask_attributes);
    ledTask = osThreadNew(task_led, nullptr, &ledTask_attributes);
}
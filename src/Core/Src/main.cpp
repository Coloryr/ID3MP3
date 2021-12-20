#include "main.h"
#include "cmsis_os.h"
#include "init/init.h"

osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
        .name = "defaultTask",
        .stack_size = 128 * 4,
        .priority = (osPriority_t) osPriorityNormal,
};

[[noreturn]] void StartDefaultTask(void *argument) {

    osThreadDetach(defaultTaskHandle);
}


int main() {
    init();

    osKernelInitialize();
    defaultTaskHandle = osThreadNew(StartDefaultTask, nullptr, &defaultTask_attributes);

    osKernelStart();
    for (;;);
}



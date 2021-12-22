#include "main.h"
#include "cmsis_os.h"
#include "init/init.h"
#include "tasks/task_start.h"

int main() {
    __HAL_RCC_D2SRAM1_CLK_ENABLE();
    __HAL_RCC_D2SRAM2_CLK_ENABLE();
    __HAL_RCC_D2SRAM3_CLK_ENABLE();
    init();
    osKernelInitialize();

    task_init();

    osKernelStart();
    for (;;);
}



#include "main.h"
#include "cmsis_os.h"
#include "init/init.h"
#include "tasks/task_start.h"

int main() {
    __HAL_RCC_D2SRAM1_CLK_ENABLE();
    __HAL_RCC_D2SRAM2_CLK_ENABLE();
    __HAL_RCC_D2SRAM3_CLK_ENABLE();
    SCB_EnableICache();//使能I-Cache
    SCB_EnableDCache();//使能D-Cache
    SCB->CACR |= 1 << 2;   //强制D-Cache透写,如不开启,实际使用中可能遇到各种问题
    init();
    osKernelInitialize();

    task_init();

    osKernelStart();
    for (;;);
}



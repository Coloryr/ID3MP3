#include "task_led.h"

void task_led(void *arg) {
    for (;;) {
        LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_1);
        osDelay(100);
        LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_1);
        osDelay(100);
    }
}
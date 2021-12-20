#include "main.h"
#include "cmsis_os.h"
#include "init/init.h"
#include "tasks/task_start.h"

int main() {
    init();
    osKernelInitialize();

    task_init();

    osKernelStart();
    for (;;);
}



#include "init.h"
#include "main.h"
#include "cmsis_os.h"
#include "libjpeg.h"
#include "fatfs.h"
#include "lvgl.h"

/* BSP LCD driver */
#include "stm32_adafruit_lcd.h"
/* BSP TS driver */
#include "stm32_adafruit_ts.h"

JPEG_HandleTypeDef hjpeg;

QSPI_HandleTypeDef hqspi;

SD_HandleTypeDef hsd1;

SRAM_HandleTypeDef hsram1;

FRESULT res;

#define DISP_HOR_RES 320
#define DISP_VER_RES 480

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[DISP_HOR_RES * DISP_VER_RES / 10];                        /*Declare a buffer for 1/10 screen size*/

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


        /*Now create the actual image*/
        lv_obj_t * img = lv_img_create(lv_scr_act());
        lv_img_set_src(img, "S:test.jpg");
        lv_img_set_pivot(img, 0, 0);    /*Rotate around the top left corner*/
    }

}

void Fatfs_Config(){
    res = f_mount(&SDFatFS,(TCHAR const*)SDPath, 1);
}

void MPU_Config() {

    /* Disables the MPU */
    LL_MPU_Disable();
    /** Initializes and configures the Region and the memory to be protected
    */
    LL_MPU_ConfigRegion(LL_MPU_REGION_NUMBER0, 0x0, 0x2400000,
                        LL_MPU_REGION_SIZE_512KB | LL_MPU_REGION_NO_ACCESS | LL_MPU_INSTRUCTION_ACCESS_ENABLE |
                        LL_MPU_ACCESS_NOT_SHAREABLE | LL_MPU_ACCESS_CACHEABLE | LL_MPU_ACCESS_BUFFERABLE);
    LL_MPU_EnableRegion(LL_MPU_REGION_NUMBER0);
    /* Enables the MPU */
    LL_MPU_Enable(LL_MPU_CTRL_PRIVILEGED_DEFAULT);

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config() {
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);
    while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_4) {
    }
    LL_PWR_ConfigSupply(LL_PWR_LDO_SUPPLY);
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE0);
    LL_RCC_HSE_Enable();

    /* Wait till HSE is ready */
    while (LL_RCC_HSE_IsReady() != 1) {

    }
    LL_RCC_HSE_EnableCSS();
    LL_RCC_PLL_SetSource(LL_RCC_PLLSOURCE_HSE);
    LL_RCC_PLL1P_Enable();
    LL_RCC_PLL1R_Enable();
    LL_RCC_PLL1_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_4_8);
    LL_RCC_PLL1_SetVCOOutputRange(LL_RCC_PLLVCORANGE_WIDE);
    LL_RCC_PLL1_SetM(5);
    LL_RCC_PLL1_SetN(192);
    LL_RCC_PLL1_SetP(2);
    LL_RCC_PLL1_SetQ(2);
    LL_RCC_PLL1_SetR(2);
    LL_RCC_PLL1_Enable();

    /* Wait till PLL is ready */
    while (LL_RCC_PLL1_IsReady() != 1) {
    }

    /* Intermediate AHB prescaler 2 when target frequency clock is higher than 80 MHz */
    LL_RCC_SetAHBPrescaler(LL_RCC_AHB_DIV_2);

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL1);
    LL_RCC_SetSysPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAHBPrescaler(LL_RCC_AHB_DIV_2);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);
    LL_RCC_SetAPB3Prescaler(LL_RCC_APB3_DIV_2);
    LL_RCC_SetAPB4Prescaler(LL_RCC_APB4_DIV_2);
    LL_SetSystemCoreClock(480000000);

    /* Update the time base */
    if (HAL_InitTick(TICK_INT_PRIORITY) != HAL_OK) {
        Error_Handler();
    }
    LL_RCC_HSE_EnableCSS();
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config() {
    LL_RCC_PLL2P_Enable();
    LL_RCC_PLL2R_Enable();
    LL_RCC_PLL2_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_2_4);
    LL_RCC_PLL2_SetVCOOutputRange(LL_RCC_PLLVCORANGE_WIDE);
    LL_RCC_PLL2_SetM(10);
    LL_RCC_PLL2_SetN(160);
    LL_RCC_PLL2_SetP(2);
    LL_RCC_PLL2_SetQ(2);
    LL_RCC_PLL2_SetR(2);
    LL_RCC_PLL2_Enable();

    /* Wait till PLL is ready */
    while (LL_RCC_PLL2_IsReady() != 1) {
    }

}

/**
  * @brief JPEG Initialization Function
  * @param None
  * @retval None
  */
void MX_JPEG_Init() {

    /* USER CODE BEGIN JPEG_Init 0 */

    /* USER CODE END JPEG_Init 0 */

    /* USER CODE BEGIN JPEG_Init 1 */

    /* USER CODE END JPEG_Init 1 */
    hjpeg.Instance = JPEG;
    if (HAL_JPEG_Init(&hjpeg) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN JPEG_Init 2 */

    /* USER CODE END JPEG_Init 2 */

}

/**
  * @brief QUADSPI Initialization Function
  * @param None
  * @retval None
  */
void MX_QUADSPI_Init() {

    /* USER CODE BEGIN QUADSPI_Init 0 */

    /* USER CODE END QUADSPI_Init 0 */

    /* USER CODE BEGIN QUADSPI_Init 1 */

    /* USER CODE END QUADSPI_Init 1 */
    /* QUADSPI parameter configuration*/
    hqspi.Instance = QUADSPI;
    hqspi.Init.ClockPrescaler = 255;
    hqspi.Init.FifoThreshold = 1;
    hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_NONE;
    hqspi.Init.FlashSize = 1;
    hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;
    hqspi.Init.ClockMode = QSPI_CLOCK_MODE_0;
    hqspi.Init.FlashID = QSPI_FLASH_ID_1;
    hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
    if (HAL_QSPI_Init(&hqspi) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN QUADSPI_Init 2 */

    /* USER CODE END QUADSPI_Init 2 */

}

/**
  * @brief SDMMC1 Initialization Function
  * @param None
  * @retval None
  */
void MX_SDMMC1_SD_Init() {

    /* USER CODE BEGIN SDMMC1_Init 0 */

    /* USER CODE END SDMMC1_Init 0 */

    /* USER CODE BEGIN SDMMC1_Init 1 */

    /* USER CODE END SDMMC1_Init 1 */
    hsd1.Instance = SDMMC1;
    hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
    hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    hsd1.Init.BusWide = SDMMC_BUS_WIDE_4B;
    hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
    hsd1.Init.ClockDiv = 0;
    /* USER CODE BEGIN SDMMC1_Init 2 */

    /* USER CODE END SDMMC1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
void MX_SPI1_Init() {

    /* USER CODE BEGIN SPI1_Init 0 */

    /* USER CODE END SPI1_Init 0 */

    LL_SPI_InitTypeDef SPI_InitStruct = {0};

    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    LL_RCC_SetSPIClockSource(LL_RCC_SPI123_CLKSOURCE_PLL2P);

    /* Peripheral clock enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);

    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOA);
    /**SPI1 GPIO Configuration
    PA5   ------> SPI1_SCK
    PA6   ------> SPI1_MISO
    PA7   ------> SPI1_MOSI
    */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_5 | LL_GPIO_PIN_6 | LL_GPIO_PIN_7;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USER CODE BEGIN SPI1_Init 1 */

    /* USER CODE END SPI1_Init 1 */
    /* SPI1 parameter configuration*/
    SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
    SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
    SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_4BIT;
    SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
    SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;
    SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;
    SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV2;
    SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
    SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
    SPI_InitStruct.CRCPoly = 0x0;
    LL_SPI_Init(SPI1, &SPI_InitStruct);
    LL_SPI_SetStandard(SPI1, LL_SPI_PROTOCOL_MOTOROLA);
    LL_SPI_EnableNSSPulseMgt(SPI1);
    /* USER CODE BEGIN SPI1_Init 2 */

    /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
void MX_USART1_Init() {

    /* USER CODE BEGIN USART1_Init 0 */

    /* USER CODE END USART1_Init 0 */

    LL_USART_InitTypeDef USART_InitStruct = {0};
    LL_USART_ClockInitTypeDef USART_ClockInitStruct = {0};

    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    LL_RCC_SetUSARTClockSource(LL_RCC_USART16_CLKSOURCE_PCLK2);

    /* Peripheral clock enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);

    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOB);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOA);
    /**USART1 GPIO Configuration
    PB14   ------> USART1_TX
    PB15   ------> USART1_RX
    PA8   ------> USART1_CK
    */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_14 | LL_GPIO_PIN_15;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_4;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_8;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USER CODE BEGIN USART1_Init 1 */

    /* USER CODE END USART1_Init 1 */
    USART_InitStruct.PrescalerValue = LL_USART_PRESCALER_DIV1;
    USART_InitStruct.BaudRate = 115200;
    USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity = LL_USART_PARITY_NONE;
    USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
    USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_8;

    LL_USART_Init(USART1, &USART_InitStruct);
    LL_USART_SetTXFIFOThreshold(USART1, LL_USART_FIFOTHRESHOLD_1_8);
    LL_USART_SetRXFIFOThreshold(USART1, LL_USART_FIFOTHRESHOLD_1_8);
    USART_ClockInitStruct.ClockOutput = LL_USART_CLOCK_ENABLE;
    USART_ClockInitStruct.ClockPolarity = LL_USART_POLARITY_LOW;
    USART_ClockInitStruct.ClockPhase = LL_USART_PHASE_1EDGE;
    USART_ClockInitStruct.LastBitClockPulse = LL_USART_LASTCLKPULSE_NO_OUTPUT;
    LL_USART_ClockInit(USART1, &USART_ClockInitStruct);
    LL_USART_DisableFIFO(USART1);
    LL_USART_ConfigSyncMode(USART1);

    /* USER CODE BEGIN WKUPType USART1 */

    /* USER CODE END WKUPType USART1 */

    LL_USART_Enable(USART1);

    /* Polling USART1 initialisation */
    while ((!(LL_USART_IsActiveFlag_TEACK(USART1))) || (!(LL_USART_IsActiveFlag_REACK(USART1)))) {
    }
    /* USER CODE BEGIN USART1_Init 2 */

    /* USER CODE END USART1_Init 2 */

}

/* FMC initialization function */
void MX_FMC_Init() {

    /* USER CODE BEGIN FMC_Init 0 */

    /* USER CODE END FMC_Init 0 */

    FMC_NORSRAM_TimingTypeDef Timing = {0};

    /* USER CODE BEGIN FMC_Init 1 */

    /* USER CODE END FMC_Init 1 */

    /** Perform the SRAM1 memory initialization sequence
    */
    hsram1.Instance = FMC_NORSRAM_DEVICE;
    hsram1.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
    /* hsram1.Init */
    hsram1.Init.NSBank = FMC_NORSRAM_BANK1;
    hsram1.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE;
    hsram1.Init.MemoryType = FMC_MEMORY_TYPE_SRAM;
    hsram1.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_8;
    hsram1.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_DISABLE;
    hsram1.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
    hsram1.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;
    hsram1.Init.WriteOperation = FMC_WRITE_OPERATION_ENABLE;
    hsram1.Init.WaitSignal = FMC_WAIT_SIGNAL_DISABLE;
    hsram1.Init.ExtendedMode = FMC_EXTENDED_MODE_DISABLE;
    hsram1.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_DISABLE;
    hsram1.Init.WriteBurst = FMC_WRITE_BURST_DISABLE;
    hsram1.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;
    hsram1.Init.WriteFifo = FMC_WRITE_FIFO_ENABLE;
    hsram1.Init.PageSize = FMC_PAGE_SIZE_NONE;
    /* Timing */
    Timing.AddressSetupTime = 15;
    Timing.AddressHoldTime = 15;
    Timing.DataSetupTime = 255;
    Timing.BusTurnAroundDuration = 15;
    Timing.CLKDivision = 16;
    Timing.DataLatency = 17;
    Timing.AccessMode = FMC_ACCESS_MODE_A;
    /* ExtTiming */

    if (HAL_SRAM_Init(&hsram1, &Timing, NULL) != HAL_OK) {
        Error_Handler();
    }

    /* USER CODE BEGIN FMC_Init 2 */

    /* USER CODE END FMC_Init 2 */
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
void MX_GPIO_Init() {
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOE);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOC);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOH);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOA);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOB);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOD);

    /**/
    LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_1);
    LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_6);

    /**/
    GPIO_InitStruct.Pin = LL_GPIO_PIN_0;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /**/
    GPIO_InitStruct.Pin = LL_GPIO_PIN_1;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
    LL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void init(){
    MPU_Config();
    SCB_EnableICache();
    SCB_EnableDCache();
    HAL_Init();
    SystemClock_Config();
    PeriphCommonClock_Config();
    MX_GPIO_Init();
//    MX_FMC_Init();
    MX_JPEG_Init();
    MX_QUADSPI_Init();
    MX_SDMMC1_SD_Init();
    MX_SPI1_Init();
    MX_USART1_Init();
    MX_FATFS_Init();
    MX_LIBJPEG_Init();

    BSP_LCD_Init();

    Lvgl_Config();
    Fatfs_Config();
    //    BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
}
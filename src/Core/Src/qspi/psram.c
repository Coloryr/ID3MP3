#include "psram.h"
#include "main.h"
#include "qspi.h"
#include "cmsis_os2.h"

static uint8_t PSRAM_Id [10] = {0};

#define PSEL LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_4);
#define PUSEL LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_4);

void QSPI_PSRAM_Init() {

    PSEL
    QSPI_Send_CMD(RESET_ENABLE_CMD, 0, 0, QSPI_INSTRUCTION_1_LINE,
                  QSPI_ADDRESS_NONE, QSPI_ADDRESS_24_BITS,QSPI_DATA_NONE);
    PUSEL

    PSEL
    QSPI_Send_CMD(RESET_CMD, 0, 0, QSPI_INSTRUCTION_1_LINE,
                  QSPI_ADDRESS_NONE, QSPI_ADDRESS_24_BITS,QSPI_DATA_NONE);
    PUSEL

    PSEL
    QSPI_Send_CMD(RESET_ENABLE_CMD, 0, 0, QSPI_INSTRUCTION_1_LINE,
                  QSPI_ADDRESS_NONE, QSPI_ADDRESS_24_BITS,QSPI_DATA_NONE);
    PUSEL

    PSEL
    QSPI_Send_CMD(RESET_CMD, 0, 0, QSPI_INSTRUCTION_1_LINE,
                  QSPI_ADDRESS_NONE, QSPI_ADDRESS_24_BITS,QSPI_DATA_NONE);
    PUSEL

    QSPI_PSRAM_ReadID();

    PSEL
    QSPI_Send_CMD(ENTER_QUAD_MODE_CMD, 0, 0, QSPI_INSTRUCTION_1_LINE,
                  QSPI_ADDRESS_NONE, QSPI_ADDRESS_24_BITS,QSPI_DATA_NONE);
    PUSEL

}

void QSPI_PSRAM_ReadID() {
    PSEL
    QSPI_Send_CMD(READ_ID_CMD, 0, 0, QSPI_INSTRUCTION_1_LINE,
                  QSPI_ADDRESS_1_LINE, QSPI_ADDRESS_24_BITS,QSPI_DATA_1_LINE);

    if (QSPI_Receive(PSRAM_Id, 8) != HAL_OK) {
        while (1);
    }
    PUSEL

    PSRAM_Id[9] = PSRAM_Id[9];
}

void QSPI_PSRAM_Read(uint8_t *pData,uint32_t addr, uint32_t bufSize) {
    PSEL
    QSPI_Send_CMD(FAST_READ_4_BYTE_ADDR_CMD, addr, 6, QSPI_INSTRUCTION_4_LINES, QSPI_ADDRESS_4_LINES,
                  QSPI_ADDRESS_24_BITS,QSPI_DATA_4_LINES);
    if (QSPI_Receive(pData, bufSize) != HAL_OK) {
        while (1);
    }
    osDelay(1);
    PUSEL
}

void QSPI_PSRAM_Write(uint8_t *pData,uint32_t addr, uint32_t bufSize) {
    PSEL
    QSPI_Send_CMD(WRITE_4_BYTE_ADDR_CMD, addr, 0, QSPI_INSTRUCTION_4_LINES, QSPI_ADDRESS_4_LINES,
                  QSPI_ADDRESS_24_BITS,QSPI_DATA_4_LINES);
    if (QSPI_Transmit(pData, bufSize) != HAL_OK) {
        while (1);
    }
    osDelay(1);
    PUSEL
}
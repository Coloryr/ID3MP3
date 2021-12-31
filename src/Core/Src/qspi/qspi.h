#ifndef ID3MP3_QSPI_H
#define ID3MP3_QSPI_H

#include "main.h"

extern QSPI_HandleTypeDef hqspi;

void QSPI_Send_CMD(uint32_t instruction, uint32_t address, uint32_t dummyCycles, uint32_t instructionMode,
                   uint32_t addressMode, uint32_t addressSize, uint32_t dataMode);

uint8_t QSPI_Receive(uint8_t *buf, uint32_t datalen);

uint8_t QSPI_Transmit(uint8_t *buf, uint32_t datalen);

#endif //ID3MP3_QSPI_H

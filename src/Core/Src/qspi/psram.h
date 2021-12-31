#ifndef ID3MP3_PSRAM_H
#define ID3MP3_PSRAM_H

#include "main.h"

/* IPUS Command Set*/
#define READ_ID_CMD                          0x9F

#define READ_CMD                             0x03
#define FAST_READ_CMD                        0x0B
#define FAST_READ_4_BYTE_ADDR_CMD            0xEB

#define WRITE_CMD                            0x02
#define WRITE_4_BYTE_ADDR_CMD                0x38

#define ENTER_QUAD_MODE_CMD                  0x35
#define EXIT_QUAD_MODE_CMD                   0xF5

#define RESET_ENABLE_CMD                     0x66
#define RESET_CMD                            0x99

#define SET_BURST_LENGTH_CMD                 0xC0

#ifdef __cplusplus
extern "C" {
#endif

void QSPI_PSRAM_Init();
void QSPI_PSRAM_ReadID();
void QSPI_PSRAM_Read(uint8_t *pData,uint32_t addr, uint32_t bufSize);
void QSPI_PSRAM_Write(uint8_t *pData,uint32_t addr, uint32_t bufSize);

#ifdef __cplusplus
}
#endif

#endif //ID3MP3_PSRAM_H

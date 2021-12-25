#include "spi.h"

uint8_t SPI1_ReadWriteByte(uint8_t data)
{
    uint8_t retry = 0;

    /* Check if Tx buffer is empty */
    while (!LL_SPI_IsActiveFlag_TXE(SPI1))
    {
        retry++;
        if (retry > 200)
            return 0;
    }

    /* Write character in Data register.
	TXE flag is cleared by reading data in DR register */
    LL_SPI_TransmitData8(SPI1, data);
    retry = 0;

    /* Check if Rx buffer is not empty */
    while (!LL_SPI_IsActiveFlag_RXNE(SPI1))
    {
        retry++;
        if (retry > 200)
            return 0;
    }

    /* received byte from SPI lines. */
    return LL_SPI_ReceiveData8(SPI1);
}

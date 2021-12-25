#include "adc.h"

uint16_t read_ADC()
{
    LL_ADC_REG_StartConversionSWStart(ADC1);
    LL_mDelay(1);
    LL_ADC_REG_StopConversionExtTrig(ADC1);
    return LL_ADC_REG_ReadConversionData12(ADC1);
}
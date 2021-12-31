#include "qspi.h"
#include "main.h"

void QSPI_Send_CMD(uint32_t instruction, uint32_t address, uint32_t dummyCycles, uint32_t instructionMode,
                   uint32_t addressMode, uint32_t addressSize, uint32_t dataMode) {
    QSPI_CommandTypeDef Cmdhandler;

    Cmdhandler.Instruction = instruction;                    //指令
    Cmdhandler.Address = address;                                //地址
    Cmdhandler.DummyCycles = dummyCycles;                     //设置空指令周期数
    Cmdhandler.InstructionMode = instructionMode;                //指令模式
    Cmdhandler.AddressMode = addressMode;                    //地址模式
    Cmdhandler.AddressSize = addressSize;                    //地址长度
    Cmdhandler.DataMode = dataMode;                            //数据模式
    Cmdhandler.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;        //每次都发送指令
    Cmdhandler.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE; //无交替字节
    Cmdhandler.DdrMode = QSPI_DDR_MODE_DISABLE;            //关闭DDR模式
    Cmdhandler.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;

    HAL_QSPI_Command(&hqspi, &Cmdhandler, 5000);
}

uint8_t QSPI_Receive(uint8_t *buf, uint32_t datalen) {
    hqspi.Instance->DLR = datalen - 1;                           //配置数据长度
    if (HAL_QSPI_Receive(&hqspi, buf, 5000) == HAL_OK) return 0;  //接收数据
    else return 1;
}

uint8_t QSPI_Transmit(uint8_t *buf, uint32_t datalen) {
    hqspi.Instance->DLR = datalen - 1;                            //配置数据长度
    if (HAL_QSPI_Transmit(&hqspi, buf, 5000) == HAL_OK) return 0;  //发送数据
    else return 1;
}
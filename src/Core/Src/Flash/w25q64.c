#include "w25q64.h"
#include "main.h"
#include "qspi.h"
#include "lvgl.h"
#include "Show/show.h"

ramfast uint16_t W25QXX_TYPE = W25Q64;    //默认是W25Q64
ramfast uint8_t W25QXX_QPI_MODE = 0;        //QSPI模式标志:0,SPI模式;1,QSPI模式.

void W25QXX_Init() {
    uint8_t temp;
    W25QXX_Qspi_Enable();            //使能QSPI模式
    W25QXX_TYPE = W25QXX_ReadID();    //读取FLASH ID.
    if (W25QXX_TYPE == W25Q128)         //SPI FLASH为W25Q64
    {
////     此处注释掉，只有当是W25Q256的时候，此处才有效检查

////			temp=W25QXX_ReadSR(3);      //读取状态寄存器3，判断地址模式
////
////			if((temp&0X01)==0)			    //如果不是4字节地址模式,则进入4字节地址模式
////			{
////				W25QXX_Write_Enable();	 //写使能
////
////				QSPI_Send_CMD(W25X_Enable4ByteAddr,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE);//QSPI,使能4字节地址指令,地址为0,无数据_8位地址_无地址_4线传输指令,无空周期,0个字节数据
////			}
        W25QXX_Write_Enable();        //写使能
        QSPI_Send_CMD(W25X_SetReadParam, 0, 0, QSPI_INSTRUCTION_4_LINES, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS,
                      QSPI_DATA_4_LINES);        //QPI,设置读参数指令,地址为0,4线传数据_8位地址_无地址_4线传输指令,无空周期,1个字节数据

        temp = 3 << 4;                    //设置P4&P5=11,8个dummy clocks,104M
        QSPI_Transmit(&temp, 1);        //发送1个字节
    }
}

void W25QXX_Qspi_Enable() {
    uint8_t stareg2;
    stareg2 = W25QXX_ReadSR(2);        //先读出状态寄存器2的原始值

    if ((stareg2 & 0X02) == 0)            //QE位未使能
    {
        W25QXX_Write_Enable();          //写使能
        stareg2 |= 1 << 1;                      //使能QE位
        W25QXX_Write_SR(2, stareg2);    //写状态寄存器2
    }

    QSPI_Send_CMD(W25X_EnterQPIMode, 0, 0, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS,
                  QSPI_DATA_NONE);//写command指令,地址为0,无数据_8位地址_无地址_单线传输指令,无空周期,0个字节数据

    W25QXX_QPI_MODE = 1;                //标记QSPI模式
}

void W25QXX_Qspi_Disable(void) {
    QSPI_Send_CMD(W25X_ExitQPIMode, 0, 0, QSPI_INSTRUCTION_4_LINES, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS,
                  QSPI_DATA_NONE);//写command指令,地址为0,无数据_8位地址_无地址_4线传输指令,无空周期,0个字节数据

    W25QXX_QPI_MODE = 0;                //标记SPI模式
}

uint8_t W25QXX_ReadSR(uint8_t regno) {
    uint8_t byte = 0, command = 0;
    switch (regno) {
        case 1:
            command = W25X_ReadStatusReg1;    //读状态寄存器1指令
            break;
        case 2:
            command = W25X_ReadStatusReg2;    //读状态寄存器2指令
            break;
        case 3:
            command = W25X_ReadStatusReg3;    //读状态寄存器3指令
            break;
        default:
            command = W25X_ReadStatusReg1;
            break;
    }
    if (W25QXX_QPI_MODE)
        QSPI_Send_CMD(command, 0, 0, QSPI_INSTRUCTION_4_LINES, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS,
                      QSPI_DATA_4_LINES);    //QPI,写command指令,地址为0,4线传数据_8位地址_无地址_4线传输指令,无空周期,1个字节数据
    else
        QSPI_Send_CMD(command, 0, 0, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS,
                      QSPI_DATA_1_LINE);                //SPI,写command指令,地址为0,单线传数据_8位地址_无地址_单线传输指令,无空周期,1个字节数据

    QSPI_Receive(&byte, 1);
    return byte;
}

void W25QXX_Write_SR(uint8_t regno, uint8_t sr) {
    uint8_t command = 0;
    switch (regno) {
        case 1:
            command = W25X_WriteStatusReg1;    //写状态寄存器1指令
            break;
        case 2:
            command = W25X_WriteStatusReg2;    //写状态寄存器2指令
            break;
        case 3:
            command = W25X_WriteStatusReg3;    //写状态寄存器3指令
            break;
        default:
            command = W25X_WriteStatusReg1;
            break;
    }
    if (W25QXX_QPI_MODE)
        QSPI_Send_CMD(command, 0, 0, QSPI_INSTRUCTION_4_LINES, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS,
                      QSPI_DATA_4_LINES);    //QPI,写command指令,地址为0,4线传数据_8位地址_无地址_4线传输指令,无空周期,1个字节数据
    else
        QSPI_Send_CMD(command, 0, 0, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS,
                      QSPI_DATA_1_LINE);        //SPI,写command指令,地址为0,单线传数据_8位地址_无地址_单线传输指令,无空周期,1个字节数据

    QSPI_Transmit(&sr, 1);
}

void W25QXX_Write_Enable() {
    if (W25QXX_QPI_MODE)
        QSPI_Send_CMD(W25X_WriteEnable, 0, 0, QSPI_INSTRUCTION_4_LINES, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS,
                      QSPI_DATA_NONE);    //QPI,写使能指令,地址为0,无数据_8位地址_无地址_4线传输指令,无空周期,0个字节数据
    else
        QSPI_Send_CMD(W25X_WriteEnable, 0, 0, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS,
                      QSPI_DATA_NONE);                //SPI,写使能指令,地址为0,无数据_8位地址_无地址_单线传输指令,无空周期,0个字节数据
}

void W25QXX_Write_Disable() {
    if (W25QXX_QPI_MODE)
        QSPI_Send_CMD(W25X_WriteDisable, 0, 0, QSPI_INSTRUCTION_4_LINES, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS,
                      QSPI_DATA_NONE); //QPI,写禁止指令,地址为0,无数据_8位地址_无地址_4线传输指令,无空周期,0个字节数据
    else
        QSPI_Send_CMD(W25X_WriteDisable, 0, 0, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS,
                      QSPI_DATA_NONE);  //SPI,写禁止指令,地址为0,无数据_8位地址_无地址_单线传输指令,无空周期,0个字节数据
}

uint16_t W25QXX_ReadID() {
    uint8_t temp[2];
    uint16_t deviceid;
    if (W25QXX_QPI_MODE)
        QSPI_Send_CMD(W25X_ManufactDeviceID, 0, 0, QSPI_INSTRUCTION_4_LINES, QSPI_ADDRESS_4_LINES, QSPI_ADDRESS_24_BITS,
                      QSPI_DATA_4_LINES);//QPI,读id,地址为0,4线传输数据_24位地址_4线传输地址_4线传输指令,无空周期,2个字节数据
    else
        QSPI_Send_CMD(W25X_ManufactDeviceID, 0, 0, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_1_LINE, QSPI_ADDRESS_24_BITS,
                      QSPI_DATA_1_LINE);            //SPI,读id,地址为0,单线传输数据_24位地址_单线传输地址_单线传输指令,无空周期,2个字节数据

    QSPI_Receive(temp, 2);
    deviceid = (temp[0] << 8) | temp[1];
    return deviceid;
}

void W25QXX_Read(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead) {
    if(ReadAddr > MAX_LENGTH) {
        info_show();
        lv_label_set_text_fmt(info, "W25Qxx read error:out of size %d,max:%d", ReadAddr, MAX_LENGTH);
        return;
    }
    QSPI_Send_CMD(W25X_FastReadData, ReadAddr, 8, QSPI_INSTRUCTION_4_LINES, QSPI_ADDRESS_4_LINES, QSPI_ADDRESS_24_BITS,
                  QSPI_DATA_4_LINES);    //QPI,快速读数据,地址为ReadAddr,4线传输数据_24位地址_4线传输地址_4线传输指令,8空周期,NumByteToRead个数据
    QSPI_Receive(pBuffer, NumByteToRead);
}

void W25QXX_Read_Utils(w25qxx_utils *head, void *pBuffer, uint16_t NumByteToRead) {
    W25QXX_Read(pBuffer, head->local + head->pos, NumByteToRead);
    head->pos += NumByteToRead;
}

void W25QXX_Write_Page(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite) {
    W25QXX_Write_Enable();                    //写使能
    QSPI_Send_CMD(W25X_PageProgram, WriteAddr, 0, QSPI_INSTRUCTION_4_LINES, QSPI_ADDRESS_4_LINES, QSPI_ADDRESS_24_BITS,
                  QSPI_DATA_4_LINES);    //QPI,页写指令,地址为WriteAddr,4线传输数据_24位地址_4线传输地址_4线传输指令,无空周期,NumByteToWrite个数据
    QSPI_Transmit(pBuffer, NumByteToWrite);
    W25QXX_Wait_Busy();                       //等待写入结束
}

void W25QXX_Write_NoCheck(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite) {
    uint16_t pageremain;

    pageremain = 256 - WriteAddr % 256; //单页剩余的字节数

    if (NumByteToWrite <= pageremain)pageremain = NumByteToWrite;//不大于256个字节

    while (1) {
        W25QXX_Write_Page(pBuffer, WriteAddr, pageremain);

        if (NumByteToWrite == pageremain)break;//写入结束了

        else //NumByteToWrite>pageremain
        {
            pBuffer += pageremain;
            WriteAddr += pageremain;

            NumByteToWrite -= pageremain;              //减去已经写入了的字节数
            if (NumByteToWrite > 256)pageremain = 256; //一次可以写入256个字节
            else pageremain = NumByteToWrite;      //不够256个字节了
        }
    }
}

ramfast uint8_t W25QXX_BUFFER[4096];

void W25QXX_Write(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite) {
    uint32_t secpos;
    uint16_t secoff;
    uint16_t secremain;
    uint16_t i;
    uint8_t *W25QXX_BUF;

    if(WriteAddr > MAX_LENGTH) {
        info_show();
        lv_label_set_text_fmt(info, "W25Qxx read error:out of size %d,max:%d", WriteAddr, MAX_LENGTH);
        return;
    }

    W25QXX_Wait_Busy();

    W25QXX_BUF = W25QXX_BUFFER;
    secpos = WriteAddr / 4096;//扇区地址
    secoff = WriteAddr % 4096;//在扇区内的偏移
    secremain = 4096 - secoff;//扇区剩余空间大小

    if (NumByteToWrite <= secremain)secremain = NumByteToWrite;//不大于4096个字节

    while (1) {
        W25QXX_Read(W25QXX_BUF, secpos * 4096, 4096);//读出整个扇区的内容
        for (i = 0; i < secremain; i++)//校验数据
        {
            if (W25QXX_BUF[secoff + i] != 0XFF)break;//需要擦除
        }
        if (i < secremain)//需要擦除
        {
            W25QXX_Erase_Sector(secpos);//擦除这个扇区
            for (i = 0; i < secremain; i++)       //复制
            {
                W25QXX_BUF[i + secoff] = pBuffer[i];
            }
            W25QXX_Write_NoCheck(W25QXX_BUF, secpos * 4096, 4096);//写入整个扇区

        } else W25QXX_Write_NoCheck(pBuffer, WriteAddr, secremain);//写已经擦除了的,直接写入扇区剩余区间.

        if (NumByteToWrite == secremain)break;//写入结束了
        else//写入未结束
        {
            secpos++;//扇区地址增1
            secoff = 0;//偏移位置为0

            pBuffer += secremain;  //指针偏移
            WriteAddr += secremain;//写地址偏移
            NumByteToWrite -= secremain;                //字节数递减

            if (NumByteToWrite > 4096)secremain = 4096;    //下一个扇区还是写不完
            else secremain = NumByteToWrite;            //下一个扇区可以写完了
        }
    }
}

void W25QXX_Erase_Chip() {
    W25QXX_Write_Enable();                    //SET WEL
    W25QXX_Wait_Busy();
    QSPI_Send_CMD(W25X_ChipErase, 0, 0, QSPI_INSTRUCTION_4_LINES, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS,
                  QSPI_DATA_NONE);//QPI,写全片擦除指令,地址为0,无数据_8位地址_无地址_4线传输指令,无空周期,0个字节数据
    W25QXX_Wait_Busy();                        //等待芯片擦除结束
}

void W25QXX_Erase_Sector(uint32_t Dst_Addr) {

    Dst_Addr *= 4096;
    W25QXX_Write_Enable();                  //SET WEL
    W25QXX_Wait_Busy();
    QSPI_Send_CMD(W25X_SectorErase, Dst_Addr, 0, QSPI_INSTRUCTION_4_LINES, QSPI_ADDRESS_4_LINES, QSPI_ADDRESS_24_BITS,
                  QSPI_DATA_NONE);//QPI,写扇区擦除指令,地址为0,无数据_24位地址_4线传输地址_4线传输指令,无空周期,0个字节数据
    W25QXX_Wait_Busy();                    //等待擦除完成
}

void W25QXX_Wait_Busy() {
    while ((W25QXX_ReadSR(1) & 0x01) == 0x01);// 等待BUSY位清空
}
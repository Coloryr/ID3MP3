#ifndef VS1053_H
#define VS1053_H
#include "main.h"
#include "../bit.h"
#include "../spi/spi.h"

//移植时候的接口
//data:要写入的数据
//返回值:读到的数据
uint8_t VS_SPI_ReadWriteByte(uint8_t data);

//////////////////////////////////////////////////////////////
//与外部的接口
#define VS_DQ PAin(3)	 //DREQ
#define VS_RST PAout(2)	 //RST
#define VS_XCS PAout(1)	 //XCS
#define VS_XDCS PAout(0) //XDCS

typedef struct vs10xx_obj
{
	uint8_t mvol;	 //主音量,范围:0~254
	uint8_t bflimit; //低音频率限定,范围:2~15(单位:10Hz)
	uint8_t bass;	 //低音,范围:0~15.0表示关闭.(单位:1dB)
	uint8_t tflimit; //高音频率限定,范围:1~15(单位:Khz)
	uint8_t treble;	 //高音,范围:0~15(单位:1.5dB)(原本范围是:-8~7,通过函数修改了);
	uint8_t effect;	 //空间效果设置.0,关闭;1,最小;2,中等;3,最大.

} _vs10xx_obj;

extern _vs10xx_obj vsset; //VS10XX设置

#define VS_WRITE_COMMAND 0x02
#define VS_READ_COMMAND 0x03
//VS10XX寄存器定义
#define SPI_MODE 0x00
#define SPI_STATUS 0x01
#define SPI_BASS 0x02
#define SPI_CLOCKF 0x03
#define SPI_DECODE_TIME 0x04
#define SPI_AUDATA 0x05
#define SPI_WRAM 0x06
#define SPI_WRAMADDR 0x07
#define SPI_HDAT0 0x08
#define SPI_HDAT1 0x09

#define SPI_AIADDR 0x0a
#define SPI_VOL 0x0b
#define SPI_AICTRL0 0x0c
#define SPI_AICTRL1 0x0d
#define SPI_AICTRL2 0x0e
#define SPI_AICTRL3 0x0f
#define SM_DIFF 0x01
#define SM_JUMP 0x02
#define SM_RESET 0x04
#define SM_OUTOFWAV 0x08
#define SM_PDOWN 0x10
#define SM_TESTS 0x20
#define SM_STREAM 0x40
#define SM_PLUSV 0x80
#define SM_DACT 0x100
#define SM_SDIORD 0x200
#define SM_SDISHARE 0x400
#define SM_SDINEW 0x800
#define SM_ADPCM 0x1000
#define SM_ADPCM_HP 0x2000

uint16_t VS_RD_Reg(uint8_t address);			//读寄存器
uint16_t VS_WRAM_Read(uint16_t addr);			//读RAM
void VS_WR_Data(uint8_t data);					//写数据
void VS_WR_Cmd(uint8_t address, uint16_t data); //写命令
uint8_t VS_HD_Reset(void);						//硬复位
void VS_Soft_Reset(void);						//软复位
uint16_t VS_Ram_Test(void);						//RAM测试
void VS_Sine_Test(void);						//正弦测试

void VS_SPI_SpeedLow(void);
void VS_SPI_SpeedHigh(void);
void VS_Set_Speed(uint8_t t);			 //设置播放速度
uint16_t VS_Get_HeadInfo(void);			 //得到比特率
uint32_t VS_Get_ByteRate(void);			 //得到字节速率
uint16_t VS_Get_EndFillByte(void);		 //得到填充字节
uint8_t VS_Send_MusicData(uint8_t *buf); //向VS10XX发送32字节
void VS_Restart_Play(void);				 //重新开始下一首歌播放
void VS_Reset_DecodeTime(void);			 //重设解码时间
uint16_t VS_Get_DecodeTime(void);		 //得到解码时间

void VS_Load_Patch(uint16_t *patch, uint16_t len);							  //加载用户patch
uint8_t VS_Get_Spec(uint16_t *p);											  //得到分析数据
void VS_Set_Bands(uint16_t *buf, uint8_t bands);							  //设置中心频率
void VS_Set_Vol(uint8_t volx);												  //设置主音量
void VS_Set_Bass(uint8_t bfreq, uint8_t bass, uint8_t tfreq, uint8_t treble); //设置高低音
void VS_Set_Effect(uint8_t eft);											  //设置音效
void VS_Set_All(void);

void vs10xx_read_para(_vs10xx_obj *vs10xxdev);
void vs10xx_save_para(_vs10xx_obj *vs10xxdev);

void VS1053_Init(void);

#endif

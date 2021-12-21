#ifndef __GIF_H__
#define __GIF_H__

#include "main.h"
#include "fatfs.h"

//////////////////////////////////////////用户配置区//////////////////////////////////
#define GIF_USE_MALLOC        1    //定义是否使用malloc,这里我们选择使用malloc
//////////////////////////////////////////////END/////////////////////////////////////


#define LCD_MAX_LOG_COLORS  256
#define MAX_NUM_LWZ_BITS    12


#define GIF_INTRO_TERMINATOR ';'    //0X3B   GIF文件结束符
#define GIF_INTRO_EXTENSION  '!'    //0X21
#define GIF_INTRO_IMAGE      ','    //0X2C

#define GIF_COMMENT        0xFE
#define GIF_APPLICATION    0xFF
#define GIF_PLAINTEXT    0x01
#define GIF_GRAPHICCTL    0xF9

typedef struct {
    uint8_t aBuffer[258];                     // Input buffer for data block
    short aCode[(1 << MAX_NUM_LWZ_BITS)]; // This array stores the LZW codes for the compressed strings
    uint8_t aPrefix[(1 << MAX_NUM_LWZ_BITS)]; // Prefix character of the LZW code.
    uint8_t aDecompBuffer[3000];              // Decompression buffer. The higher the compression, the more bytes are needed in the buffer.
    uint8_t *sp;                               // Pointer into the decompression buffer
    int CurBit;
    int LastBit;
    int GetDone;
    int LastByte;
    int ReturnClear;
    int CodeSize;
    int SetCodeSize;
    int MaxCode;
    int MaxCodeSize;
    int ClearCode;
    int EndCode;
    int FirstCode;
    int OldCode;
} LZW_INFO;

//逻辑屏幕描述块
typedef struct {
    uint16_t width;        //GIF宽度
    uint16_t height;        //GIF高度
    uint8_t flag;        //标识符  1:3:1:3=全局颜色表标志(1):颜色深度(3):分类标志(1):全局颜色表大小(3)
    uint8_t bkcindex;    //背景色在全局颜色表中的索引(仅当存在全局颜色表时有效)
    uint8_t pixratio;    //像素宽高比
} LogicalScreenDescriptor;


//图像描述块
typedef struct {
    uint16_t xoff;        //x方向偏移
    uint16_t yoff;        //y方向偏移
    uint16_t width;        //宽度
    uint16_t height;        //高度
    uint8_t flag;        //标识符  1:1:1:2:3=局部颜色表标志(1):交织标志(1):保留(2):局部颜色表大小(3)
} ImageScreenDescriptor;

//图像描述
typedef struct {
    LogicalScreenDescriptor gifLSD;    //逻辑屏幕描述块
    ImageScreenDescriptor gifISD;    //图像描述快
    uint16_t colortbl[256];                //当前使用颜色表
    uint16_t bkpcolortbl[256];            //备份颜色表.当存在局部颜色表时使用
    uint16_t numcolors;                    //颜色表大小
    uint16_t delay;                        //延迟时间
    LZW_INFO *lzw;                    //LZW信息
} gif89a;

extern uint8_t gifdecoding;    //GIF正在解码标记.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t gif_check_head(FIL *file);                                                            //检测GIF头
uint16_t gif_getrgb565(uint8_t *ctb);                                                                //将RGB888转为RGB565
uint8_t gif_readcolortbl(FIL *file, gif89a *gif, uint16_t num);                                    //读取颜色表
uint8_t gif_getinfo(FIL *file, gif89a *gif);                                                    //得到逻辑屏幕描述,图像尺寸等
void gif_savegctbl(gif89a *gif);                                                        //保存全局颜色表
void gif_recovergctbl(gif89a *gif);                                                        //恢复全局颜色表
void gif_initlzw(gif89a *gif, uint8_t codesize);                                                //初始化LZW相关参数
uint16_t gif_getdatablock(FIL *gfile, uint8_t *buf, uint16_t maxnum);                                    //读取一个数据块
uint8_t gif_readextension(FIL *gfile, gif89a *gif, int *pTransIndex, uint8_t *pDisposal);            //读取扩展部分
int gif_getnextcode(FIL *gfile, gif89a *gif);                                            //从LZW缓存中得到下一个LZW码,每个码包含12位
int gif_getnextbyte(FIL *gfile, gif89a *gif);                                            //得到LZW的下一个码
uint8_t gif_dispimage(FIL *gfile, gif89a *gif, uint16_t x0, uint16_t y0, int Transparency, uint8_t Disposal);    //显示图片
void gif_clear2bkcolor(uint16_t x, uint16_t y, gif89a *gif, ImageScreenDescriptor pimge);            //恢复成背景色
uint8_t gif_drawimage(FIL *gfile, gif89a *gif, uint16_t x0, uint16_t y0);                                    //画GIF图像的一帧

uint8_t gif_decode(const uint8_t *filename, uint16_t x, uint16_t y, uint16_t width, uint16_t height);//在指定区域解码一个GIF文件.
void gif_quit(void);                                    //退出当前解码.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

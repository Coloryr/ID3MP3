#include "font.h"
#include "fatfs.h"
#include "string.h"
#include "malloc.h"
#include "../lcd/lcd.h"
#include "../flash/flash.h"

#define FONTINFOADDR (1024 * 25) * 1024 //默认是24M的地址
//字库信息结构体.
//用来保存字库基本信息，地址，大小等
_font_info ftinfo;

//在sd卡中的路径
#define GBK32_SDPATH "0:/SYSTEM/FONT/GBK32.FON"   //GBK32的存放位置
#define GBK24_SDPATH "0:/SYSTEM/FONT/GBK24.FON"   //GBK24的存放位置
#define GBK16_SDPATH "0:/SYSTEM/FONT/GBK16.FON"   //GBK16的存放位置
#define GBK12_SDPATH "0:/SYSTEM/FONT/GBK12.FON"   //GBK12的存放位置
#define UNIGBK_SDPATH "0:/SYSTEM/FONT/UNIGBK.BIN" //UNIGBK.BIN的存放位置

//显示当前字体更新进度
//x,y:坐标
//size:字体大小
//fsize:整个文件大小
//pos:当前文件指针位置
uint32_t fupd_prog(uint16_t x, uint16_t y, uint8_t size, uint32_t fsize, uint32_t pos)
{
    float prog;
    uint8_t t = 0XFF;
    prog = (float)pos / fsize;
    prog *= 100;
    if (t != prog)
    {
        LCD_ShowString(x + 3 * size / 2, y, 240, 320, size, "%");
        t = prog;
        if (t > 100)
            t = 100;
        LCD_ShowNum(x, y, t, 3, size); //显示数值
    }
    return 0;
}
//更新某一个
//x,y:坐标
//size:字体大小
//fxpath:路径
//fx:更新的内容 0,ungbk;1,gbk12;2,gbk16;
//返回值:0,成功;其他,失败.
uint8_t updata_fontx(uint16_t x, uint16_t y, uint8_t size, uint8_t *fxpath, uint8_t fx)
{
    uint32_t flashaddr = 0;
    FIL *fftemp;
    uint8_t *tempbuf;
    uint8_t res;
    uint16_t bread;
    uint32_t offx = 0;
    uint8_t rval = 0;
    fftemp = (FIL *)malloc(sizeof(FIL)); //分配内存
    if (fftemp == NULL)
        rval = 1;
    tempbuf = malloc(4096); //分配4096个字节空间
    if (tempbuf == NULL)
        rval = 1;
    res = f_open(fftemp, (const TCHAR *)fxpath, FA_READ);
    if (res)
        rval = 2; //打开文件失败
    if (rval == 0)
    {
        if (fx == 0) //更新UNIGBK.BIN
        {
            ftinfo.ugbkaddr = sizeof(ftinfo) + FONTINFOADDR; //信息头之后，紧跟UNIGBK转换码表
            ftinfo.ugbksize = fftemp->fsize;                 //UNIGBK大小
            flashaddr = ftinfo.ugbkaddr;
        }
        else if (fx == 1) //GBK12
        {
            ftinfo.f12addr = ftinfo.ugbkaddr + ftinfo.ugbksize; //UNIGBK之后，紧跟GBK12字库
            ftinfo.gbk12size = fftemp->fsize;                   //GBK12字库大小
            flashaddr = ftinfo.f12addr;                         //GBK12的起始地址
        }
        else if (fx == 2) //GBK16
        {
            ftinfo.f16addr = ftinfo.f12addr + ftinfo.gbk12size; //GBK12之后，紧跟GBK16字库
            ftinfo.gbk16size = fftemp->fsize;                   //GBK16字库大小
            flashaddr = ftinfo.f16addr;                         //GBK16的起始地址
        }
        else if (fx == 3) //GBK24
        {
            ftinfo.f24addr = ftinfo.f16addr + ftinfo.gbk16size; //GBK12之后，紧跟GBK16字库
            ftinfo.gbk24size = fftemp->fsize;                   //GBK16字库大小
            flashaddr = ftinfo.f24addr;                         //GBK16的起始地址
        }
        else if (fx == 4) //GBK24
        {
            ftinfo.f32addr = ftinfo.f24addr + ftinfo.gbk24size; //GBK12之后，紧跟GBK16字库
            ftinfo.gbk32size = fftemp->fsize;                   //GBK16字库大小
            flashaddr = ftinfo.f32addr;                         //GBK16的起始地址
        }
        while (res == FR_OK) //死循环执行
        {
            res = f_read(fftemp, tempbuf, 4096, (UINT *)&bread); //读取数据
            if (res != FR_OK)
                break;                                        //执行错误
            SPI_Flash_Write(tempbuf, offx + flashaddr, 4096); //从0开始写入4096个数据
            offx += bread;
            fupd_prog(x, y, size, fftemp->fsize, offx); //进度显示
            if (bread != 4096)
                break; //读完了.
        }
        f_close(fftemp);
    }
    free(fftemp);  //释放内存
    free(tempbuf); //释放内存
    return res;
}

//更新字体文件,UNIGBK,GBK12,GBK16一起更新
//x,y:提示信息的显示地址
//size:字体大小
//提示信息字体大小
//src:0,从SD卡更新.
//	  1,从25QXX更新
//返回值:0,更新成功;
//		 其他,错误代码.
uint8_t update_font(uint16_t x, uint16_t y, uint8_t size)
{
    uint8_t *gbk32_path;
    uint8_t *gbk24_path;
    uint8_t *gbk16_path;
    uint8_t *gbk12_path;
    uint8_t *unigbk_path;
    uint8_t res;
    unigbk_path = (uint8_t *)UNIGBK_SDPATH;
    gbk12_path = (uint8_t *)GBK12_SDPATH;
    gbk16_path = (uint8_t *)GBK16_SDPATH;
    gbk24_path = (uint8_t *)GBK24_SDPATH;
    gbk32_path = (uint8_t *)GBK32_SDPATH;
    res = 0XFF;
    ftinfo.fontok = 0XFF;
    LCD_ShowString(x, y, 240, 320, size, "FLASH Erasing.......");
    SPI_Flash_Erase_Chip();
    SPI_Flash_Write((uint8_t *)&ftinfo, FONTINFOADDR, sizeof(ftinfo)); //清除之前字库成功的标志.防止更新到一半重启,导致的字库部分数据丢失.
    SPI_Flash_Read((uint8_t *)&ftinfo, FONTINFOADDR, sizeof(ftinfo));  //重新读出ftinfo结构体数据
    LCD_ShowString(x, y, 240, 320, size, "Updating UNIGBK.BIN");
    res = updata_fontx(x + 20 * size / 2, y, size, unigbk_path, 0); //更新UNIGBK.BIN
    if (res)
        return 1;
    LCD_ShowString(x, y, 240, 320, size, "Updating GBK12.BIN  ");
    res = updata_fontx(x + 20 * size / 2, y, size, gbk12_path, 1); //更新GBK12.FON
    if (res)
        return 2;
    LCD_ShowString(x, y, 240, 320, size, "Updating GBK16.BIN  ");
    res = updata_fontx(x + 20 * size / 2, y, size, gbk16_path, 2); //更新GBK16.FON
    if (res)
        return 3;
    LCD_ShowString(x, y, 240, 320, size, "Updating GBK24.BIN  ");
    res = updata_fontx(x + 20 * size / 2, y, size, gbk24_path, 3); //更新GBK12.FON
    if (res)
        return 4;
    LCD_ShowString(x, y, 240, 320, size, "Updating GBK32.BIN  ");
    res = updata_fontx(x + 20 * size / 2, y, size, gbk32_path, 4); //更新GBK16.FON
    if (res)
        return 5;
    //全部更新好了
    ftinfo.fontok = 0XAA;
    SPI_Flash_Write((uint8_t *)&ftinfo, FONTINFOADDR, sizeof(ftinfo)); //保存字库信息
    return 0;                                                          //无错误.
}
//初始化字体
//返回值:0,字库完好.
//		 其他,字库丢失
uint8_t font_init(void)
{
    ftinfo.ugbkaddr = FONTINFOADDR + 41;                              //UNICODEGBK 表存放首地址固定地址
    SPI_Flash_Read((uint8_t *)&ftinfo, FONTINFOADDR, sizeof(ftinfo)); //读出ftinfo结构体数据
    if (ftinfo.fontok != 0XAA)
        return 1; //字库错误.
    return 0;
}

#include "piclib.h"
#include "lcd.h"
#include "stm32_adafruit_lcd.h"
#include "fatfs.h"
#include "exfuns.h"
#include "tjpgd.h"
#include "gif.h"

ramfast _pic_info picinfo;        //图片信息
ramfast _pic_phy pic_phy;        //图片显示物理接口
//////////////////////////////////////////////////////////////////////////
//lcd.h没有提供划横线函数,需要自己实现
void piclib_draw_hline(uint16_t x0, uint16_t y0, uint16_t len, uint16_t color) {
    if ((len == 0) || (x0 > lcd_drv->getLcdPixelWidth()) || (y0 > lcd_drv->getLcdPixelHeight()))return;
    lcd_drv->FillRect(x0, y0, x0 + len - 1, y0, color);
}

//填充颜色
//x,y:起始坐标
//width，height：宽度和高度。
//*color：颜色数组
void piclib_fill_color(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t *color) {
    lcd_drv->DrawRGBImage(x, y, x + width - 1, y + height - 1, color);//其他情况,直接填充
}

//////////////////////////////////////////////////////////////////////////
//画图初始化,在画图之前,必须先调用此函数
//指定画点/读点
void piclib_init() {
    pic_phy.read_point = lcd_drv->readPixel;        //读点函数实现,仅BMP需要
    pic_phy.draw_point = lcd_drv->WritePixel;    //画点函数实现
    pic_phy.fill = lcd_drv->FillRect;                    //填充函数实现,仅GIF需要
    pic_phy.draw_hline = piclib_draw_hline;    //画线函数实现,仅GIF需要
    pic_phy.fillcolor = piclib_fill_color;    //颜色填充函数实现,仅TJPGD需要

    picinfo.lcdwidth = lcd_drv->getLcdPixelHeight();    //得到LCD的宽度像素
    picinfo.lcdheight = lcd_drv->getLcdPixelHeight();//得到LCD的高度像素

    picinfo.ImgWidth = 0;    //初始化宽度为0
    picinfo.ImgHeight = 0;//初始化高度为0
    picinfo.Div_Fac = 0;    //初始化缩放系数为0
    picinfo.S_Height = 0;    //初始化设定的高度为0
    picinfo.S_Width = 0;    //初始化设定的宽度为0
    picinfo.S_XOFF = 0;    //初始化x轴的偏移量为0
    picinfo.S_YOFF = 0;    //初始化y轴的偏移量为0
    picinfo.staticx = 0;    //初始化当前显示到的x坐标为0
    picinfo.staticy = 0;    //初始化当前显示到的y坐标为0
}

//快速ALPHA BLENDING算法.
//src:源颜色
//dst:目标颜色
//alpha:透明程度(0~32)
//返回值:混合后的颜色.
uint16_t piclib_alpha_blend(uint16_t src, uint16_t dst, uint8_t alpha) {
    uint32_t src2;
    uint32_t dst2;
    //Convert to 32bit |-----GGGGGG-----RRRRR------BBBBB|
    src2 = ((src << 16) | src) & 0x07E0F81F;
    dst2 = ((dst << 16) | dst) & 0x07E0F81F;
    //Perform blending R:G:B with alpha in range 0..32
    //Note that the reason that alpha may not exceed 32 is that there are only
    //5bits of space between each R:G:B value, any higher value will overflow
    //into the next component and deliver ugly result.
    dst2 = ((((dst2 - src2) * alpha) >> 5) + src2) & 0x07E0F81F;
    return (dst2 >> 16) | dst2;
}

//初始化智能画点
//内部调用
void ai_draw_init() {
    float temp, temp1;
    temp = (float) picinfo.S_Width / picinfo.ImgWidth;
    temp1 = (float) picinfo.S_Height / picinfo.ImgHeight;
    if (temp < temp1)temp1 = temp;//取较小的那个
    if (temp1 > 1)temp1 = 1;
    //使图片处于所给区域的中间
    picinfo.S_XOFF += (picinfo.S_Width - temp1 * picinfo.ImgWidth) / 2;
    picinfo.S_YOFF += (picinfo.S_Height - temp1 * picinfo.ImgHeight) / 2;
    temp1 *= 8192;//扩大8192倍
    picinfo.Div_Fac = temp1;
    picinfo.staticx = 0xffff;
    picinfo.staticy = 0xffff;//放到一个不可能的值上面
}

//判断这个像素是否可以显示
//(x,y) :像素原始坐标
//chg   :功能变量.
//返回值:0,不需要显示.1,需要显示
uint8_t is_element_ok(uint16_t x, uint16_t y, uint8_t chg) {
    if (x != picinfo.staticx || y != picinfo.staticy) {
        if (chg == 1) {
            picinfo.staticx = x;
            picinfo.staticy = y;
        }
        return 1;
    } else return 0;
}


//智能画图
//FileName:要显示的图片文件  BMP/JPG/JPEG/GIF
//x,y,width,height:坐标及显示区域尺寸
//fast:使能jpeg/jpg小图片(图片尺寸小于等于液晶分辨率时才启用)快速解码,0,不使能;1,使能.
//     当有硬件JPEG解码的时候,快速解码使用硬件jpeg解码,以提高速度
//图片在开始和结束的坐标点范围内显示
uint8_t ai_load_picfile(const uint8_t *filename, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t fast) {
    uint8_t res;//返回值
    uint8_t temp;
    if ((x + width) > picinfo.lcdwidth)return PIC_WINDOW_ERR;        //x坐标超范围了.
    if ((y + height) > picinfo.lcdheight)return PIC_WINDOW_ERR;        //y坐标超范围了.
    //得到显示方框大小
    if (width == 0 || height == 0)return PIC_WINDOW_ERR;    //窗口设定错误
    picinfo.S_Height = height;
    picinfo.S_Width = width;
    //显示区域无效
    if (picinfo.S_Height == 0 || picinfo.S_Width == 0) {
        picinfo.S_Height = lcd_drv->getLcdPixelHeight();
        picinfo.S_Width = lcd_drv->getLcdPixelWidth();
        return FALSE;
    }
    if (pic_phy.fillcolor == NULL)fast = 0;//颜色填充函数未实现,不能快速显示
    //显示的开始坐标点
    picinfo.S_YOFF = y;
    picinfo.S_XOFF = x;
    //文件名传递
    temp = f_typetell((uint8_t *) filename);    //得到文件的类型
    switch (temp) {
        case T_BMP:
            res = stdbmp_decode(filename);                //解码bmp
            break;
        case T_JPG:
        case T_JPEG:
            if (fast)                                    //可能需要硬件解码
            {
                res = jpg_get_size(filename, &picinfo.ImgWidth, &picinfo.ImgHeight);
                if (res == 0) {
                    res = jpg_decode(filename, fast);//采用软件解码JPG/JPEG
                }
            } else res = jpg_decode(filename, fast);        //统一采用软件解码JPG/JPEG
            break;
        case T_GIF:
            res = gif_decode(filename, x, y, width, height);    //解码gif
            break;
        default:
            res = PIC_FORMAT_ERR;                        //非图片格式!!!
            break;
    }
    return res;
}

//动态分配内存
void *pic_memalloc(uint32_t size) {
    return (void *) malloc(size);
}

//释放内存
void pic_memfree(void *mf) {
    free(mf);
}

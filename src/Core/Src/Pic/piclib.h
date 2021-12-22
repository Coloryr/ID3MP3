#ifndef __PICLIB_H
#define __PICLIB_H

#include "main.h"
#include "lcd.h"
#include "malloc.h"
#include "ff.h"
#include "bmp.h"

#define PIC_FORMAT_ERR        0x27    //格式错误
#define PIC_SIZE_ERR        0x28    //图片尺寸错误
#define PIC_WINDOW_ERR        0x29    //窗口设定错误
#define PIC_MEM_ERR            0x11    //内存错误
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef TRUE
#define TRUE    1
#endif
#ifndef FALSE
#define FALSE   0
#endif

//图片显示物理层接口
//在移植的时候,必须由用户自己实现这几个函数
typedef struct {
    uint32_t (*read_point)(uint16_t, uint16_t);                //u32 read_point(u16 x,u16 y)						读点函数
    void (*draw_point)(uint16_t, uint16_t, uint32_t);            //void draw_point(u16 x,u16 y,u32 color)		    画点函数
    void (*fill)(uint16_t, uint16_t, uint16_t, uint16_t,
                 uint32_t);        ///void fill(u16 sx,u16 sy,u16 ex,u16 ey,u32 color) 单色填充函数
    void (*draw_hline)(uint16_t, uint16_t, uint16_t,
                       uint16_t);        //void draw_hline(u16 x0,u16 y0,u16 len,u16 color)  画水平线函数
    void (*fillcolor)(uint16_t, uint16_t, uint16_t, uint16_t,
                      uint16_t *);    //void piclib_fill_color(u16 x,u16 y,u16 width,u16 height,u16 *color) 颜色填充
} _pic_phy;

extern _pic_phy pic_phy;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//图像信息
typedef struct {
    uint16_t lcdwidth;    //LCD的宽度
    uint16_t lcdheight;    //LCD的高度
    uint32_t ImgWidth;    //图像的实际宽度和高度
    uint32_t ImgHeight;

    uint32_t Div_Fac;    //缩放系数 (扩大了8192倍的)

    uint32_t S_Height;    //设定的高度和宽度
    uint32_t S_Width;

    uint32_t S_XOFF;        //x轴和y轴的偏移量
    uint32_t S_YOFF;

    uint32_t staticx;    //当前显示到的ｘｙ坐标
    uint32_t staticy;
} _pic_info;
extern _pic_info picinfo;//图像信息

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void piclib_fill_color(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t *color);

void piclib_init(void);                                //初始化画图
uint16_t piclib_alpha_blend(uint16_t src, uint16_t dst, uint8_t alpha);    //alphablend处理
void ai_draw_init(void);                            //初始化智能画图
uint8_t is_element_ok(uint16_t x, uint16_t y, uint8_t chg);                //判断像素是否有效
uint8_t ai_load_picfile(const uint8_t *filename, uint16_t x, uint16_t y, uint16_t width, uint16_t height);//智能画图
void *pic_memalloc(uint32_t size);    //pic申请内存
void pic_memfree(void *mf);    //pic释放内存

#ifdef __cplusplus
}
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

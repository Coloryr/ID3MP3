#include "jpg.h"
#include "piclib.h"
#include "extra/libs/sjpg/tjpgd.h"

ramfast FIL *f_jpeg;            //JPEG文件指针
ramfast JDEC *jpeg_dev;        //待解码对象结构体指针
ramfast uint8_t *jpg_buffer;        //定义jpeg解码工作区大小(最少需要3092字节)，作为解压缓冲区，必须4字节对齐

//给占内存大的数组/结构体申请内存
uint8_t jpeg_mallocall(void) {
    f_jpeg = (FIL *) pic_memalloc(sizeof(FIL));
    if (f_jpeg == NULL)return PIC_MEM_ERR;            //申请内存失败.
    jpeg_dev = (JDEC *) pic_memalloc(sizeof(JDEC));
    if (jpeg_dev == NULL)return PIC_MEM_ERR;        //申请内存失败.
    jpg_buffer = (uint8_t *) pic_memalloc(JPEG_WBUF_SIZE);
    if (jpg_buffer == NULL)return PIC_MEM_ERR;        //申请内存失败.
    return 0;
}

//释放内存
void jpeg_freeall(void) {
    pic_memfree(f_jpeg);        //释放f_jpeg申请到的内存
    pic_memfree(jpeg_dev);        //释放jpeg_dev申请到的内存
    pic_memfree(jpg_buffer);    //释放jpg_buffer申请到的内存
}

//jpeg数据输入回调函数
//jd:储存待解码的对象信息的结构体
//buf:输入数据缓冲区 (NULL:执行地址偏移)
//num:需要从输入数据流读出的数据量/地址偏移量
//返回值:读取到的字节数/地址偏移量
UINT jpeg_in_func(JDEC *jd, uint8_t *buf, UINT num) {
    UINT rb; //读取到的字节数
    FIL *dev = (FIL *) jd->device;  //待解码的文件的信息，使用FATFS中的FIL结构类型进行定义
    if (buf)                    //读取数据有效，开始读取数据
    {
        f_read(dev, buf, num, &rb);//调用FATFS的f_read函数，用于把jpeg文件的数据读取出来
        return rb;                //返回读取到的字节数目
    } else return (f_lseek(dev, f_tell(dev) + num) == FR_OK) ? num : 0;//重新定位数据点，相当于删除之前的n字节数据
}

//采用填充的方式进行图片解码显示
//jd:储存待解码的对象信息的结构体
//rgbbuf:指向等待输出的RGB位图数据的指针
//rect:等待输出的矩形图像的参数
//返回值:1,输出成功;
int jpeg_out_func_fill(JDEC *jd, void *rgbbuf, JRECT *rect) {
    uint8_t *s = (uint8_t *) rgbbuf;
    uint16_t w, *d = (uint16_t *) s;
    uint16_t width = rect->right - rect->left + 1;        //填充的宽度
    uint16_t height = rect->bottom - rect->top + 1;    //填充的高度
    unsigned int n = width * height;
    do {
        w = (*s++ & 0xF8) << 8;        /* RRRRR----------- */
        w |= (*s++ & 0xFC) << 3;    /* -----GGGGGG----- */
        w |= *s++ >> 3;                /* -----------BBBBB */
        *d++ = w;
    } while (--n);
    pic_phy.fillcolor(rect->left + picinfo.S_XOFF, rect->top + picinfo.S_YOFF, width, height,
                      (uint16_t *) rgbbuf);//颜色填充
    return 1;    //返回1,使得解码工作继续执行
}

//采用画点的方式进行图片解码显示
//jd:储存待解码的对象信息的结构体
//rgbbuf:指向等待输出的RGB位图数据的指针
//rect:等待输出的矩形图像的参数
//返回值:0,输出成功;1,输出失败/结束输出
int jpeg_out_func_point(JDEC *jd, void *rgbbuf, JRECT *rect) {
    uint16_t i, j;
    uint16_t realx = rect->left, realy = 0;
    uint16_t width = rect->right - rect->left + 1;        //图片的宽度
    uint16_t height = rect->bottom - rect->top + 1;    //图片的高度

    uint8_t *s = (uint8_t *) rgbbuf;
    uint16_t w, *d = (uint16_t *) s;
    unsigned int n = width * height;
    do {
        w = (*s++ & 0xF8) << 8;        /* RRRRR----------- */
        w |= (*s++ & 0xFC) << 3;    /* -----GGGGGG----- */
        w |= *s++ >> 3;                /* -----------BBBBB */
        *d++ = w;
    } while (--n);
    uint16_t *color = (uint16_t *) rgbbuf;
    for (i = 0; i < height; i++)//y坐标
    {
        realy = (picinfo.Div_Fac * (rect->top + i)) >> 13;//实际Y坐标
        //在这里不改变picinfo.staticx和picinfo.staticy的值 ,如果在这里改变,则会造成每块的第一个点不显示!!!
        if (!is_element_ok(realx, realy, 0))//行值是否满足条件? 寻找满足条件的行
        {
            color += width;
            continue;
        }
        for (j = 0; j < width; j++)//x坐标
        {
            realx = (picinfo.Div_Fac * (rect->left + j)) >> 13;//实际X坐标
            //在这里改变picinfo.staticx和picinfo.staticy的值
            if (!is_element_ok(realx, realy, 1))//列值是否满足条件? 寻找满足条件的列
            {
                color++;
                continue;
            }
            pic_phy.draw_point(realx + picinfo.S_XOFF, realy + picinfo.S_YOFF, *color);//显示图片
            color++;
        }
    }
    return 1;    //返回0,使得解码工作继续执行
}

//获取jpeg/jpg图片的宽度和高度
//width,height:图片的宽度和高度
//返回值:0,成功
//    其他,失败
uint8_t jpg_get_size(const uint8_t *filename, uint32_t *width, uint32_t *height) {
    uint8_t res = JDR_OK;    //返回值
    res = jpeg_mallocall();
    if (res == 0) {
        //得到JPEG/JPG图片的开始信息
        res = f_open(f_jpeg, (const TCHAR *) filename, FA_READ);//打开文件
        if (res == FR_OK)//打开文件成功
        {
            res = jd_prepare(jpeg_dev, jpeg_in_func, jpg_buffer, JPEG_WBUF_SIZE,
                             f_jpeg);//执行解码的准备工作，调用TjpgDec模块的jd_prepare函数
            if (res == JDR_OK)//准备解码成功,即获取到了图片的宽度和高度.返回正确的宽度和高度
            {
                *width = jpeg_dev->width;
                *height = jpeg_dev->height;
            }
        }
        f_close(f_jpeg); //解码工作执行成功，返回0
    }
    jpeg_freeall();        //释放内存
    return res;
}

//解码jpeg/jpg文件s
//filename:jpeg/jpg路径+文件名
//返回值:0,解码成功;其他,解码失败.
uint8_t jpg_decode(const uint8_t *filename) {
    uint8_t res = 0;    //返回值
    uint8_t scale;    //图像输出比例 0,1/2,1/4,1/8
    int (*outfun)(JDEC *, void *, JRECT *);

    res = jpeg_mallocall();
    if (res == 0) {
        //得到JPEG/JPG图片的开始信息
        res = f_open(f_jpeg, (const TCHAR *) filename, FA_READ);//打开文件
        if (res == FR_OK)//打开文件成功
        {
            res = jd_prepare(jpeg_dev, jpeg_in_func, jpg_buffer, JPEG_WBUF_SIZE,
                             f_jpeg);//执行解码的准备工作，调用TjpgDec模块的jd_prepare函数
            outfun = jpeg_out_func_point;//默认采用画点的方式显示
            if (res == JDR_OK)//准备解码成功
            {
                for (scale = 0; scale < 4; scale++)//确定输出图像的比例因子
                {
                    if ((jpeg_dev->width >> scale) <= picinfo.S_Width &&
                        (jpeg_dev->height >> scale) <= picinfo.S_Height)//在目标区域内
                    {
                        if (((jpeg_dev->width >> scale) != picinfo.S_Width) &&
                            ((jpeg_dev->height >> scale) != picinfo.S_Height && scale))
                            scale = 0;//不能贴边,则不缩放
                        else outfun = jpeg_out_func_fill;    //在显示尺寸以内,可以采用填充的方式显示
                        break;
                    }
                }
                if (scale == 4)scale = 0;//错误
                picinfo.ImgHeight = jpeg_dev->height >> scale;    //缩放后的图片尺寸
                picinfo.ImgWidth = jpeg_dev->width >> scale;    //缩放后的图片尺寸
                ai_draw_init();                                //初始化智能画图
                //执行解码工作，调用TjpgDec模块的jd_decomp函数
                res = jd_decomp(jpeg_dev, outfun, scale);
            }
        }
        f_close(f_jpeg); //解码工作执行成功，返回0
    }
    jpeg_freeall();        //释放内存
    return res;
}


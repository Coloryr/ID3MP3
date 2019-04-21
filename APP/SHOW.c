#include "includes.h"
#include "show.h"
#include "lcd.h"
#include "ff.h"
#include "mp3player.h"
#include "text.h"
#include "vs10xx.h"
#include "string.h"
#include "guix.h"
#include "tjpgd.h"
#include "piclib.h"

u8 lcd_bit = 0;
FIL *fmp3_pic = 0;

JDEC *jpeg_dev; //待解码对象结构体指针
u8 *jpg_buffer; //定义jpeg解码工作区大小(最少需要3092字节)，作为解压缓冲区，必须4字节对齐

//给占内存大的数组/结构体申请内存
u8 jpeg_mallocall(void)
{
	jpeg_dev = (JDEC *)mymalloc(sizeof(JDEC));
	if (jpeg_dev == NULL)
		return PIC_MEM_ERR; //申请内存失败.
	jpg_buffer = (u8 *)mymalloc(JPEG_WBUF_SIZE);
	if (jpg_buffer == NULL)
		return PIC_MEM_ERR; //申请内存失败.
	return 0;
}
//释放内存
void jpeg_freeall(void)
{
	myfree(jpeg_dev);   //释放jpeg_dev申请到的内存
	myfree(jpg_buffer); //释放jpg_buffer申请到的内存
}

void show_mp3_pic(void *pdata)
{
	u8 res;
	UINT(*outfun)
	(JDEC *, void *, JRECT *);
	u8 scale; //图像输出比例 0,1/2,1/4,1/8
	CPU_SR_ALLOC();
	while (1)
	{
		if (info.pic_show == 1)
		{
			OS_CRITICAL_ENTER(); //进入临界区
			//得到显示方框大小
			picinfo.S_Height = pic_show_size;
			picinfo.S_Width = pic_show_size;
			//显示的开始坐标点
			picinfo.S_YOFF = pic_show_y;
			picinfo.S_XOFF = pic_show_x;

			res = jpeg_mallocall();
			if (res == 0)
			{
				//得到JPEG/JPG图片的开始信息
				if (res == FR_OK) //打开文件成功
				{
					res = jd_prepare(jpeg_dev, jpeg_in_func, jpg_buffer, JPEG_WBUF_SIZE, fmp3_pic); //执行解码的准备工作，调用TjpgDec模块的jd_prepare函数
					outfun = jpeg_out_func_point;													//默认采用画点的方式显示
					if (res == JDR_OK)																//准备解码成功
					{
						for (scale = 0; scale < 4; scale++) //确定输出图像的比例因子
						{
							if ((jpeg_dev->width >> scale) <= picinfo.S_Width && (jpeg_dev->height >> scale) <= picinfo.S_Height) //在目标区域内
							{
								if (((jpeg_dev->width >> scale) != picinfo.S_Width) && ((jpeg_dev->height >> scale) != picinfo.S_Height && scale))
									scale = 0; //不能贴边,则不缩放
								else
									outfun = jpeg_out_func_fill; //在显示尺寸以内,可以采用填充的方式显示
								break;
							}
						}
						if (scale == 4) //错误
						{
							scale = 0;
							Show_Str(0, 0, 240, 16, "图片错误", 16, 0);
						}
						picinfo.ImgHeight = jpeg_dev->height >> scale; //缩放后的图片尺寸
						picinfo.ImgWidth = jpeg_dev->width >> scale;   //缩放后的图片尺寸
						ai_draw_init();								   //初始化智能画图
						write_bit = 0x00;
						//执行解码工作，调用TjpgDec模块的jd_decomp函数
						OS_CRITICAL_EXIT(); //进入临界区
						res = jd_decomp(jpeg_dev, outfun, scale);
						OS_CRITICAL_ENTER(); //进入临界区
					}
				}
			}
			jpeg_freeall(); //释放内存
			info.pic_show = 0;
			write_bit = 0x20;
			OS_CRITICAL_EXIT(); //进入临界区
		}
		else if (info.pic_show == 2)
		{
			OS_CRITICAL_ENTER(); //进入临界区
			LCD_Fill(0, 0, 240, (240 + 16 * 3) - 1, BLACK);
			show_all(1); //显示一次歌名
			show_all(3); //显示歌曲信息
			Show_Str(0, 0, 240, 16, "PNG格式图片不支持", 16, 0);
			info.pic_show = 0;
			OS_CRITICAL_EXIT(); //进入临界区
		}
		else if (info.pic_show == 3)
		{
			OS_CRITICAL_ENTER(); //进入临界区
			LCD_Fill(0, 0, 240, (240 + 16 * 3) - 1, BLACK);
			show_all(1); //显示一次歌名
			show_all(3); //显示歌曲信息
			Show_Str(0, 0, 240, 16, "没有图片", 16, 0);
			info.pic_show = 0;
			OS_CRITICAL_EXIT(); //进入临界区
		}
	}
}

void show_all(u8 mode)
{
	u16 temp = 0;
	info.time = VS_Get_DecodeTime(); //得到解码时间
	if (mode == 1)
	{
		if (info.size != 0)
		{
			if (info.TIT2[0] != 0)
				Show_Str(0, 240, 240, 16, info.TIT2, 16, 0); //显示歌曲名字
			if (info.TPE1[0] != 0)
				Show_Str(0, 240 + 17, 240, 16, info.TPE1, 16, 0); //显示歌曲作者
			if (info.TALB[0] != 0)
				Show_Str(0, 240 + 34, 240, 16, info.TALB, 16, 0); //显示歌曲专辑
			if (info.TIT2[0] == 0 && info.TPE1[0] == 0 && info.TALB[0] == 0)
				Show_Str(0, 240, 240, 16, info.fn, 16, 0); //显示歌曲名字
		}
		else
		{
			Show_Str(0, 0, 320, 16, info.fn, 16, 0); //显示歌曲名字
		}
		//Show_Str(186, 292, 48, 24, "设置", 24, 0);
	}
	else if (((info.pic_show == 0 && mode == 0 && lcd_bit == 1) || mode == 3))
	{
		if (info.playtime == 0)
			info.playtime = info.time;
		else if (((info.time != info.playtime) && (info.time != 0)) || mode == 3) //1s时间到,更新显示数据
		{
			info.playtime = info.time; //更新时间
			temp = VS_Get_HeadInfo();  //获得比特率
			if (temp != info.kbps)
				info.kbps = temp; //更新KBPS
			//显示播放时间
			LCD_ShowxNum(0, 289, info.time / 60, 2, 16, 0X80); //分钟
			LCD_ShowChar(16, 289, ':', 16, 0);
			LCD_ShowxNum(24, 289, info.time % 60, 2, 16, 0X80); //秒钟
			LCD_ShowChar(40, 289, '/', 16, 0);
			//显示总时间
			if (info.kbps)
				info.time = ((info.fmp3->fsize - info.size) / info.kbps) / 125; //得到秒钟数   (文件长度(字节)/(1000/8)/比特率=持续秒钟数
			else
				info.time = 0;									//非法位率
			LCD_ShowxNum(48, 289, info.time / 60, 2, 16, 0X80); //分钟
			LCD_ShowChar(64, 289, ':', 16, 0);
			LCD_ShowxNum(72, 289, info.time % 60, 2, 16, 0X80); //秒钟
			//显示位率
			LCD_ShowxNum(62, 303, info.kbps, 3, 16, 0X80); //显示位率
			LCD_ShowString(62 + 24, 303, 200, 16, 16, "Kbps");
		}
		LCD_ShowString(0, 303, 32, 16, 16, "VOL:");
		LCD_ShowxNum(0 + 32, 303, (vsset.mvol - 100) / 10, 2, 16, 0X80); //显示音量
		LCD_ShowxNum(95, 289, info.curindex + 1, 3, 16, 0X80);			 //索引
		LCD_ShowChar(95 + 24, 289, '/', 16, 0);
		LCD_ShowxNum(95 + 32, 289, info.totmp3num, 3, 16, 0X80); //总曲目
	}
	else if (mode == 2)
	{
		Show_Str(88, 20, 64, 16, "音效设置", 16, 0);

		Show_Str(10, 44, 64, 16, "音量大小", 16, 0);
		Show_Str(10, 76, 64, 16, "低音频率", 16, 0);
		Show_Str(10, 108, 64, 16, "低音增益", 16, 0);
		Show_Str(10, 140, 64, 16, "高音频率", 16, 0);
		Show_Str(10, 172, 64, 16, "高音增益", 16, 0);
		Show_Str(10, 204, 64, 16, "空间效果", 16, 0);

		LCD_ShowString(94, 40, 32, 24, 24, "<");
		LCD_ShowString(94, 72, 32, 24, 24, "<");
		LCD_ShowString(94, 104, 32, 24, 24, "<");
		LCD_ShowString(94, 136, 32, 24, 24, "<");
		LCD_ShowString(94, 168, 32, 24, 24, "<");
		LCD_ShowString(94, 200, 32, 24, 24, "<");

		LCD_ShowxNum(126, 44, (vsset.mvol - 100) / 10, 2, 16, 0X80);
		LCD_ShowxNum(126, 76, vsset.bflimit, 2, 16, 0X80);
		LCD_ShowxNum(126, 108, vsset.bass, 2, 16, 0X80);
		LCD_ShowxNum(126, 140, vsset.tflimit, 2, 16, 0X80);
		LCD_ShowxNum(126, 172, vsset.treble, 2, 16, 0X80);
		LCD_ShowxNum(130, 204, vsset.effect, 1, 16, 0X80);

		LCD_ShowString(162, 40, 32, 24, 24, ">");
		LCD_ShowString(162, 72, 32, 24, 24, ">");
		LCD_ShowString(162, 104, 32, 24, 24, ">");
		LCD_ShowString(162, 136, 32, 24, 24, ">");
		LCD_ShowString(162, 168, 32, 24, 24, ">");
		LCD_ShowString(162, 200, 32, 24, 24, ">");

		Show_Str(186, 292, 48, 24, "保存", 24, 0);
	}
}

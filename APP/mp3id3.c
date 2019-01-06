#include "mp3id3.h" 
#include "exfuns.h"
#include "ff.h"
#include "malloc.h" 
#include "piclib.h"
#include "lcd.h"
#include "tjpgd.h"
#include "text.h"
#include "delay.h"
#include "flash.h"

u16 size;

u8 *TIT2;						//歌名
u8 *TPE1;						//作者

u16 UNICODEtoGBK(u16 unicode)  //???????
{
	u16 t[2];
	u16 c;
	u32 i, li, hi;
	u16 n;
	u32 gbk2uni_offset = 0;

	if (unicode < 0x80)c = unicode;//ASCII,直接不用转换.
	else
	{
		//UNICODE 2 GBK   
		gbk2uni_offset = 0;
		/* Unicode to OEMCP */
		hi = ftinfo.ugbksize / 2;//对半开.
		hi = hi / 4 - 1;
		li = 0;
		for (n = 16; n; n--)
		{
			i = li + (hi - li) / 2;
			SPI_Flash_Read((u8*)&t, ftinfo.ugbkaddr + i * 4 + gbk2uni_offset, 4);//读出4个字节  
			if (unicode == t[0]) break;
			if (unicode > t[0])li = i;
			else hi = i;
		}
		c = n ? t[1] : 0;
	}
	return c;
}

//下面根据是否使用malloc来决定变量的分配方法.
#if JPEG_USE_MALLOC == 1 //使用malloc	 

FIL *f_jpeg;			//JPEG文件指针
JDEC *jpeg_dev;   		//待解码对象结构体指针  
u8  *jpg_buffer;    	//定义jpeg解码工作区大小(最少需要3092字节)，作为解压缓冲区，必须4字节对齐

//给占内存大的数组/结构体申请内存
u8 jpeg_mallocall(void)
{
	f_jpeg=(FIL*)mymalloc(sizeof(FIL));
	if(f_jpeg==NULL)return PIC_MEM_ERR;			//申请内存失败.	  
	jpeg_dev=(JDEC*)mymalloc(sizeof(JDEC));
	if(jpeg_dev==NULL)return PIC_MEM_ERR;		//申请内存失败.
	jpg_buffer=(u8*)mymalloc(JPEG_WBUF_SIZE);
	if(jpg_buffer==NULL)return PIC_MEM_ERR;		//申请内存失败. 
	return 0;
}
//释放内存
void jpeg_freeall(void)
{
	myfree(f_jpeg);			//释放f_jpeg申请到的内存
	myfree(jpeg_dev);		//释放jpeg_dev申请到的内存
	myfree(jpg_buffer);		//释放jpg_buffer申请到的内存
}

#else 	//不使用malloc   

FIL  tf_jpeg; 
JDEC tjpeg_dev;   		  
FIL  *f_jpeg=&tf_jpeg;						//JPEG文件指针
JDEC *jpeg_dev=&tjpeg_dev;   				//待解码对象结构体指针   
__align(4) u8 jpg_buffer[JPEG_WBUF_SIZE];	//定义jpeg解码工作区大小(最少需要3092字节)，作为解压缓冲区，必须4字节对齐
	
#endif

u16 mp3id3_is(const TCHAR* path, u8 pic_show)
{
	FIL* fmp3 = 0;
	u16 i = 0;
	u8 res = 0;
	u8 *databuf = 0;
	u8 img = 1;
	u8 temp;
	u16 tag_size;
	u8 code_type;
	u8 scale;	//图像输出比例 0,1/2,1/4,1/8  
	u16 a = 0;
	u8 temp1, temp2;
	UINT(*outfun)(JDEC*, void*, JRECT*);

	databuf = (u8*)mymalloc(READ_buff_size);
	fmp3 = (FIL*)mymalloc(sizeof(FIL));
	if (TIT2 == NULL)
		TIT2 = (u8*)mymalloc(40);
	if (TPE1 == NULL)
		TPE1 = (u8*)mymalloc(40);
	for (temp = 0; temp < 40; temp++)
	{
		TIT2[temp] = 0;
		TPE1[temp] = 0;
	}
	if (databuf == NULL || fmp3 == NULL)//内存申请失败.
		while (1)
		{
			Show_Str(30, 20, 160, 16, "MP3ID3：内存申请失败", 16, 0);
			delay_ms(200);
			LCD_Fill(30, 20, 160, 16, BLACK);//清除显示	     
			delay_ms(200);
		}
	while (f_open(fmp3, path, FA_READ | FA_OPEN_EXISTING))
	{
		Show_Str(30, 20, 160, 16, "MP3ID3：文件读取错误", 16, 0);
		delay_ms(200);
		LCD_Fill(30, 20, 160, 16, BLACK);//清除显示	     
		delay_ms(200);
	}
	res = f_read(fmp3, databuf, 10, (UINT*)&br);//读出mp3id3头
	if (databuf[0] == 0x49 && databuf[1] == 0x44 && databuf[2] == 0x33)
	{
		//计算大小
		size = databuf[6] & 0x7f | ((databuf[7] & 0x7f) << 7)
			| ((databuf[8] & 0x7f) << 14) | ((databuf[9] & 0x7f) << 21);
		res = f_read(fmp3, databuf, READ_buff_size, (UINT*)&br);
		i = 0;
		while (img)							//查找歌名
		{
			if (databuf[i] == 0x54 && databuf[i + 1] == 0x49 && databuf[i + 2] == 0x54 && databuf[i + 3] == 0x32)
			{	//找到位置
				tag_size = databuf[i + 4] << 24
					| databuf[i + 5] << 16
					| databuf[i + 6] << 8
					| databuf[i + 7];
				if (databuf[i + 11] == 0xFE && databuf[i + 12] == 0xFF && databuf[i + 10] == 0x01)
				{
					code_type = 2;							//UTF-16BE
					i += 13;
					tag_size -= 3;
				}
				else if (databuf[i + 11] == 0xFF && databuf[i + 12] == 0xFE && databuf[i + 10] == 0x01)
				{
					code_type = 1;							//UTF-16LE
					i += 13;
					tag_size -= 3;
				}
				else if (databuf[i + 10] == 0x00)
				{
					code_type = 0;							//iso-8859-1
					i += 11;
					tag_size -= 1;
				}
				else if (databuf[i + 10] == 0x03)
				{
					code_type = 3;							//UTF-8
					i += 11;
					tag_size -= 1;
				}
				if (code_type == 3)			//UTF-8
				{
					for (temp = 0; temp < tag_size;)
					{
						a = (databuf[i + temp] << 8) | databuf[i + temp + 1];
						if (a < 0x80) {				/* 7-bit */
							TIT2[temp++] = (BYTE)a;
						}
						else {
							if (a < 0x800) {		/* 11-bit */
								TIT2[temp++] = (BYTE)(0xC0 | a >> 6);
							}
							else {				/* 16-bit */
								TIT2[temp++] = (BYTE)(0xE0 | a >> 12);
								TIT2[temp++] = (BYTE)(0x80 | (a >> 6 & 0x3F));
							}
							TIT2[temp++] = (BYTE)(0x80 | (a & 0x3F));
						}
					}
				}
				else if (code_type == 2)			//UTF-16BE
				{
					temp2 = 0;
					for (temp = 0; temp < tag_size;)
					{
						a = (databuf[i + temp] << 8) | databuf[i + temp + 1];
						a = UNICODEtoGBK(a);
						temp1 = (a >> 8) & 0xff;
						if (temp1 != 0x00)
						{
							TIT2[temp2] = temp1;
							temp2++;
						}
						temp1 = a & 0xff;
						if (temp1 != 0x00)
						{
							TIT2[temp2] = temp1;
							temp2++;
						}
						temp += 2;
					}
				}
				else if (code_type == 1)			//UTF-16LE
				{
					temp2 = 0;
					for (temp = 0; temp < tag_size;)
					{
						a = (databuf[i + temp + 1] << 8) | databuf[i + temp];
						a = UNICODEtoGBK(a);
						temp1 = (a >> 8) & 0xff;
						if (temp1 != 0x00)
						{
							TIT2[temp2] = temp1;
							temp2++;
						}
						temp1 = a & 0xff;
						if (temp1 != 0x00)
						{
							TIT2[temp2] = temp1;
							temp2++;
						}
						temp += 2;
					}
				}
				else if (code_type == 0)			//iso-8859-1
				{
					for (temp = 0; temp < tag_size;)
					{
						temp1 = databuf[i + temp];
						if (temp1 != 0x00)
							TIT2[temp++] = temp1;
						else
							temp++;
					}
				}
				img = 0;
			}
			else
				i++;
			if (i >= 4096)								//找不到位置
			{
				TPE1 = 0;
				img = 0;
				code_type = 4;
			}
		}
		img = 1;
		i = 0;
		while (img)							//查找作者
		{
			myfree(TPE1);
			if (databuf[i] == 0x54 && databuf[i + 1] == 0x50 && databuf[i + 2] == 0x45 && databuf[i + 3] == 0x31)
			{	//找到位置
				tag_size = databuf[i + 4] << 24
					| databuf[i + 5] << 16
					| databuf[i + 6] << 8
					| databuf[i + 7];
				if (databuf[i + 11] == 0xFE && databuf[i + 12] == 0xFF && databuf[i + 10] == 0x01)
				{
					code_type = 2;							//UTF-16BE
					i += 13;
					tag_size -= 3;
				}
				else if (databuf[i + 11] == 0xFF && databuf[i + 12] == 0xFE && databuf[i + 10] == 0x01)
				{
					code_type = 1;							//UTF-16LE
					i += 13;
					tag_size -= 3;
				}
				else if (databuf[i + 10] == 0x00)
				{
					code_type = 0;							//iso-8859-1
					i += 11;
					tag_size -= 1;
				}
				else if (databuf[i + 10] == 0x03)
				{
					code_type = 3;							//UTF-8
					i += 11;
					tag_size -= 1;
				}
				TPE1 = (u8*)mymalloc(tag_size);

				if (code_type == 3)			//UTF-8
				{
					for (temp = 0; temp < tag_size;)
					{
						a = (databuf[i + temp] << 8) | databuf[i + temp + 1];
						if (a < 0x80) {				/* 7-bit */
							TPE1[temp++] = (BYTE)a;
						}
						else {
							if (a < 0x800) {		/* 11-bit */
								TPE1[temp++] = (BYTE)(0xC0 | a >> 6);
							}
							else {				/* 16-bit */
								TPE1[temp++] = (BYTE)(0xE0 | a >> 12);
								TPE1[temp++] = (BYTE)(0x80 | (a >> 6 & 0x3F));
							}
							TPE1[temp++] = (BYTE)(0x80 | (a & 0x3F));
						}
					}
				}
				else if (code_type == 2)			//UTF-16BE
				{
					temp2 = 0;
					for (temp = 0; temp < tag_size;)
					{
						a = (databuf[i + temp] << 8) | databuf[i + temp + 1];
						a = UNICODEtoGBK(a);
						temp1 = (a >> 8) & 0xff;
						if (temp1 != 0x00)
						{
							TPE1[temp2] = temp1;
							temp2++;
						}
						temp1 = a & 0xff;
						if (temp1 != 0x00)
						{
							TPE1[temp2] = temp1;
							temp2++;
						}
						temp += 2;
					}
				}
				else if (code_type == 1)			//UTF-16LE
				{
					temp2 = 0;
					for (temp = 0; temp < tag_size;)
					{
						a = (databuf[i + temp + 1] << 8) | databuf[i + temp];
						a = UNICODEtoGBK(a);
						temp1 = (a >> 8) & 0xff;
						if (temp1 != 0x00)
						{
							TPE1[temp2] = temp1;
							temp2++;
						}
						temp1 = a & 0xff;
						if (temp1 != 0x00)
						{
							TPE1[temp2] = temp1;
							temp2++;
						}
						temp += 2;
					}
				}
				else if (code_type == 0)			//iso-8859-1
				{
					for (temp = 0; temp < tag_size;)
					{
						temp1 = databuf[i + temp];
						if (temp1 != 0x00)
						{
							TPE1[temp] = temp1;
							temp++;
						}
						else
							temp++;
					}
				}
				img = 0;
			}
			else
				i++;
			if (i >= 4096)								//找不到位置
			{
				TPE1 = 0;
				code_type = 4;
				img = 0;
			}
		}
		img = 1;
		i = 0;
		while (img)
		{
			if (databuf[i] == 0x41 && databuf[i + 1] == 0x50 && databuf[i + 2] == 0x49 && databuf[i + 3] == 0x43)
			{	//找到位置
				if (databuf[i + 24] == 0xff && databuf[i + 25] == 0xd8 && databuf[i + 26] == 0xff && databuf[i + 27] == 0xe0)
				{
					code_type = 0; //JPG
				}
				if (databuf[i + 24] == 0x89 && databuf[i + 25] == 0x50 && databuf[i + 26] == 0x4e && databuf[i + 27] == 0x47)
				{
					code_type = 1; //PNG
				}
				img = 0;
			}
			else
				i++;
			if (i >= 4096)								//找不到位置
			{
				code_type = 2;
				code_type = 2;
				img = 0;
			}
		}
		if (pic_show == 1 && code_type == 0)
		{
			if ((pic_show_x + pic_show_size) > picinfo.lcdwidth)return PIC_WINDOW_ERR;		//x坐标超范围了.
			if ((pic_show_y + pic_show_size) > picinfo.lcdheight)return PIC_WINDOW_ERR;		//y坐标超范围了.  
			//得到显示方框大小	  	 
			picinfo.S_Height = pic_show_size;
			picinfo.S_Width = pic_show_size;
			//显示区域无效
			if (picinfo.S_Height == 0 || picinfo.S_Width == 0)
			{
				picinfo.S_Height = lcddev.height;
				picinfo.S_Width = lcddev.width;
				return FALSE;
			}
			//显示的开始坐标点
			picinfo.S_YOFF = pic_show_y;
			picinfo.S_XOFF = pic_show_x;

#if JPEG_USE_MALLOC == 1	//使用malloc
			res = jpeg_mallocall();
#endif
			if (res == 0)
			{
				i += 14 + 20;
				f_lseek(fmp3, i);				//跳过头

				//得到JPEG/JPG图片的开始信息		 

				if (res == FR_OK)//打开文件成功
				{
					res = jd_prepare(jpeg_dev, jpeg_in_func, jpg_buffer, JPEG_WBUF_SIZE, fmp3);//执行解码的准备工作，调用TjpgDec模块的jd_prepare函数
					outfun = jpeg_out_func_point;//默认采用画点的方式显示
					if (res == JDR_OK)//准备解码成功 
					{
						for (scale = 0; scale < 4; scale++)//确定输出图像的比例因子
						{
							if ((jpeg_dev->width >> scale) <= picinfo.S_Width && (jpeg_dev->height >> scale) <= picinfo.S_Height)//在目标区域内
							{
								if (((jpeg_dev->width >> scale) != picinfo.S_Width) && ((jpeg_dev->height >> scale) != picinfo.S_Height&&scale))scale = 0;//不能贴边,则不缩放
								else outfun = jpeg_out_func_fill;	//在显示尺寸以内,可以采用填充的方式显示 
								break;
							}
						}
						if (scale == 4)scale = 0;//错误
						picinfo.ImgHeight = jpeg_dev->height >> scale;	//缩放后的图片尺寸
						picinfo.ImgWidth = jpeg_dev->width >> scale;	//缩放后的图片尺寸 
						ai_draw_init();								//初始化智能画图 
						//执行解码工作，调用TjpgDec模块的jd_decomp函数
						res = jd_decomp(jpeg_dev, outfun, scale);
					}
				}
			}
#if JPEG_USE_MALLOC == 1//使用malloc
			jpeg_freeall();		//释放内存
#endif
		}
		else if (code_type == 1 && pic_show == 1)
		{
			i += 14 + 20;
			f_lseek(fmp3, i);				//跳过头
		}
		else if (code_type == 2 && pic_show == 1)
		{
			LCD_Fill(pic_show_x, pic_show_y, pic_show_x + pic_show_size,
				pic_show_y + pic_show_size, BACK_COLOR);
		}
		f_close(fmp3);
		myfree(fmp3);
		myfree(databuf);				//释放内存			    
		return size;
	}
	f_close(fmp3);
	myfree(fmp3);
	myfree(databuf);						//释放内存			     
	return 0;
}








#include "includes.h" 
#include "show.h"
#include "lcd.h"
#include "lunar.h"
#include "ff.h"
#include "rtc.h"
#include "mp3player.h"
#include "text.h"
#include "vs10xx.h"
#include "string.h"
#include "guix.h"
#include "tjpgd.h"
#include "piclib.h"

u8 lcd_bit=0;
FIL* fmp3 = 0;

u8 show_mode = 0;

_lunar_obj moon;

/*显示生肖-----------------------------------------------------*/
void displaysx(void) {
	unsigned char LunarYearD, ReYear, tiangan, dizhi;        //?????????  ??????? 
	   //?????,??????,?c_moon????,year_moon???,month_moon,day_moon????BCD??
	LunarYearD = (moon.year_moon / 16) * 10 + moon.year_moon % 16;        //??????10??? 
	ReYear = LunarYearD % 12;                             //????12,????
	tiangan = LunarYearD % 10;         //??  ???.....
	dizhi = LunarYearD % 12;         //??  ???.....

	switch (tiangan)	//天干
	{
	case 0: Show_Str(10, 130, 24, 24, "庚", 24, 0); break;   //余0即整除 天干  庚
	case 1: Show_Str(10, 130, 24, 24, "辛", 24, 0); break;   //
	case 2: Show_Str(10, 130, 24, 24, "壬", 24, 0); break;
	case 3: Show_Str(10, 130, 24, 24, "癸", 24, 0); break;
	case 4: Show_Str(10, 130, 24, 24, "甲", 24, 0); break;
	case 5: Show_Str(10, 130, 24, 24, "乙", 24, 0); break;
	case 6: Show_Str(10, 130, 24, 24, "丙", 24, 0); break;
	case 7: Show_Str(10, 130, 24, 24, "丁", 24, 0); break;
	case 8: Show_Str(10, 130, 24, 24, "戊", 24, 0); break;
	case 9: Show_Str(10, 130, 24, 24, "已", 24, 0); break;
	}

	switch (dizhi) //地支
	{
	case 0: Show_Str(34, 130, 24, 24, "辰", 24, 0); break;   //余0即整除 地支 辰
	case 1: Show_Str(34, 130, 24, 24, "巳", 24, 0); break;   //
	case 2: Show_Str(34, 130, 24, 24, "午", 24, 0); break;
	case 3: Show_Str(34, 130, 24, 24, "未", 24, 0); break;
	case 4: Show_Str(34, 130, 24, 24, "申", 24, 0); break;
	case 5: Show_Str(34, 130, 24, 24, "酉", 24, 0); break;
	case 6: Show_Str(34, 130, 24, 24, "戌", 24, 0); break;
	case 7: Show_Str(34, 130, 24, 24, "亥", 24, 0); break;
	case 8: Show_Str(34, 130, 24, 24, "子", 24, 0); break;
	case 9: Show_Str(34, 130, 24, 24, "丑", 24, 0); break;
	case 10:Show_Str(34, 130, 24, 24, "寅", 24, 0); break;
	case 11:Show_Str(34, 130, 24, 24, "卯", 24, 0); break;
	}

	switch (ReYear)	//生肖
	{
	case 0: Show_Str(58, 130, 24, 24, "龙", 24, 0); break;   //余0即整除 农历 龙年
	case 1: Show_Str(58, 130, 24, 24, "蛇", 24, 0); break;   //蛇年
	case 2: Show_Str(58, 130, 24, 24, "马", 24, 0); break;
	case 3: Show_Str(58, 130, 24, 24, "羊", 24, 0); break;
	case 4: Show_Str(58, 130, 24, 24, "猴", 24, 0); break;
	case 5: Show_Str(58, 130, 24, 24, "鸡", 24, 0); break;
	case 6: Show_Str(58, 130, 24, 24, "狗", 24, 0); break;
	case 7: Show_Str(58, 130, 24, 24, "猪", 24, 0); break;
	case 8: Show_Str(58, 130, 24, 24, "鼠", 24, 0); break;
	case 9: Show_Str(58, 130, 24, 24, "牛", 24, 0); break;
	case 10:Show_Str(58, 130, 24, 24, "虎", 24, 0); break;
	case 11:Show_Str(58, 130, 24, 24, "兔", 24, 0); break;
	}
	Show_Str(84, 130, 24, 24, "年", 24, 0);
}

/*显示农历日期---------------------------------------------------------------*/
void displaynl(void)
{
	unsigned char a, b, c;
	a = calendar.w_year % 1000 % 100 % 10;
	a = a + (calendar.w_year % 1000 % 100 / 10) * 16;
	b = calendar.w_month % 10;
	b = b + (calendar.w_month / 10) * 16;
	c = calendar.w_date % 10;
	c = c + (calendar.w_date / 10) * 16;
	Conversion(0, a, b, c);        //?????
/*****	显示农历月份  *****/

	if (moon.month_moon == 1) { Show_Str(116, 130, 24, 24, "正", 24, 0); }
	if (moon.month_moon == 2) { Show_Str(116, 130, 24, 24, "二", 24, 0); }
	if (moon.month_moon == 3) { Show_Str(116, 130, 24, 24, "三", 24, 0); }
	if (moon.month_moon == 4) { Show_Str(116, 130, 24, 24, "四", 24, 0); }
	if (moon.month_moon == 5) { Show_Str(116, 130, 24, 24, "五", 24, 0); }
	if (moon.month_moon == 6) { Show_Str(116, 130, 24, 24, "六", 24, 0); }
	if (moon.month_moon == 7) { Show_Str(116, 130, 24, 24, "七", 24, 0); }
	if (moon.month_moon == 8) { Show_Str(116, 130, 24, 24, "八", 24, 0); }
	if (moon.month_moon == 9) { Show_Str(116, 130, 24, 24, "九", 24, 0); }
	if (moon.month_moon == 10) { Show_Str(116, 130, 24, 24, "十", 24, 0); }
	if (moon.month_moon == 11) { Show_Str(116, 130, 24, 24, "冬", 24, 0); }
	if (moon.month_moon == 12) { Show_Str(116, 130, 24, 24, "腊", 24, 0); }
	Show_Str(140, 130, 24, 24, "月", 24, 0);

	/*****	显示农历日的十位  *****/
	if (moon.day_moon / 10 == 0) { Show_Str(164, 130, 24, 24, "初", 24, 0); }    //
	if (moon.day_moon / 10 == 1)
		if (moon.day_moon / 10 == 1 & moon.day_moon % 10 == 0) { Show_Str(164, 130, 24, 24, "初", 24, 0); }
		else { Show_Str(164, 130, 24, 24, "十", 24, 0); }    //
	if (moon.day_moon / 10 == 2)
		if (moon.day_moon / 10 == 2 & moon.day_moon % 10 == 0) { Show_Str(164, 130, 24, 24, "二", 24, 0); }
		else { Show_Str(164, 130, 24, 24, "廿", 24, 0); }   //
	if (moon.day_moon / 10 == 3) { Show_Str(164, 130, 24, 24, "三", 24, 0); }	  //"三"字的代码

/*****	显示农历日的个位  *****/

	if (moon.day_moon % 10 == 1) { Show_Str(188, 130, 24, 24, "一", 24, 0); }    //
	if (moon.day_moon % 10 == 2) { Show_Str(188, 130, 24, 24, "二", 24, 0); }    //
	if (moon.day_moon % 10 == 3) { Show_Str(188, 130, 24, 24, "三", 24, 0); }	  //"三"字的代码
	if (moon.day_moon % 10 == 4) { Show_Str(188, 130, 24, 24, "四", 24, 0); }    //
	if (moon.day_moon % 10 == 5) { Show_Str(188, 130, 24, 24, "五", 24, 0); }    //
	if (moon.day_moon % 10 == 6) { Show_Str(188, 130, 24, 24, "六", 24, 0); }    //
	if (moon.day_moon % 10 == 7) { Show_Str(188, 130, 24, 24, "七", 24, 0); }    //
	if (moon.day_moon % 10 == 8) { Show_Str(188, 130, 24, 24, "八", 24, 0); }    //
	if (moon.day_moon % 10 == 9) { Show_Str(188, 130, 24, 24, "九", 24, 0); }    //
	if (moon.day_moon % 10 == 0) { Show_Str(188, 130, 24, 24, "十", 24, 0); }    //
}

/*----------显示节气---------------------------------------------*/
void displayjieqi(void)
{
	unsigned char j, a, b, c;
	a = calendar.w_year % 1000 % 100 % 10;
	a = a + (calendar.w_year % 1000 % 100 / 10) * 16;
	b = calendar.w_month % 10;
	b = b + (calendar.w_month / 10) * 16;
	c = calendar.w_date % 10;
	c = c + (calendar.w_date / 10) * 16;
	j = jieqi(a, b, c);
	if (j == 1) { Show_Str(220, 130, 96, 24, "今天小寒", 24, 0); }
	if (j == 2) { Show_Str(220, 130, 96, 24, "今天大寒", 24, 0); }
	if (j == 3) { Show_Str(220, 130, 96, 24, "今天立春", 24, 0); }
	if (j == 4) { Show_Str(220, 130, 96, 24, "今天雨水", 24, 0); }
	if (j == 5) { Show_Str(220, 130, 96, 24, "今天惊蛰", 24, 0); }
	if (j == 6) { Show_Str(220, 130, 96, 24, "今天春分", 24, 0); }
	if (j == 7) { Show_Str(220, 130, 96, 24, "今天清明", 24, 0); }
	if (j == 8) { Show_Str(220, 130, 96, 24, "今天谷雨", 24, 0); }
	if (j == 9) { Show_Str(220, 130, 96, 24, "今天立夏", 24, 0); }
	if (j == 10) { Show_Str(220, 130, 96, 24, "今天小满", 24, 0); }
	if (j == 11) { Show_Str(220, 130, 96, 24, "今天芒种", 24, 0); }
	if (j == 12) { Show_Str(220, 130, 96, 24, "今天夏至", 24, 0); }
	if (j == 13) { Show_Str(220, 130, 96, 24, "今天小暑", 24, 0); }
	if (j == 14) { Show_Str(220, 130, 96, 24, "今天大暑", 24, 0); }
	if (j == 15) { Show_Str(220, 130, 96, 24, "今天立秋", 24, 0); }
	if (j == 16) { Show_Str(220, 130, 96, 24, "今天处暑", 24, 0); }
	if (j == 17) { Show_Str(220, 130, 96, 24, "今天白露", 24, 0); }
	if (j == 18) { Show_Str(220, 130, 96, 24, "今天秋分", 24, 0); }
	if (j == 19) { Show_Str(220, 130, 96, 24, "今天寒露", 24, 0); }
	if (j == 20) { Show_Str(220, 130, 96, 24, "今天霜降", 24, 0); }
	if (j == 21) { Show_Str(220, 130, 96, 24, "今天立冬", 24, 0); }
	if (j == 22) { Show_Str(220, 130, 96, 24, "今天小雪", 24, 0); }
	if (j == 23) { Show_Str(220, 130, 96, 24, "今天大雪", 24, 0); }
	if (j == 24) { Show_Str(220, 130, 96, 24, "今天冬至", 24, 0); }
	if (j == 0) { Show_Str(224, 130, 96, 24, "无节气", 24, 0); }
}

void timeplay(void)
{
	//static u8 h = 0, m = 0;
	Show_Str(58, 0, 24, 24, "年", 24, 0);
	Show_Str(114, 0, 24, 24, "月", 24, 0);
	Show_Str(170, 0, 24, 24, "日", 24, 0);
	Show_Str(220, 0, 48, 24, "星期", 24, 0);
	LCD_ShowxNum(10, 0, calendar.w_year, 4, 24, 0);//显示年
	LCD_ShowxNum(90, 0, calendar.w_month, 2, 24, 0);//显示月
	LCD_ShowxNum(146, 0, calendar.w_date, 2, 24, 0);//显示日

	switch (calendar.week)
	{
	case 0:
		Show_Font(268, 0, "天", 24, 0);
		break;
	case 1:
		Show_Font(268, 0, "一", 24, 0);
		break;
	case 2:
		Show_Font(268, 0, "二", 24, 0);
		break;
	case 3:
		Show_Font(268, 0, "三", 24, 0);
		break;
	case 4:
		Show_Font(268, 0, "四", 24, 0);
		break;
	case 5:
		Show_Font(268, 0, "五", 24, 0);
		break;
	case 6:
		Show_Font(268, 0, "六", 24, 0);
		break;
	}

	/*
	if (h != calendar.hour)
	{
		if (calendar.hour / 10 != 0)
		{
			image_display(0, 30, (u8*)gImage[calendar.hour / 10]);//在指定地址显示图?
		}
		else { image_display(0, 30, (u8*)gImage[11]); }

		image_display(50, 30, (u8*)gImage[calendar.hour % 10]);//在指定地址显示图片?
		h = calendar.hour;
	}
	if (calendar.sec % 10 == 0) { image_display(100, 30, (u8*)gImage[10]); }
	if (calendar.sec % 10 == 2) { image_display(100, 30, (u8*)gImage[10]); }
	if (calendar.sec % 10 == 4) { image_display(100, 30, (u8*)gImage[10]); }
	if (calendar.sec % 10 == 6) { image_display(100, 30, (u8*)gImage[10]); }
	if (calendar.sec % 10 == 8) { image_display(100, 30, (u8*)gImage[10]); }
	if (calendar.sec % 10 == 1) { image_display(100, 30, (u8*)gImage[12]); }
	if (calendar.sec % 10 == 3) { image_display(100, 30, (u8*)gImage[12]); }
	if (calendar.sec % 10 == 5) { image_display(100, 30, (u8*)gImage[12]); }
	if (calendar.sec % 10 == 7) { image_display(100, 30, (u8*)gImage[12]); }
	if (calendar.sec % 10 == 9) { image_display(100, 30, (u8*)gImage[12]); }

	if (m != calendar.min)
	{
		image_display(150, 30, (u8*)gImage[calendar.min / 10]);
		image_display(200, 30, (u8*)gImage[calendar.min % 10]);
		m = calendar.min;
	}
	*/
	LCD_ShowChar(250, 86, ':', 24, 0);
	LCD_ShowxNum(262, 86, calendar.sec, 2, 24, 0x80);
}
          
/**********************************************************
正常界面
**********************************************************/
void time_go(void)
{
	timeplay();
	displaynl();
	displayjieqi();
	displaysx();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//显示一根柱子
//x,y,width,height:位置和尺寸
//curval:当值
//topval:最大值
void fft_show_oneband(u16 x, u16 y, u16 width, u16 height, u16 curval, u16 topval)
{
	gui_fill_rectangle(x, y, width, height - curval, BLACK);					//填充背景色
	gui_fill_rectangle(x, y + height - curval, width, curval, FFT_BANDS_COLOR);	//填充柱状色
	gui_draw_hline(x, y + height - topval - 1, width, FFT_TOP_COLOR);
}

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

void show_mp3_pic(void *pdata)
{
	u8 res;
	UINT(*outfun)(JDEC*, void*, JRECT*);
	u8 scale;	//图像输出比例 0,1/2,1/4,1/8  
	CPU_SR_ALLOC();
	while (1)
	{
		if (info.pic_show == 1 && show_mode == 1)
		{
			OS_CRITICAL_ENTER();	//进入临界区
			//得到显示方框大小	  	 
			picinfo.S_Height = pic_show_size;
			picinfo.S_Width = pic_show_size;
			//显示的开始坐标点
			picinfo.S_YOFF = pic_show_y;
			picinfo.S_XOFF = pic_show_x;

			res = jpeg_mallocall();
			if (res == 0)
			{
				f_lseek(fmp3, info.pic_local);				//跳过头
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
						OS_CRITICAL_EXIT();	//进入临界区
						res = jd_decomp(jpeg_dev, outfun, scale);
						OS_CRITICAL_ENTER();	//进入临界区
					}
				}
			}
			jpeg_freeall();		//释放内存
			info.pic_show = 0;
			if (write_bit == 0x20)
			{
				write_bit = 0x30;
			}
			else if (write_bit == 0x10)
			{
				write_bit = 0x30;
			}
			OS_CRITICAL_EXIT();	//进入临界区			
		}
	}
}

void show_clear(void)
{
	if(show_mode==0)
		LCD_Fill(0, 162, 320, 178, BLACK);
	else
		LCD_Fill(0, 0, 320, 16, BLACK);
}

void show_pic_clear(void)
{
	if(show_mode==0)
	{
		LCD_Fill(pic_show_x, pic_show_y, pic_show_x + pic_show_size,
				pic_show_y + pic_show_size, BACK_COLOR);
	}
}

void show_all(void)
{
	u8 *fn;
	u16 temp = 0;
	static u8 sec;
	if (show_mode == 1 && info.pic_show == 0)
	{
		VS_Get_Spec(info.FFTbuf); //提取频谱数据
		FFT_post(info.FFTbuf);	  //进行频谱效果显示
	}
	RTC_Get();
	info.time = VS_Get_DecodeTime(); //得到解码时间
	if (sec != calendar.sec && lcd_bit == 1)
	{
		sec = calendar.sec;
		fn = (u8*)(*info.mp3fileinfo.lfname ? info.mp3fileinfo.lfname : info.mp3fileinfo.fname);
		temp = VS_Get_HeadInfo(); //获得比特率
		if (info.kbps == 0xffff)//未更新过
		{
			info.playtime = 0;
			info.kbps = VS_Get_HeadInfo();//获得比特率
		}

		if (show_mode == 0)
		{
			time_go();
			if (info.size != 0)
			{
				if (info.TIT2[0] != 0 && info.TPE1[0] != 0 && info.TALB[0] != 0)
				{
					strcat((char*)info.TIT2, "-");
					strcat((char*)info.TIT2, (char*)info.TPE1);
					strcat((char*)info.TIT2, "-");
					strcat((char*)info.TIT2, (char*)info.TALB);
					Show_Str(0, 162, 320, 16, info.TIT2, 16, 0);				//显示歌曲名字 
				}
				else if (info.TIT2 != NULL && info.TIT2[0] != 0x00)
				{
					Show_Str(0, 162, 320, 16, info.TIT2, 16, 0);				//显示歌曲名字 
				}
				else
					Show_Str(0, 162, 320, 16, fn, 16, 0);				//显示歌曲名字 
			}
			else
			{
				Show_Str(0, 162, 320, 16, fn, 16, 0);				//显示歌曲名字 
			}
			LCD_ShowxNum(0, 182, info.curindex + 1, 3, 16, 0X80);		//索引
			LCD_ShowChar(24, 182, '/', 16, 0);
			LCD_ShowxNum(32, 182, info.totmp3num, 3, 16, 0X80); 	//总曲目	
			LCD_ShowString(32 + 30, 182, 32, 16, 16, "VOL:");
			LCD_ShowxNum(64 + 30, 182, (vsset.mvol - 100) / 5, 2, 16, 0X80); 	//显示音量		

			if (temp != info.kbps)
			{
				info.kbps = temp;//更新KBPS	  				     
			}
			//显示播放时间			 
			LCD_ShowxNum(118, 182, info.time / 60, 2, 16, 0X80);		//分钟
			LCD_ShowChar(118 + 16, 182, ':', 16, 0);
			LCD_ShowxNum(118 + 24, 182, info.time % 60, 2, 16, 0X80);	//秒钟		
			LCD_ShowChar(118 + 40, 182, '/', 16, 0);
			//显示总时间
			if (info.kbps)info.time = (info.fmp3->fsize / info.kbps) / 125;//得到秒钟数   (文件长度(字节)/(1000/8)/比特率=持续秒钟数    	  
			else info.time = 0;//非法位率	  
			LCD_ShowxNum(118 + 48, 182, info.time / 60, 2, 16, 0X80);	//分钟
			LCD_ShowChar(118 + 64, 182, ':', 16, 0);
			LCD_ShowxNum(118 + 72, 182, info.time % 60, 2, 16, 0X80);	//秒钟	  		    
			//显示位率			   
			LCD_ShowxNum(214, 182, info.kbps, 3, 16, 0X80); 	//显示位率	 
			LCD_ShowString(214 + 48, 182, 200, 16, 16, "Kbps");
		}
		else if (show_mode == 1 && info.pic_show == 0)
		{
			if (info.size != 0)
			{
				if (info.TIT2[0] != 0 && info.TPE1[0] != 0 && info.TALB[0] != 0)
				{
					strcat((char*)info.TIT2, "-");
					strcat((char*)info.TIT2, (char*)info.TPE1);
					strcat((char*)info.TIT2, "-");
					strcat((char*)info.TIT2, (char*)info.TALB);
					Show_Str(0, 0, 320, 16, info.TIT2, 16, 0);				//显示歌曲名字 
				}
				else if (info.TIT2 != NULL && info.TIT2[0] != 0x00)
				{
					Show_Str(0, 0, 320, 16, info.TIT2, 16, 0);				//显示歌曲名字 
				}
				else
					Show_Str(0, 0, 320, 16, fn, 16, 0);				//显示歌曲名字 
			}
			else
			{
				Show_Str(0, 0, 320, 16, fn, 16, 0);				//显示歌曲名字 
			}
			LCD_ShowString(0, 256, 32, 16, 16, "VOL:");
			LCD_ShowxNum(32, 256, (vsset.mvol - 100) / 5, 2, 16, 0X80); 	//显示音量	 
			LCD_ShowxNum(0, 273, info.curindex + 1, 3, 16, 0X80);		//索引
			LCD_ShowChar(24, 273, '/', 16, 0);
			LCD_ShowxNum(32, 273, info.totmp3num, 3, 16, 0X80); 	//总曲目	

			if (info.playtime == 0)info.playtime = info.time;
			else if ((info.time != info.playtime) && (info.time != 0))//1s时间到,更新显示数据
			{
				info.playtime = info.time;//更新时间 	 			
				temp = VS_Get_HeadInfo(); //获得比特率	   				 
				if (temp != info.kbps)
				{
					info.kbps = temp;//更新KBPS	  				     
				}
				//显示播放时间			 
				LCD_ShowxNum(0, 290, info.time / 60, 2, 16, 0X80);		//分钟
				LCD_ShowChar(16, 290, ':', 16, 0);
				LCD_ShowxNum(24, 290, info.time % 60, 2, 16, 0X80);	//秒钟		
				LCD_ShowChar(40, 290, '/', 16, 0);
				//显示总时间
				if (info.kbps)info.time = (info.fmp3->fsize / info.kbps) / 125;//得到秒钟数   (文件长度(字节)/(1000/8)/比特率=持续秒钟数    	  
				else info.time = 0;//非法位率	  
				LCD_ShowxNum(48, 290, info.time / 60, 2, 16, 0X80);	//分钟
				LCD_ShowChar(64, 290, ':', 16, 0);
				LCD_ShowxNum(72, 290, info.time % 60, 2, 16, 0X80);	//秒钟	  		    
				//显示位率			   
				LCD_ShowxNum(65, 256, info.kbps, 3, 16, 0X80); 	//显示位率	 
				LCD_ShowString(65 + 24, 256, 200, 16, 16, "Kbps");
			}
		}
	}
}




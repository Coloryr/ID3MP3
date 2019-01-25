#include "mp3player.h" 
#include "malloc.h" 
#include "lcd.h" 
#include "ff.h" 
#include "vs_path.h"
#include "show.h"
#include "vs10xx.h"
#include "exfuns.h"
#include "text.h"
#include "data.h"
#include "includes.h"
#include "string.h"
#include "mp3id3.h"
#include "key.h"
#include "tjpgd.h"

mp3_info info;	//定义fft变量管理结构体

//新的频率值
const u16 VS_NEW_BANDS_FREQ_TBL[14]={80,300,800,1270,2016,3200,4500,6000,7500,9000,11000,13000,15000,20000};

void vs_reset(void)
{
	VS_Restart_Play();  					//重启播放 
	VS_Set_All();        					//设置音量等信息 			 
	VS_Reset_DecodeTime();					//复位解码时间 	  
	VS_Load_Patch((u16*)VS1053_PATCH, 1000); 	  //加载频谱分析补丁
	VS_Set_Bands((u16*)VS_NEW_BANDS_FREQ_TBL, FFT_BANDS);//重设频谱频率
}
	
//得到path路径下,目标文件的总个数
//path:路径		    
//返回值:总有效文件数
u16 mp3_get_tnum(u8 *path)
{
	u8 res;
	u16 rval = 0;
	DIR tdir;	 		//临时目录
	FILINFO tfileinfo;	//临时文件信息		
	u8 *fn;
	res = f_opendir(&tdir, (const TCHAR*)path); 	//打开目录
	tfileinfo.lfsize = _MAX_LFN * 2 + 1;				//长文件名最大长度
	tfileinfo.lfname = mymalloc(tfileinfo.lfsize);//为长文件缓存区分配内?
	if (res == FR_OK && tfileinfo.fname != NULL)
	{
		while (1)//查询总的有效文件数
		{
			res = f_readdir(&tdir, &tfileinfo);       		//读取目录下的一个文件
			if (res != FR_OK || tfileinfo.fname[0] == 0)break;	//错误了/到末尾了,退出		  
			fn = (u8*)(*tfileinfo.fname ? tfileinfo.fname : tfileinfo.fname);
			res = f_typetell(fn);
			if ((res & 0XF0) == 0X40)//取高四位,看看是不是音乐文件	
			{
				rval++;//有效文件数增加1
			}
		}
	}
	myfree(tfileinfo.fname);
	return rval;
}


//初始化频谱管理数据
void init_fft(void)
{
	u8 i;
	for (i = 0; i < FFT_BANDS; i++)	//初始化频谱管理数据
	{
		info.fft_cur[i] = 0;
		info.fft_top[i] = 80;
		info.fft_time[i] = 30;
	}
}


//显示FFT_BANDS根柱子
//mp3devx:MP3结构体
//pdt:频谱数据
void FFT_post(u16 *pbuf)
{
	u8 i = 0;
	u8 temp;
	for (i = 0; i < FFT_BANDS; i++)	//显示各个频谱	   循环显示14个段
	{

		temp = (pbuf[i] & 0X3E) * 3; 			//得到当前值,并乘2倍 主要为增加显示效果	因为输出的频率都相对较低

		if (info.fft_cur[i] < temp) 	  //当前值小于temp
			info.fft_cur[i] = temp;
		else							  //当前值大于等于temp	 开始往下降 一次降1
		{
			if (info.fft_cur[i] > 1)info.fft_cur[i] -= 1;
			else info.fft_cur[i] = 0;
		}

		if (info.fft_cur[i] > info.fft_top[i])//当前值大于峰值时 更新峰值
		{
			info.fft_top[i] = info.fft_cur[i];
			info.fft_time[i] = 30;               //重设峰值停顿时间
		}

		if (info.fft_time[i])info.fft_time[i]--;   //如果停顿时间大于1 即未减完
		else 										   //停顿时间已减没
		{
			if (info.fft_top[i]) info.fft_top[i]--;   //峰值下降1
		}


		if (info.fft_cur[i] > 79)info.fft_cur[i] = 79;	  //保证在范围内 因为前面有增倍效果
		if (info.fft_top[i] > 79)info.fft_top[i] = 79;

		fft_show_oneband(224 + i * 6, 130, 6, 80, info.fft_cur[i], info.fft_top[i]);//显示柱子	   
	}
}


void mp3_play_ready()
{
	u8 res;
	DIR mp3dir;	 		//目录
	u8 *fn;   			//长文件名 
	u16 temp;

	VS_Sine_Test();
	vs_reset();
	VS_SPI_SpeedHigh();	//高速			

	lcd_bit = 1;
	info.pic_show = 0;

	init_fft();
	Show_Str(30, 20, 240, 16, "正在读取文件", 16, 0);
	while (f_opendir(&mp3dir, "0:/MUSIC"))//打开音乐文件夹
	{
		Show_Str(30, 20, 240, 16, "MUSIC文件夹错误!", 16, 0);
		delay_ms(200);
		LCD_Fill(30, 20, 240, 226, BLACK);//清除显示	     
		delay_ms(200);
	}
	info.totmp3num = mp3_get_tnum("0:/MUSIC"); //得到总有效文件数
	while (info.totmp3num == NULL)//音乐文件总数为0		
	{
		Show_Str(30, 20, 240, 16, "没有音乐文件!", 16, 0);
		delay_ms(200);
		LCD_Fill(30, 20, 240, 226, BLACK);//清除显示	     
		delay_ms(200);
	}
	info.mp3fileinfo.lfsize = _MAX_LFN * 2 + 1;				//长文件名最大长度
	info.mp3fileinfo.lfname = mymalloc(info.mp3fileinfo.lfsize);//为长文件缓存区分配内存
	info.pname = mymalloc(info.mp3fileinfo.lfsize);				//为带路径的文件名分配内存
	info.mp3indextbl = mymalloc(2 * info.totmp3num);				//申请2*totmp3num个字节的内存,用于存放音乐文件索引
	info.fmp3 = (FIL*)mymalloc(sizeof(FIL));	//申请内存
	fmp3 = (FIL*)mymalloc(sizeof(FIL));	//申请内存
	while (info.mp3fileinfo.lfname == NULL || info.pname == NULL ||
		info.mp3indextbl == NULL || info.fmp3 == NULL)//内存分配出错
	{
		Show_Str(30, 20, 240, 16, "内存分配失败!", 16, 0);
		delay_ms(200);
		LCD_Fill(30, 20, 240, 226, BLACK);//清除显示	     
		delay_ms(200);
	}
	//记录索引
	res = f_opendir(&mp3dir, "0:/MUSIC"); //打开目录
	if (res == FR_OK)
	{
		info.curindex = 0;//当前索引为0
		while (1)//全部查询一遍
		{
			temp = mp3dir.index;								//记录当前index
			res = f_readdir(&mp3dir, &info.mp3fileinfo);       		//读取目录下的一个文件
			if (res != FR_OK || info.mp3fileinfo.fname[0] == 0)break;	//错误了/到末尾了,退出		  
			fn = (u8*)(*info.mp3fileinfo.lfname ? info.mp3fileinfo.lfname : info.mp3fileinfo.fname);
			res = f_typetell(fn);
			if ((res & 0XF0) == 0X40)//取高四位,看看是不是音乐文件	
			{
				info.mp3indextbl[info.curindex] = temp;//记录索引
				info.curindex++;
			}
		}
	}
	read_data();
	LCD_Clear(BLACK);
}

//播放音乐
void mp3_play(void *pdata)
{
	u8 res;
	DIR mp3dir;	 		//目录
	u8 *fn;   			//长文件名
	u8 key;					//键值		  
	u8 *databuf;
	u8 rval = 0;
	u16 i = 0;
	static u8 pause = 0;		//暂停标志   
	CPU_SR_ALLOC();

	databuf = (u8*)mymalloc(1024);		//开辟4096字节的内存区域
	if (databuf == NULL)rval = 0XFF;//内存申请失败.

	OS_CRITICAL_ENTER();	//进入临界区
	mp3_play_ready();

	VS_Set_Vol(vsset.mvol);
	res = f_opendir(&mp3dir, (const TCHAR*)"0:/MUSIC"); 	//打开目录
	OS_CRITICAL_EXIT();
	while (res == FR_OK)//打开成功
	{
		OS_CRITICAL_ENTER();	//进入临界区
		dir_sdi(&mp3dir, info.mp3indextbl[info.curindex]);			//改变当前目录索引	   
		res = f_readdir(&mp3dir, &info.mp3fileinfo);       		//读取目录下的一个文件
		if (res != FR_OK || info.mp3fileinfo.fname[0] == 0)break;	//错误了/到末尾了,退出
		fn = (u8*)(*info.mp3fileinfo.lfname ? info.mp3fileinfo.lfname : info.mp3fileinfo.fname);
		strcpy((char*)info.pname, "0:/MUSIC/");				//复制路径(目录)
		strcat((char*)info.pname, (const char*)fn);  			//将文件名接在后面	
		info.size = 1;
		res = f_open(info.fmp3, (const TCHAR*)info.pname, FA_READ);
		if (write_bit == 0x10)
		{
			write_bit = 0x20;
			OS_CRITICAL_EXIT();
			while (write_bit == 0x20);
			OS_CRITICAL_ENTER();
		}
		f_open(fmp3, (const TCHAR*)info.pname, FA_READ);
		if (res != FR_OK)
			while (1)
			{
				Show_Str(30, 120, 240, 16, "MUSIC文件夹错误!", 16, 0);
				delay_ms(200);
				LCD_Fill(30, 120, 240, 226, BLACK);//清除显示	     
				delay_ms(200);
			}
		mp3id3();
		f_lseek(info.fmp3, info.size);
		rval = 0;
		OS_CRITICAL_EXIT();
		while (rval == 0)
		{
			OS_CRITICAL_ENTER();	//进入临界区
			res = f_read(info.fmp3, databuf, 1024, (UINT*)&br);//读出4096个字节 			
			OS_CRITICAL_EXIT();
			i = 0;
			do//主播放循环
			{
				if ((VS_Send_MusicData(databuf + i) == 0) && (pause == 0))//给VS10XX发送音频数据
				{
					i += 32;
				}
				else
				{
					key = KEY_Scan(0);
					switch (key)
					{
					case KEY0_PRES:
						rval = KEY0_PRES;		//下一曲
						break;
					case KEY1_PRES:
						rval = KEY1_PRES;		//上一曲
						break;
					case KEY2_PRES:
						vsset.mvol = vsset.mvol + 10;
						if (vsset.mvol >= 200)
						{
							vsset.mvol = 100;
						}
						VS_Set_Vol(vsset.mvol);
						save_bit[2] = vsset.mvol;
						write_data();
						break;
					case KEY3_PRES:	   //暂停/播放
						/*if (lcd_bit == 0)
						{
							lcd_bit = 1;
							LCD_LED = 1;
						}
						else if (lcd_bit == 1)
						{
							lcd_bit = 0;
							LCD_LED = 0;
						}*/
						LCD_Clear(BLACK);
						if (show_mode == 0)
							show_mode = 1;
						else
							show_mode = 0;
						break;
					default:
						break;
					}
					show_all();
				}
			} while (i < 1024);//循环发送4096个字节 
			if (br != 1024 || res != 0)
			{
				rval = KEY0_PRES;
			}
		}
		f_close(info.fmp3);
		vs_reset();
		if (rval == KEY1_PRES)		//上一曲
		{
			if (info.curindex)info.curindex--;
			else info.curindex = info.totmp3num - 1;
			write_data();
		}
		else if (rval == KEY0_PRES)//下一曲
		{
			info.curindex++;
			if (info.curindex >= info.totmp3num)info.curindex = 0;//到末尾的时候,自动从头开始
			write_data();
		}
		else break;	//产生了错误 	 
	}
	myfree(info.mp3fileinfo.lfname);	//释放内存			    
	myfree(info.pname);				//释放内存			    
	myfree(info.mp3indextbl);			//释放内存	 
}







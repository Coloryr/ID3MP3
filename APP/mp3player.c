#include "mp3player.h"
#include "vs10xx.h"	 
#include "delay.h"
#include "key.h"
#include "lcd.h"		 
#include "malloc.h"
#include "text.h"
#include "string.h"
#include "exfuns.h"	 
#include "ff.h"   
#include "vs_path.h"
#include "show.h"
#include "mp3id3.h"
#include "piclib.h"
#include "flash.h"
#include "data.h"
#include "includes.h" 

mp3_info info;	//定义fft变量管理结构体

//新的频率值
const u16 VS_NEW_BANDS_FREQ_TBL[14]={80,300,800,1270,2016,3200,4500,6000,7500,9000,11000,13000,15000,20000};

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

		temp = (pbuf[i] & 0X3D) * 3; 			//得到当前值,并乘2倍 主要为增加显示效果	因为输出的频率都相对较低

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

void join2(u8 *a, u8 *b) {
	if (*a != 0)
	{
		while (*a != '\0') {
			a++;
		}
		if (*b != 0)
		{
			while ((*a++ = *b++) != '\0') {
				;
			}
		}
	}
}

void mp3_play_ready()
{
	u8 res;
	DIR mp3dir;	 		//目录
	u8 *fn;   			//长文件名 
	u16 temp;
  
	VS_Sine_Test();	
	lcd_bit = 1;

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
	while (info.mp3fileinfo.lfname == NULL || info.pname == NULL || info.mp3indextbl == NULL)//内存分配出错
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
}

//播放音乐
void mp3_play(void *pdata)
{
	u8 res;
	DIR mp3dir;	 		//目录
	u8 *fn;   			//长文件名
	u8 key;					//键值		  
	
	mp3_play_ready();
	info.curindex = (save_bit[0] << 8) | save_bit[1];
	vsset.mvol = save_bit[2];
	VS_Set_Vol(vsset.mvol);
	res = f_opendir(&mp3dir, (const TCHAR*)"0:/MUSIC"); 	//打开目录
	while (res == FR_OK)//打开成功
	{		
		dir_sdi(&mp3dir, info.mp3indextbl[info.curindex]);			//改变当前目录索引	   
		res = f_readdir(&mp3dir, &info.mp3fileinfo);       		//读取目录下的一个文件
		if (res != FR_OK || info.mp3fileinfo.fname[0] == 0)break;	//错误了/到末尾了,退出
		fn = (u8*)(*info.mp3fileinfo.lfname ? info.mp3fileinfo.lfname : info.mp3fileinfo.fname);
		strcpy((char*)info.pname, "0:/MUSIC/");				//复制路径(目录)
		strcat((char*)info.pname, (const char*)fn);  			//将文件名接在后面	
		info.size=1;
		//mp3id3();
		if (info.size != 0)
		{
			LCD_Fill(0, 0, 320, 16, BLACK);				//清除之前的显示
			if (info.TIT2 != NULL && info.TPE1 != NULL && info.TIT2[0] != 0x00)
			{
				strcat((char*)info.TIT2, "-");
				strcat((char*)info.TIT2, (char*)info.TPE1);
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
			LCD_Fill(0, 0, 320, 16, BLACK);				//清除之前的显示
			Show_Str(0, 0, 320, 16, fn, 16, 0);				//显示歌曲名字 
		}
		mp3_vol_show((vsset.mvol - 100) / 5);
		mp3_index_show(info.curindex + 1, info.totmp3num);
		key = mp3_play_song(); 				 		//播放这个MP3    
		if (key == KEY1_PRES)		//上一曲
		{
			if (info.curindex)info.curindex--;
			else info.curindex = info.totmp3num - 1;
			save_bit[0] = (info.curindex >> 8) & 0xff;
			save_bit[1] = info.curindex & 0xff;
			write_data();
		}
		else if (key == KEY0_PRES)//下一曲
		{
			info.curindex++;
			if (info.curindex >= info.totmp3num)info.curindex = 0;//到末尾的时候,自动从头开始
			save_bit[0] = (info.curindex >> 8) & 0xff;
			save_bit[1] = info.curindex & 0xff;
			write_data();
		}
		else break;	//产生了错误 	 
	}
	myfree(info.mp3fileinfo.lfname);	//释放内存			    
	myfree(info.pname);				//释放内存			    
	myfree(info.mp3indextbl);			//释放内存	 
}

//播放一曲指定的歌曲	
//pname:歌曲路径+名字
//返回值:0,正常播放完成
//		 1,下一曲
//       2,上一曲
//       0XFF,出现错误了
u8 mp3_play_song(void)
{
	u16 br;
	u8 res, rval;
	u8 *databuf;
	u16 i = 0;
	u8 key;
	FIL* fmp3 = 0;					//MP3文件

	static u8 pause = 0;		//暂停标志 
	rval = 0;
	fmp3 = (FIL*)mymalloc(sizeof(FIL));	//申请内存
	databuf = (u8*)mymalloc(4096);		//开辟4096字节的内存区域
	if (databuf == NULL || fmp3 == NULL)rval = 0XFF;//内存申请失败.
	if (rval == 0)
	{
		VS_Restart_Play();  					//重启播放 
		VS_Set_All();        					//设置音量等信息 			 
		VS_Reset_DecodeTime();					//复位解码时间 	  
		VS_Load_Patch((u16*)VS1053_PATCH, 1000); 	  //加载频谱分析补丁
		VS_Set_Bands((u16*)VS_NEW_BANDS_FREQ_TBL, FFT_BANDS);//重设频谱频率
		res = f_typetell(info.pname);	 	 			//得到文件后缀	 			

		if (res == 0x4c)//如果是flac,加载patch
		{
			VS_Load_Patch((u16*)vs1053b_patch, VS1053B_PATCHLEN);
		}
		res = f_open(fmp3, (const TCHAR*)info.pname, FA_READ);//打开文件
		f_lseek(fmp3, info.size);
		if (res == 0)//打开成功.
		{
			VS_SPI_SpeedHigh();	//高速						   
			while (rval == 0)
			{    
				res = f_read(fmp3, databuf, 4096, (UINT*)&br);//读出4096个字节 			
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
							mp3_vol_show((vsset.mvol - 100) / 5);	//音量限制在:100~250,显示的时候,按照公式(vol-100)/5,显示,也就是0~30   
							VS_Set_Vol(vsset.mvol);
							mp3_vol_show((vsset.mvol - 100) / 5);
							save_bit[2] = vsset.mvol;
							write_data();
							break;
						case KEY3_PRES:	   //暂停/播放
							if (lcd_bit == 0)
							{
								lcd_bit = 1;
								LCD_LED = 1;
							}
							else if (lcd_bit == 1)
							{
								lcd_bit = 0;
								LCD_LED = 0;
							}
							break;
						default:
							break;
						}
						mp3_msg_show(fmp3->fsize);//显示信息	    
					}
				} while (i < 4096);//循环发送4096个字节 
				if (br != 4096 || res != 0)
				{
					rval = KEY0_PRES;
					break;//读完了.	
				}
			}
			f_close(fmp3);
		}
		else rval = 0XFF;//出现错误	   	  
	}
	myfree(databuf);
	myfree(fmp3);
	return rval;
}





























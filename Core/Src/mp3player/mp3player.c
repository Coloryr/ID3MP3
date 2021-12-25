#include "mp3player.h"
#include "malloc.h"
#include "../lcd/lcd.h"
#include "fatfs.h"
#include "../show/show.h"
#include "../vs10xx/vs10XX.h"
#include "../font/text.h"
#include "../data/data.h"
#include "string.h"
#include "../mp3id3/mp3id3.h"
#include "../key/key.h"
#include "../jpg/tjpgd.h"
#include "stdlib.h"
#include "main.h"
#include "../adc/adc.h"
#include "../exfuns/exfuns.h"

mp3_info info; //定义变量管理结构体

void vs_reset(void)
{
	VS_Restart_Play();	   //重启播放
	VS_Reset_DecodeTime(); //复位解码时间
}

uint8_t button_check(void)
{
	uint8_t temp;
	switch (KEY_Scan(0))
	{
	case 1:
		temp = 1; //下一曲
		return temp;
	case 2:
		temp = 5; //随机
		return temp;
	case 3:
		if (vsset.mvol >= 200)
			vsset.mvol = 100;
		else
			vsset.mvol = vsset.mvol + 10;
		VS_Set_Vol(vsset.mvol);
		break;
	case 4:
//		if (lcd_bit == 0)
//		{
//			lcd_bit = 1;
//			TIM_SetCompare3(TIM3, 1000);
//		}
//		else if (lcd_bit == 1)
//		{
//			lcd_bit = 0;
//			TIM_SetCompare3(TIM3, 850);
//		}
		break;
	}
	return 0;
}

//得到path路径下,目标文件的总个数
//path:路径
//返回值:总有效文件数
uint16_t mp3_get_tnum(uint8_t *path)
{
	uint8_t res;
	uint16_t rval = 0;
	DIR tdir;		   //临时目录
	FILINFO tfileinfo; //临时文件信息
	uint8_t *fn;
	res = f_opendir(&tdir, (const TCHAR *)path); //打开目录
	tfileinfo.lfsize = _MAX_LFN * 2 + 1;		 //长文件名最大长度
	tfileinfo.lfname = malloc(tfileinfo.lfsize); //为长文件缓存区分配内?
	if (res == FR_OK && tfileinfo.fname != NULL)
	{
		while (1) //查询总的有效文件数
		{
			res = f_readdir(&tdir, &tfileinfo); //读取目录下的一个文件
			if (res != FR_OK || tfileinfo.fname[0] == 0)
				break; //错误了/到末尾了,退出
			fn = (uint8_t *)(*tfileinfo.fname ? tfileinfo.fname : tfileinfo.fname);
			res = f_typetell(fn);
			if ((res & 0XF0) == 0X40) //取高四位,看看是不是音乐文件
			{
				rval++; //有效文件数增加1
			}
		}
	}
	free(tfileinfo.lfname);
	return rval;
}

void mp3_play_ready()
{
	uint8_t res;
	DIR mp3dir;	 //目录
	uint8_t *fn; //长文件名
	uint16_t temp;

	lcd_bit = 1;
	info.pic_show = 0;

	Show_Str(30, 20, 240, 16, "正在读取文件", 16, 0);
	while (f_opendir(&mp3dir, "0:/MUSIC")) //打开音乐文件夹
	{
		Show_Str(30, 20, 240, 16, "文件夹错误!", 16, 0);
	}
	info.totmp3num = mp3_get_tnum("0:/MUSIC"); //得到总有效文件数
	while (info.totmp3num == 0)			   //音乐文件总数为0
	{
		Show_Str(30, 20, 240, 16, "没有文件!", 16, 0);
	}
	info.mp3fileinfo.lfsize = _MAX_LFN * 2 + 1;					 //长文件名最大长度
	info.mp3fileinfo.lfname = malloc(info.mp3fileinfo.lfsize); //为长文件缓存区分配内存
	info.pname = malloc(info.mp3fileinfo.lfsize);				 //为带路径的文件名分配内存
	info.mp3indextbl = malloc(2 * info.totmp3num);			 //申请2*totmp3num个字节的内存,用于存放音乐文件索引
	info.fmp3 = (FIL *)malloc(sizeof(FIL));					 //申请内存
	fmp3_pic = (FIL *)malloc(sizeof(FIL));					 //申请内存
	while (info.mp3fileinfo.lfname == NULL || info.pname == NULL ||
		   info.mp3indextbl == NULL || info.fmp3 == NULL) //内存分配出错
	{
		Show_Str(30, 20, 240, 16, "内存分配失败!", 16, 0);
	}
	//记录索引
	res = f_opendir(&mp3dir, "0:/MUSIC"); //打开目录
	if (res == FR_OK)
	{
		info.curindex = 0; //当前索引为0
		while (1)		   //全部查询一遍
		{
			temp = mp3dir.index;						 //记录当前index
			res = f_readdir(&mp3dir, &info.mp3fileinfo); //读取目录下的一个文件
			if (res != FR_OK || info.mp3fileinfo.fname[0] == 0)
				break; //错误了/到末尾了,退出
			fn = (uint8_t *)(*info.mp3fileinfo.lfname ? info.mp3fileinfo.lfname : info.mp3fileinfo.fname);
			res = f_typetell(fn);
			if ((res & 0XF0) == 0X40) //取高四位,看看是不是音乐文件
			{
				info.mp3indextbl[info.curindex] = temp; //记录索引
				info.curindex++;
			}
		}
	}
	VS_Sine_Test();
	vs_reset();
	VS_Set_Bass(10, 15, 10, 0);
	read_data();
	VS_SPI_SpeedHigh(); //高速
	LCD_Clear(BLACK);
}

//播放音乐
void mp3_play(void *pdata)
{
	uint8_t res;
	DIR mp3dir; //目录
	uint8_t *databuf;
	uint8_t rval = 0;
	uint16_t i = 0;
	static uint8_t pause = 0; //暂停标志

	databuf = (uint8_t *)malloc(MP3_BUFF_SIZE); //开辟4096字节的内存区域
	if (databuf == NULL)
		rval = 0XFF;	 //内存申请失败.
	vPortEnterCritical(); //进入临界区
	mp3_play_ready();
	res = f_opendir(&mp3dir, (const TCHAR *)"0:/MUSIC"); //打开目录
	vPortExitCritical();
	while (1) //打开成功
	{
		vPortEnterCritical(); //进入临界区
		LCD_Fill(0, 0, pic_show_size,
				 16 + pic_show_size, BACK_COLOR);
		dir_sdi(&mp3dir, info.mp3indextbl[info.curindex]); //改变当前目录索引
		res = f_readdir(&mp3dir, &info.mp3fileinfo);	   //读取目录下的一个文件
		if (res != FR_OK || info.mp3fileinfo.fname[0] == 0)
		{
			rval = KEY0_PRES; //错误了/到末尾了,退出
			vPortExitCritical();
		}
		else
		{
			info.fn = (uint8_t *)(*info.mp3fileinfo.lfname ? info.mp3fileinfo.lfname : info.mp3fileinfo.fname);
			strcpy((char *)info.pname, "0:/MUSIC/");		   //复制路径(目录)
			strcat((char *)info.pname, (const char *)info.fn); //将文件名接在后面
			info.size = 1;
			res = f_open(info.fmp3, (const TCHAR *)info.pname, FA_READ);
			f_open(fmp3_pic, (const TCHAR *)info.pname, FA_READ);
			if (res != FR_OK)
			{
				Show_Str(30, 120, 240, 16, "文夹错误!", 16, 0);
				Show_Str(0, 140, 240, 16, info.fn, 16, 0);
				rval = KEY0_PRES;
				vPortExitCritical();
			}
			else
			{
				mp3id3();
				LCD_Fill(0, 0, 240, (240 + 16 * 3) - 1, BLACK);
				show_all(1); //显示一次歌名
				show_all(3); //显示歌曲信息
				f_lseek(info.fmp3, info.size);
				rval = 0;
				vPortExitCritical();
				while (rval == 0)
				{
					vPortEnterCritical();										  //进入临界区
					res = f_read(info.fmp3, databuf, MP3_BUFF_SIZE, (UINT *)&br); //读出数据流
					vPortExitCritical();
					i = 0;
					do //主播放循环
					{
						if (info.mode == 0)
						{
							rval = button_check();
							if (rval == 8)
								pause = !pause;
							else if (rval != 0)
								break;
						}
						if ((VS_Send_MusicData(databuf + i) == 0) && (pause == 0)) //给VS10XX发送音频数据
							i += 32;
						else
						{
							vPortEnterCritical();
							show_all(3);
							vPortExitCritical();
							if (data_save_bit == 1)
								write_data();
						}
					} while (i < MP3_BUFF_SIZE); //循环发送4096个字节
					if (br != MP3_BUFF_SIZE || res != 0)
						rval = KEY0_PRES;
				}
			}
			f_close(info.fmp3);
			vs_reset();
		}
		if (write_bit == 0x00 && info.mode == 0)
		{
			write_bit = 0x10;
			while (write_bit == 0x20)
				;
		}
		switch (rval)
		{
		case KEY1_PRES:
			if (info.curindex)
				info.curindex--;
			else
				info.curindex = info.totmp3num - 1;
			LCD_Fill(0, 0, 240, 240 + 17 * 3, BLACK);
			break;
		case KEY0_PRES:
			info.curindex++;
			if (info.curindex >= info.totmp3num)
				info.curindex = 0; //到末尾的时候,自动从头开始
			LCD_Fill(0, 0, 240, 240 + 17 * 3, BLACK);
			break;
		case 5:
			srand(read_ADC());
			info.curindex = rand() % (info.totmp3num - 1);
			LCD_Fill(0, 0, 240, 240 + 17 * 3, BLACK);
			break;
		default:
			LCD_Clear(BLACK); //清屏
			Show_Str(0, 0, 240, 16, "发生错误，请复位", 16, 0);
			HardFault_Handler();
			while (1)
				;
		}
		write_data();
	}
}

#include "mp3id3.h" 
#include "exfuns.h"
#include "ff.h"
#include "malloc.h" 
#include "lcd.h"	
#include "text.h"
#include "delay.h"

u16 size;
u8 type;

//图片缓存路径
const u8 *PIC_local_jpg="0:/temp.jpg";
const u8 *PIC_local_png="0:/temp.png";
	
u8 mp3id3_head(u8 *a)
{
	if (a[0] == 0x49 && a[1] == 0x44 && a[2] == 0x33)
	{
		//计算大小
		size = a[6] & 0x7f | ((a[7] & 0x7f) << 7) | ((a[8] & 0x7f) << 14) | ((a[9] & 0x7f) << 21);
		return 0;
	}
	else return 1;
}

u16 mp3id3_is(const TCHAR* path)
{
	FIL* fmp3 = 0;
	FIL* f_test = 0;
	u16 i = 0;
	u16 br = 0;
	u8 res = 0;
	u8 *databuf = 0;
	u8 *fbuf = 0;
	u16 img = 1;
	u8 *img_path = 0;

	fmp3 = (FIL*)mymalloc(sizeof(FIL));	//申请内存
	databuf = (u8*)mymalloc(READ_buff_size);
	f_test = (FIL*)mymalloc(sizeof(FIL));
	fbuf = (u8*)mymalloc(WRITE_buff_size);
	if (databuf == NULL || fmp3 == NULL || fbuf == NULL || f_test == NULL)//内存申请失败.
		while (1)
		{
			Show_Str(30, 20, 160, 16, "MP3ID3：内存申请失败", 16, 0);
			delay_ms(200);
			LCD_Fill(30, 20, 160, 16, BLACK);//清除显示	     
			delay_ms(200);
		}
	res = f_open(fmp3, path, FA_READ | FA_OPEN_EXISTING);//打开文件
	if (res != 0)
	{
		while (1)
		{
			Show_Str(30, 20, 160, 16, "MP3ID3：文件读取错误", 16, 0);
			delay_ms(200);
			LCD_Fill(30, 20, 160, 16, BLACK);//清除显示	     
			delay_ms(200);
		}
	}
	res = f_read(fmp3, databuf, 10, (UINT*)&br);//读出mp3id3头
	if (mp3id3_head(databuf) == 0)
	{
		res = f_read(fmp3, databuf, READ_buff_size, (UINT*)&br);
		i = 0;
		while (img)
		{
			if (databuf[i] == 0x41 && databuf[i + 1] == 0x50 && databuf[i + 2] == 0x49 && databuf[i + 3] == 0x43)
			{	//找到位置
				if (databuf[i + 24] == 0xff && databuf[i + 25] == 0xd8 && databuf[i + 26] == 0xff && databuf[i + 27] == 0xe0)
				{
					type = 0; //JPG
				}
				if (databuf[i + 24] == 0x89 && databuf[i + 25] == 0x50 && databuf[i + 26] == 0x4e && databuf[i + 27] == 0x47)
				{
					type = 1; //PNG
				}
				img = 0;
			}
			else
				i++;
			if (i >= 4096)								//找不到位置
			{
				f_close(fmp3);
				myfree(f_test);
				myfree(fmp3);
				myfree(databuf);				//释放内存			    
				return size;
			}
		}
		i += 14 + 20;								//跳过头
		f_lseek(fmp3, i);
		if (type == 0)
			img_path = (u8*)PIC_local_jpg;
		if (type == 1)
			img_path = (u8*)PIC_local_png;
		res = f_unlink((const TCHAR*)img_path);
		res = f_open(f_test, (const TCHAR*)img_path, FA_READ | FA_WRITE);//尝试打开之前的文件
		if (res == 0x04)res = f_open(f_test, (const TCHAR*)img_path, FA_CREATE_NEW | FA_WRITE);//创建文件
		if (res != FR_OK)						//创建失败
		{
			f_close(fmp3);
			f_close(f_test);
			myfree(f_test);
			myfree(fmp3);
			myfree(databuf);				//释放内存			    
			return size;
		}
		img = 0;
		while (res == 0)						//读出图片文件
		{
			res = f_read(fmp3, fbuf, WRITE_buff_size, (UINT*)&br);
			if (res || br == 0)
			{
				type = 2;
				break;
			}
			res = f_write(f_test, fbuf, (UINT)br, (UINT*)&bw);
			if (res || bw < br)
			{
				type = 2;
				break;
			}
			img++;
			if (img > out_size)
			{
				break;
			}
		}
		f_close(fmp3);
		f_close(f_test);
		myfree(f_test);
		myfree(fmp3);
		myfree(databuf);				//释放内存			    
		return size;
	}
	type = 2;
	f_close(fmp3);
	myfree(f_test);
	myfree(fmp3);
	myfree(databuf);						//释放内存			     
	return 0;
}




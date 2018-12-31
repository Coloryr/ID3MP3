#include "mp3id3.h" 
#include "exfuns.h"
#include "ff.h"
#include "malloc.h" 
#include "lcd.h"	
#include "text.h"
#include "delay.h"

u16 size;
	u8 pic[];
	
u8 mp3id3_head(u8 *a)
{
	if (a[0] == 0x49 && a[1] == 0x44 && a[2] == 0x33)
	{
		size = a[6] & 0x7f | ((a[7] & 0x7f) << 7) | ((a[8] & 0x7f) << 14) | ((a[9] & 0x7f) << 21);
		return 0;
	}
	else return 1;
}

u16 mp3id3_is(const TCHAR* path)
{
	FIL* fmp3;
	FIL* f_test;
	u8 *fname = "test.png";
	u16 i;
	u8 a;
	u16 br;
	u8 res;
	u8 *databuf;

	u16 frmSize = 0;
	u8 buffer[10];
	u8 img = 1;
	u16 frmSize1 = 0;
	fmp3 = (FIL*)mymalloc(sizeof(FIL));	//申请内存
	databuf = (u8*)mymalloc(8192);		//开辟4096字节的内存区域
	f_test = (FIL*)mymalloc(sizeof(FIL));
	if (databuf == NULL || fmp3 == NULL)//内存申请失败.
		while (1)
		{
			Show_Str(30, 20, 160, 16, "MP3ID3：内存申请失败", 16, 0);
			delay_ms(200);
			LCD_Fill(30, 20, 160, 16, BLACK);//清除显示	     
			delay_ms(200);
		}
	res = f_open(fmp3, path, FA_READ);//打开文件
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
	res = f_read(fmp3, databuf, 10, (UINT*)&br);//读出10个字节 
	if (mp3id3_head(databuf) == 0)
	{
		res = f_read(fmp3, databuf, 8192, (UINT*)&br);//读出10个字节 
		i = 0;
		while (img)
		{
			if (i <= 8188)
			{
				if (databuf[i] == 0x41 && databuf[i + 1] == 0x50 && databuf[i + 2] == 0x49 && databuf[i + 3] == 0x43)
				{
					frmSize = databuf[i + 4] & 0x7f | ((databuf[i + 5] & 0x7f) << 7) | ((databuf[i + 6] & 0x7f) << 14) | ((databuf[i + 7] & 0x7f) << 21);
					img = 0;
				}
				else
					i++;
			}
			else
			{
				buffer[0] = databuf[i];
				buffer[1] = databuf[i + 1];
				buffer[2] = databuf[i + 2];
				buffer[3] = databuf[i + 3];
				res = f_read(fmp3, databuf, 8192, (UINT*)&br);//读出10个字节 
				i = 0;
				buffer[4] = databuf[i];
				buffer[5] = databuf[i + 1];
				buffer[6] = databuf[i + 2];
				buffer[7] = databuf[i + 3];
				for (a = 0; a < 4; a++)
				{
					if (buffer[a] == 0x41 && buffer[a + 1] == 0x50 && buffer[a + 2] == 0x49 && buffer[a + 3] == 0x43)
						img = 1;
				}
				i = 3;
			}
		}
		if (frmSize != 0)
		{
			if (i == 8190)
			{
				res = f_read(fmp3, databuf, 8192, (UINT*)&br);//读出10个字节 
				i = 0;
			}
			else if (i == 8191)
			{
				res = f_read(fmp3, databuf, 1, (UINT*)&br);//读出10个字节 
				res = f_read(fmp3, databuf, 8192, (UINT*)&br);//读出10个字节 
				i = 0;
			}
			else
			{
				i += 2;
			}
			i += 14 + 8;
			frmSize -= 14;
			frmSize1 = frmSize;
			while (frmSize)
			{
				a = 0;
				for (; i < 8192; i++)
				{
					pic[a] = databuf[i];
					a++;
					frmSize--;
				}
				while (frmSize > 8192)
				{
					res = f_read(fmp3, databuf, 8192, (UINT*)&br);//读出10个字节 
					frmSize -= 8192;
					for (i = 0; i < 8192; i++)
					{
						pic[a] = databuf[i];
						a++;
					}
				}
				if (frmSize > 0 && frmSize < 8192)
				{
					res = f_read(fmp3, databuf, frmSize, (UINT*)&br);//读出10个字节 
					for (i = 0; i < frmSize; i++)
					{
						pic[a] = databuf[i];
						a++;
					}
					frmSize = 0;
				}
				f_close(fmp3);
			}
			res = f_open(f_test, (const TCHAR*)fname, FA_READ | FA_WRITE);//尝试打开之前的文件
			if (res == 0x04)res = f_open(f_test, (const TCHAR*)fname, FA_CREATE_NEW | FA_WRITE);//创建文件
			if (res == FR_OK)//创建成功
			{
				res = f_write(f_test, (u8*)pic, frmSize1, &bw);
				f_close(f_test);
			}
			f_close(fmp3);
			f_close(f_test);
			myfree(f_test);
			myfree(fmp3);	//释放内存			    
			myfree(databuf);				//释放内存			    
			return size;
		}
		else
		{
			myfree(f_test);
			myfree(fmp3);	//释放内存			    
			myfree(databuf);				//释放内存			    
			return 1;
		}
	}
	else
	{
		myfree(f_test);
		myfree(fmp3);	//释放内存			    
		myfree(databuf);				//释放内存			    
		return 1;
	}
	myfree(f_test);
	myfree(fmp3);	//释放内存			    
	myfree(databuf);				//释放内存			     
	return 1;
}




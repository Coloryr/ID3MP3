#include "mp3id3.h" 
#include "exfuns.h"
#include "ff.h"
#include "malloc.h" 
#include "lcd.h"	
#include "text.h"
#include "delay.h"

u16 size;

u8 mp3id3_head(u8 *a)
{
	if(a[0]==0x49 && a[1]==0x44 &&a[2]==0x33)
	{
		size = a[6] & 0x7f | ((a[7] & 0x7f) << 7) | ((a[8] & 0x7f) << 14) | ((a[9] & 0x7f) << 21);
		size-=10;
		return 0;
	}
	else return 1;
}

u8 mp3id3_is(const TCHAR* path)
{
	FIL* fmp3;
	u16 i;
	u8 a;
	u16 br;
	u8 res;	  
	u8 *databuf;	 
	u16 frmSize;
	u8 buffer[10];
	u8 img;
	fmp3=(FIL*)mymalloc(sizeof(FIL));	//申请内存
	databuf=(u8*)mymalloc(8192);		//开辟4096字节的内存区域
		if(databuf==NULL||fmp3==NULL)//内存申请失败.
		while(1)
			{
				Show_Str(30,20,160,16,"MP3ID3：内存申请失败",16,0);
				delay_ms(200);				  
				LCD_Fill(30,20,160,16,BLACK);//清除显示	     
				delay_ms(200);				  
			}	
		res=f_open(fmp3,path,FA_READ);//打开文件
		if(res!=0)
		{
			while(1)
			{
				Show_Str(30,20,160,16,"MP3ID3：文件读取错误",16,0);
				delay_ms(200);				  
				LCD_Fill(30,20,160,16,BLACK);//清除显示	     
				delay_ms(200);				  
			}
		}
		res=f_read(fmp3,databuf,10,(UINT*)&br);//读出10个字节 
		if(mp3id3_head(databuf)==0)
		{
			res=f_read(fmp3,databuf,8192,(UINT*)&br);//读出10个字节 
			i=0;
			while(img)
			{
				if(i<=8188)
				{
					if(databuf[i]==0x41&&databuf[i+1]==0x50&&databuf[i+2]==0x49&&databuf[i+3]==0x43)
					{
						frmSize = databuf[i+4] & 0x7f | ((databuf[i+7] & 0x7f) << 7) | ((databuf[i+8] & 0x7f) << 14) | ((databuf[i+9] & 0x7f) << 21);
					}
					else
						i++;
			  }
				else
				{
					buffer[0]=databuf[i];
					buffer[1]=databuf[i+1];
					buffer[2]=databuf[i+2];
					buffer[3]=databuf[i+3];
					res=f_read(fmp3,databuf,8192,(UINT*)&br);//读出10个字节 
					i=0;
					buffer[4]=databuf[i];
					buffer[5]=databuf[i+1];
					buffer[6]=databuf[i+2];
					buffer[7]=databuf[i+3];
					for(a=0;a<4;a++)
					{
					if(buffer[a]==0x41&&buffer[a+1]==0x50&&buffer[a+2]==0x49&&buffer[a+3]==0x43)
					img=1;
			  	}
					i=3;
				}
			}
			return 0;	
		}
		else
		{
			return 1;	
		}
			
	return 1;
}




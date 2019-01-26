#include "mp3player.h" 
#include "malloc.h" 
#include "lcd.h" 
#include "ff.h" 
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
#include "app_start.h" 
#include "stdlib.h"

mp3_info info;	//�����������ṹ��

void vs_reset(void)
{
	VS_Restart_Play();  					//�������� 
	VS_Set_All();        					//������������Ϣ 			 
	VS_Reset_DecodeTime();					//��λ����ʱ�� 	 
}
	
//�õ�path·����,Ŀ���ļ����ܸ���
//path:·��		    
//����ֵ:����Ч�ļ���
u16 mp3_get_tnum(u8 *path)
{
	u8 res;
	u16 rval = 0;
	DIR tdir;	 		//��ʱĿ¼
	FILINFO tfileinfo;	//��ʱ�ļ���Ϣ		
	u8 *fn;
	res = f_opendir(&tdir, (const TCHAR*)path); 	//��Ŀ¼
	tfileinfo.lfsize = _MAX_LFN * 2 + 1;				//���ļ�����󳤶�
	tfileinfo.lfname = mymalloc(tfileinfo.lfsize);//Ϊ���ļ�������������?
	if (res == FR_OK && tfileinfo.fname != NULL)
	{
		while (1)//��ѯ�ܵ���Ч�ļ���
		{
			res = f_readdir(&tdir, &tfileinfo);       		//��ȡĿ¼�µ�һ���ļ�
			if (res != FR_OK || tfileinfo.fname[0] == 0)break;	//������/��ĩβ��,�˳�		  
			fn = (u8*)(*tfileinfo.fname ? tfileinfo.fname : tfileinfo.fname);
			res = f_typetell(fn);
			if ((res & 0XF0) == 0X40)//ȡ����λ,�����ǲ��������ļ�	
			{
				rval++;//��Ч�ļ�������1
			}
		}
	}
	myfree(tfileinfo.fname);
	return rval;
}

void mp3_play_ready()
{
	u8 res;
	DIR mp3dir;	 		//Ŀ¼
	u8 *fn;   			//���ļ��� 
	u16 temp;

	VS_Sine_Test();
	vs_reset();
	VS_SPI_SpeedHigh();	//����			

	lcd_bit = 1;
	info.pic_show = 0;

	Show_Str(30, 20, 240, 16, "���ڶ�ȡ�ļ�", 16, 0);
	while (f_opendir(&mp3dir, "0:/MUSIC"))//�������ļ���
	{
		Show_Str(30, 20, 240, 16, "�ļ��д���!", 16, 0);
	}
	info.totmp3num = mp3_get_tnum("0:/MUSIC"); //�õ�����Ч�ļ���
	while (info.totmp3num == NULL)//�����ļ�����Ϊ0		
	{
		Show_Str(30, 20, 240, 16, "û���ļ�!", 16, 0);
	}
	info.mp3fileinfo.lfsize = _MAX_LFN * 2 + 1;				//���ļ�����󳤶�
	info.mp3fileinfo.lfname = mymalloc(info.mp3fileinfo.lfsize);//Ϊ���ļ������������ڴ�
	info.pname = mymalloc(info.mp3fileinfo.lfsize);				//Ϊ��·�����ļ��������ڴ�
	info.mp3indextbl = mymalloc(2 * info.totmp3num);				//����2*totmp3num���ֽڵ��ڴ�,���ڴ�������ļ�����
	info.fmp3 = (FIL*)mymalloc(sizeof(FIL));	//�����ڴ�
	fmp3 = (FIL*)mymalloc(sizeof(FIL));	//�����ڴ�
	while (info.mp3fileinfo.lfname == NULL || info.pname == NULL ||
		info.mp3indextbl == NULL || info.fmp3 == NULL)//�ڴ�������
	{
		Show_Str(30, 20, 240, 16, "�ڴ����ʧ��!", 16, 0);
	}
	//��¼����
	res = f_opendir(&mp3dir, "0:/MUSIC"); //��Ŀ¼
	if (res == FR_OK)
	{
		info.curindex = 0;//��ǰ����Ϊ0
		while (1)//ȫ����ѯһ��
		{
			temp = mp3dir.index;								//��¼��ǰindex
			res = f_readdir(&mp3dir, &info.mp3fileinfo);       		//��ȡĿ¼�µ�һ���ļ�
			if (res != FR_OK || info.mp3fileinfo.fname[0] == 0)break;	//������/��ĩβ��,�˳�		  
			fn = (u8*)(*info.mp3fileinfo.lfname ? info.mp3fileinfo.lfname : info.mp3fileinfo.fname);
			res = f_typetell(fn);
			if ((res & 0XF0) == 0X40)//ȡ����λ,�����ǲ��������ļ�	
			{
				info.mp3indextbl[info.curindex] = temp;//��¼����
				info.curindex++;
			}
		}
	}
	read_data();
	LCD_Clear(BLACK);
}

//��������
void mp3_play(void *pdata)
{
	u8 res;
	DIR mp3dir;	 		//Ŀ¼
	u8 *fn;   			//���ļ���	  
	u8 *databuf;
	u8 rval = 0;
	u16 i = 0;
	static u8 pause = 0;		//��ͣ��־   
	CPU_SR_ALLOC();

	databuf = (u8*)mymalloc(MP3_BUFF_SIZE);		//����4096�ֽڵ��ڴ�����
	if (databuf == NULL)rval = 0XFF;//�ڴ�����ʧ��.
	OS_CRITICAL_ENTER();	//�����ٽ���
	mp3_play_ready();
	VS_Set_Vol(vsset.mvol);
	res = f_opendir(&mp3dir, (const TCHAR*)"0:/MUSIC"); 	//��Ŀ¼
	OS_CRITICAL_EXIT();
	while (res == FR_OK)//�򿪳ɹ�
	{
		OS_CRITICAL_ENTER();	//�����ٽ���
		LCD_Fill(0, 0, pic_show_size,
				16 + pic_show_size, BACK_COLOR);
		dir_sdi(&mp3dir, info.mp3indextbl[info.curindex]);			//�ı䵱ǰĿ¼����	   
		res = f_readdir(&mp3dir, &info.mp3fileinfo);       		//��ȡĿ¼�µ�һ���ļ�
		if (res != FR_OK || info.mp3fileinfo.fname[0] == 0)break;	//������/��ĩβ��,�˳�
		fn = (u8*)(*info.mp3fileinfo.lfname ? info.mp3fileinfo.lfname : info.mp3fileinfo.fname);
		strcpy((char*)info.pname, "0:/MUSIC/");				//����·��(Ŀ¼)
		strcat((char*)info.pname, (const char*)fn);  			//���ļ������ں���	
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
				Show_Str(30, 120, 240, 16, "�ļ��д���!", 16, 0);
			}
		mp3id3();
		f_lseek(info.fmp3, info.size);
		rval = 0;
		OS_CRITICAL_EXIT();
		while (rval == 0)
		{
			OS_CRITICAL_ENTER();	//�����ٽ���
			res = f_read(info.fmp3, databuf, MP3_BUFF_SIZE, (UINT*)&br);//����4096���ֽ� 			
			OS_CRITICAL_EXIT();
			i = 0;
			do//������ѭ��
			{
				if ((VS_Send_MusicData(databuf + i) == 0) && (pause == 0))//��VS10XX������Ƶ����
				{
					i += 32;
				}
				else
				{
					button_check();
					switch (key_now)
					{
					case KEY0_PRES:
						rval = KEY0_PRES;		//��һ��
						key_now = 0;
						break;
					case KEY1_PRES:
						rval = KEY1_PRES;		//��һ��
						key_now = 0;
						break;
					case KEY2_PRES:
						rval = 5;						//���					
						key_now = 0;
						break;
					case KEY3_PRES:	   //��ͣ/����
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
						key_now = 0;
						break;
					case 5:
						vsset.mvol = vsset.mvol + 10;
						if (vsset.mvol >= 200)
						{
							vsset.mvol = 100;
						}
						VS_Set_Vol(vsset.mvol);
						save_bit[2] = vsset.mvol;
						write_data();
						key_now = 0;
						break;
					case 6:
						vsset.mvol = vsset.mvol - 10;
						if (vsset.mvol < 100)
						{
							vsset.mvol = 200;
						}
						VS_Set_Vol(vsset.mvol);
						save_bit[2] = vsset.mvol;
						write_data();
						key_now = 0;
						break;
					default:
						break;
					}
					show_all(0);
				}
			} while (i < MP3_BUFF_SIZE);//ѭ������4096���ֽ� 
			if (br != MP3_BUFF_SIZE || res != 0)
			{
				rval = KEY0_PRES;
			}
		}
		f_close(info.fmp3);
		vs_reset();
		if (rval == KEY1_PRES)		//��һ��
		{
			if (info.curindex)info.curindex--;
			else info.curindex = info.totmp3num - 1;
			write_data();
			LCD_Fill(0, 0, 240, 240 + 17 * 3, BLACK);
		}
		else if (rval == KEY0_PRES)//��һ��
		{
			info.curindex++;
			if (info.curindex >= info.totmp3num)info.curindex = 0;//��ĩβ��ʱ��,�Զ���ͷ��ʼ
			write_data();
			LCD_Fill(0, 0, 240, 240 + 17 * 3, BLACK);
		}
		else if(rval == 5)
		{
			srand(app_get_rand(info.curindex));
			info.curindex = rand() % (info.totmp3num - 1); 
			write_data();
			LCD_Fill(0, 0, 240, 240 + 17 * 3, BLACK);
		}
		else 
		{
			LCD_Clear(BLACK);//����  
			Show_Str(0, 0, 240, 16, "���������븴λ", 16, 0);
			HardFault_Handler();
			while(1);
		}
	}
	myfree(info.mp3fileinfo.lfname);	//�ͷ��ڴ�			    
	myfree(info.pname);								//�ͷ��ڴ�			    
	myfree(info.mp3indextbl);					//�ͷ��ڴ�	 
}







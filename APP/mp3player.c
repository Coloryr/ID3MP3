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

mp3_info info;	//����fft��������ṹ��

//�µ�Ƶ��ֵ
const u16 VS_NEW_BANDS_FREQ_TBL[14]={80,300,800,1270,2016,3200,4500,6000,7500,9000,11000,13000,15000,20000};

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
//��ʼ��Ƶ�׹�������
void init_fft(void)
{
	u8 i;
	for (i = 0; i < FFT_BANDS; i++)	//��ʼ��Ƶ�׹�������
	{
		info.fft_cur[i] = 0;
		info.fft_top[i] = 80;
		info.fft_time[i] = 30;
	}
}
//��ʾFFT_BANDS������
//mp3devx:MP3�ṹ��
//pdt:Ƶ������
void FFT_post(u16 *pbuf)
{
	u8 i = 0;
	u8 temp;
	for (i = 0; i < FFT_BANDS; i++)	//��ʾ����Ƶ��	   ѭ����ʾ14����
	{

		temp = (pbuf[i] & 0X3D) * 3; 			//�õ���ǰֵ,����2�� ��ҪΪ������ʾЧ��	��Ϊ�����Ƶ�ʶ���Խϵ�

		if (info.fft_cur[i] < temp) 	  //��ǰֵС��temp
			info.fft_cur[i] = temp;
		else							  //��ǰֵ���ڵ���temp	 ��ʼ���½� һ�ν�1
		{
			if (info.fft_cur[i] > 1)info.fft_cur[i] -= 1;
			else info.fft_cur[i] = 0;
		}

		if (info.fft_cur[i] > info.fft_top[i])//��ǰֵ���ڷ�ֵʱ ���·�ֵ
		{
			info.fft_top[i] = info.fft_cur[i];
			info.fft_time[i] = 30;               //�����ֵͣ��ʱ��
		}

		if (info.fft_time[i])info.fft_time[i]--;   //���ͣ��ʱ�����1 ��δ����
		else 										   //ͣ��ʱ���Ѽ�û
		{
			if (info.fft_top[i]) info.fft_top[i]--;   //��ֵ�½�1
		}


		if (info.fft_cur[i] > 79)info.fft_cur[i] = 79;	  //��֤�ڷ�Χ�� ��Ϊǰ��������Ч��
		if (info.fft_top[i] > 79)info.fft_top[i] = 79;

		fft_show_oneband(224 + i * 6, 130, 6, 80, info.fft_cur[i], info.fft_top[i]);//��ʾ����	   
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
	DIR mp3dir;	 		//Ŀ¼
	u8 *fn;   			//���ļ��� 
	u16 temp;
  
	VS_Sine_Test();	
	lcd_bit = 1;

	init_fft();
	Show_Str(30, 20, 240, 16, "���ڶ�ȡ�ļ�", 16, 0);
	while (f_opendir(&mp3dir, "0:/MUSIC"))//�������ļ���
	{
		Show_Str(30, 20, 240, 16, "MUSIC�ļ��д���!", 16, 0);
		delay_ms(200);
		LCD_Fill(30, 20, 240, 226, BLACK);//�����ʾ	     
		delay_ms(200);
	}
	info.totmp3num = mp3_get_tnum("0:/MUSIC"); //�õ�����Ч�ļ���
	while (info.totmp3num == NULL)//�����ļ�����Ϊ0		
	{
		Show_Str(30, 20, 240, 16, "û�������ļ�!", 16, 0);
		delay_ms(200);
		LCD_Fill(30, 20, 240, 226, BLACK);//�����ʾ	     
		delay_ms(200);
	}
	info.mp3fileinfo.lfsize = _MAX_LFN * 2 + 1;				//���ļ�����󳤶�
	info.mp3fileinfo.lfname = mymalloc(info.mp3fileinfo.lfsize);//Ϊ���ļ������������ڴ�
	info.pname = mymalloc(info.mp3fileinfo.lfsize);				//Ϊ��·�����ļ��������ڴ�
	info.mp3indextbl = mymalloc(2 * info.totmp3num);				//����2*totmp3num���ֽڵ��ڴ�,���ڴ�������ļ�����
	while (info.mp3fileinfo.lfname == NULL || info.pname == NULL || info.mp3indextbl == NULL)//�ڴ�������
	{
		Show_Str(30, 20, 240, 16, "�ڴ����ʧ��!", 16, 0);
		delay_ms(200);
		LCD_Fill(30, 20, 240, 226, BLACK);//�����ʾ	     
		delay_ms(200);
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
}

//��������
void mp3_play(void *pdata)
{
	u8 res;
	DIR mp3dir;	 		//Ŀ¼
	u8 *fn;   			//���ļ���
	u8 key;					//��ֵ		  
	
	mp3_play_ready();
	info.curindex = (save_bit[0] << 8) | save_bit[1];
	vsset.mvol = save_bit[2];
	VS_Set_Vol(vsset.mvol);
	res = f_opendir(&mp3dir, (const TCHAR*)"0:/MUSIC"); 	//��Ŀ¼
	while (res == FR_OK)//�򿪳ɹ�
	{		
		dir_sdi(&mp3dir, info.mp3indextbl[info.curindex]);			//�ı䵱ǰĿ¼����	   
		res = f_readdir(&mp3dir, &info.mp3fileinfo);       		//��ȡĿ¼�µ�һ���ļ�
		if (res != FR_OK || info.mp3fileinfo.fname[0] == 0)break;	//������/��ĩβ��,�˳�
		fn = (u8*)(*info.mp3fileinfo.lfname ? info.mp3fileinfo.lfname : info.mp3fileinfo.fname);
		strcpy((char*)info.pname, "0:/MUSIC/");				//����·��(Ŀ¼)
		strcat((char*)info.pname, (const char*)fn);  			//���ļ������ں���	
		info.size=1;
		//mp3id3();
		if (info.size != 0)
		{
			LCD_Fill(0, 0, 320, 16, BLACK);				//���֮ǰ����ʾ
			if (info.TIT2 != NULL && info.TPE1 != NULL && info.TIT2[0] != 0x00)
			{
				strcat((char*)info.TIT2, "-");
				strcat((char*)info.TIT2, (char*)info.TPE1);
				Show_Str(0, 0, 320, 16, info.TIT2, 16, 0);				//��ʾ�������� 
			}
			else if (info.TIT2 != NULL && info.TIT2[0] != 0x00)
			{
				Show_Str(0, 0, 320, 16, info.TIT2, 16, 0);				//��ʾ�������� 
			}
			else
				Show_Str(0, 0, 320, 16, fn, 16, 0);				//��ʾ�������� 
		}
		else
		{
			LCD_Fill(0, 0, 320, 16, BLACK);				//���֮ǰ����ʾ
			Show_Str(0, 0, 320, 16, fn, 16, 0);				//��ʾ�������� 
		}
		mp3_vol_show((vsset.mvol - 100) / 5);
		mp3_index_show(info.curindex + 1, info.totmp3num);
		key = mp3_play_song(); 				 		//�������MP3    
		if (key == KEY1_PRES)		//��һ��
		{
			if (info.curindex)info.curindex--;
			else info.curindex = info.totmp3num - 1;
			save_bit[0] = (info.curindex >> 8) & 0xff;
			save_bit[1] = info.curindex & 0xff;
			write_data();
		}
		else if (key == KEY0_PRES)//��һ��
		{
			info.curindex++;
			if (info.curindex >= info.totmp3num)info.curindex = 0;//��ĩβ��ʱ��,�Զ���ͷ��ʼ
			save_bit[0] = (info.curindex >> 8) & 0xff;
			save_bit[1] = info.curindex & 0xff;
			write_data();
		}
		else break;	//�����˴��� 	 
	}
	myfree(info.mp3fileinfo.lfname);	//�ͷ��ڴ�			    
	myfree(info.pname);				//�ͷ��ڴ�			    
	myfree(info.mp3indextbl);			//�ͷ��ڴ�	 
}

//����һ��ָ���ĸ���	
//pname:����·��+����
//����ֵ:0,�����������
//		 1,��һ��
//       2,��һ��
//       0XFF,���ִ�����
u8 mp3_play_song(void)
{
	u16 br;
	u8 res, rval;
	u8 *databuf;
	u16 i = 0;
	u8 key;
	FIL* fmp3 = 0;					//MP3�ļ�

	static u8 pause = 0;		//��ͣ��־ 
	rval = 0;
	fmp3 = (FIL*)mymalloc(sizeof(FIL));	//�����ڴ�
	databuf = (u8*)mymalloc(4096);		//����4096�ֽڵ��ڴ�����
	if (databuf == NULL || fmp3 == NULL)rval = 0XFF;//�ڴ�����ʧ��.
	if (rval == 0)
	{
		VS_Restart_Play();  					//�������� 
		VS_Set_All();        					//������������Ϣ 			 
		VS_Reset_DecodeTime();					//��λ����ʱ�� 	  
		VS_Load_Patch((u16*)VS1053_PATCH, 1000); 	  //����Ƶ�׷�������
		VS_Set_Bands((u16*)VS_NEW_BANDS_FREQ_TBL, FFT_BANDS);//����Ƶ��Ƶ��
		res = f_typetell(info.pname);	 	 			//�õ��ļ���׺	 			

		if (res == 0x4c)//�����flac,����patch
		{
			VS_Load_Patch((u16*)vs1053b_patch, VS1053B_PATCHLEN);
		}
		res = f_open(fmp3, (const TCHAR*)info.pname, FA_READ);//���ļ�
		f_lseek(fmp3, info.size);
		if (res == 0)//�򿪳ɹ�.
		{
			VS_SPI_SpeedHigh();	//����						   
			while (rval == 0)
			{    
				res = f_read(fmp3, databuf, 4096, (UINT*)&br);//����4096���ֽ� 			
				i = 0;
				do//������ѭ��
				{
					if ((VS_Send_MusicData(databuf + i) == 0) && (pause == 0))//��VS10XX������Ƶ����
					{
						i += 32;
					}
					else
					{
						key = KEY_Scan(0);
						switch (key)
						{
						case KEY0_PRES:
							rval = KEY0_PRES;		//��һ��
							break;
						case KEY1_PRES:
							rval = KEY1_PRES;		//��һ��
							break;
						case KEY2_PRES:
							vsset.mvol = vsset.mvol + 10;
							if (vsset.mvol >= 200)
							{
								vsset.mvol = 100;
							}
							mp3_vol_show((vsset.mvol - 100) / 5);	//����������:100~250,��ʾ��ʱ��,���չ�ʽ(vol-100)/5,��ʾ,Ҳ����0~30   
							VS_Set_Vol(vsset.mvol);
							mp3_vol_show((vsset.mvol - 100) / 5);
							save_bit[2] = vsset.mvol;
							write_data();
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
							break;
						default:
							break;
						}
						mp3_msg_show(fmp3->fsize);//��ʾ��Ϣ	    
					}
				} while (i < 4096);//ѭ������4096���ֽ� 
				if (br != 4096 || res != 0)
				{
					rval = KEY0_PRES;
					break;//������.	
				}
			}
			f_close(fmp3);
		}
		else rval = 0XFF;//���ִ���	   	  
	}
	myfree(databuf);
	myfree(fmp3);
	return rval;
}





























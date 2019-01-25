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

/*��ʾ��Ф-----------------------------------------------------*/
void displaysx(void) {
	unsigned char LunarYearD, ReYear, tiangan, dizhi;        //?????????  ??????? 
	   //?????,??????,?c_moon????,year_moon???,month_moon,day_moon????BCD??
	LunarYearD = (moon.year_moon / 16) * 10 + moon.year_moon % 16;        //??????10??? 
	ReYear = LunarYearD % 12;                             //????12,????
	tiangan = LunarYearD % 10;         //??  ???.....
	dizhi = LunarYearD % 12;         //??  ???.....

	switch (tiangan)	//���
	{
	case 0: Show_Str(10, 130, 24, 24, "��", 24, 0); break;   //��0������ ���  ��
	case 1: Show_Str(10, 130, 24, 24, "��", 24, 0); break;   //
	case 2: Show_Str(10, 130, 24, 24, "��", 24, 0); break;
	case 3: Show_Str(10, 130, 24, 24, "��", 24, 0); break;
	case 4: Show_Str(10, 130, 24, 24, "��", 24, 0); break;
	case 5: Show_Str(10, 130, 24, 24, "��", 24, 0); break;
	case 6: Show_Str(10, 130, 24, 24, "��", 24, 0); break;
	case 7: Show_Str(10, 130, 24, 24, "��", 24, 0); break;
	case 8: Show_Str(10, 130, 24, 24, "��", 24, 0); break;
	case 9: Show_Str(10, 130, 24, 24, "��", 24, 0); break;
	}

	switch (dizhi) //��֧
	{
	case 0: Show_Str(34, 130, 24, 24, "��", 24, 0); break;   //��0������ ��֧ ��
	case 1: Show_Str(34, 130, 24, 24, "��", 24, 0); break;   //
	case 2: Show_Str(34, 130, 24, 24, "��", 24, 0); break;
	case 3: Show_Str(34, 130, 24, 24, "δ", 24, 0); break;
	case 4: Show_Str(34, 130, 24, 24, "��", 24, 0); break;
	case 5: Show_Str(34, 130, 24, 24, "��", 24, 0); break;
	case 6: Show_Str(34, 130, 24, 24, "��", 24, 0); break;
	case 7: Show_Str(34, 130, 24, 24, "��", 24, 0); break;
	case 8: Show_Str(34, 130, 24, 24, "��", 24, 0); break;
	case 9: Show_Str(34, 130, 24, 24, "��", 24, 0); break;
	case 10:Show_Str(34, 130, 24, 24, "��", 24, 0); break;
	case 11:Show_Str(34, 130, 24, 24, "î", 24, 0); break;
	}

	switch (ReYear)	//��Ф
	{
	case 0: Show_Str(58, 130, 24, 24, "��", 24, 0); break;   //��0������ ũ�� ����
	case 1: Show_Str(58, 130, 24, 24, "��", 24, 0); break;   //����
	case 2: Show_Str(58, 130, 24, 24, "��", 24, 0); break;
	case 3: Show_Str(58, 130, 24, 24, "��", 24, 0); break;
	case 4: Show_Str(58, 130, 24, 24, "��", 24, 0); break;
	case 5: Show_Str(58, 130, 24, 24, "��", 24, 0); break;
	case 6: Show_Str(58, 130, 24, 24, "��", 24, 0); break;
	case 7: Show_Str(58, 130, 24, 24, "��", 24, 0); break;
	case 8: Show_Str(58, 130, 24, 24, "��", 24, 0); break;
	case 9: Show_Str(58, 130, 24, 24, "ţ", 24, 0); break;
	case 10:Show_Str(58, 130, 24, 24, "��", 24, 0); break;
	case 11:Show_Str(58, 130, 24, 24, "��", 24, 0); break;
	}
	Show_Str(84, 130, 24, 24, "��", 24, 0);
}

/*��ʾũ������---------------------------------------------------------------*/
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
/*****	��ʾũ���·�  *****/

	if (moon.month_moon == 1) { Show_Str(116, 130, 24, 24, "��", 24, 0); }
	if (moon.month_moon == 2) { Show_Str(116, 130, 24, 24, "��", 24, 0); }
	if (moon.month_moon == 3) { Show_Str(116, 130, 24, 24, "��", 24, 0); }
	if (moon.month_moon == 4) { Show_Str(116, 130, 24, 24, "��", 24, 0); }
	if (moon.month_moon == 5) { Show_Str(116, 130, 24, 24, "��", 24, 0); }
	if (moon.month_moon == 6) { Show_Str(116, 130, 24, 24, "��", 24, 0); }
	if (moon.month_moon == 7) { Show_Str(116, 130, 24, 24, "��", 24, 0); }
	if (moon.month_moon == 8) { Show_Str(116, 130, 24, 24, "��", 24, 0); }
	if (moon.month_moon == 9) { Show_Str(116, 130, 24, 24, "��", 24, 0); }
	if (moon.month_moon == 10) { Show_Str(116, 130, 24, 24, "ʮ", 24, 0); }
	if (moon.month_moon == 11) { Show_Str(116, 130, 24, 24, "��", 24, 0); }
	if (moon.month_moon == 12) { Show_Str(116, 130, 24, 24, "��", 24, 0); }
	Show_Str(140, 130, 24, 24, "��", 24, 0);

	/*****	��ʾũ���յ�ʮλ  *****/
	if (moon.day_moon / 10 == 0) { Show_Str(164, 130, 24, 24, "��", 24, 0); }    //
	if (moon.day_moon / 10 == 1)
		if (moon.day_moon / 10 == 1 & moon.day_moon % 10 == 0) { Show_Str(164, 130, 24, 24, "��", 24, 0); }
		else { Show_Str(164, 130, 24, 24, "ʮ", 24, 0); }    //
	if (moon.day_moon / 10 == 2)
		if (moon.day_moon / 10 == 2 & moon.day_moon % 10 == 0) { Show_Str(164, 130, 24, 24, "��", 24, 0); }
		else { Show_Str(164, 130, 24, 24, "إ", 24, 0); }   //
	if (moon.day_moon / 10 == 3) { Show_Str(164, 130, 24, 24, "��", 24, 0); }	  //"��"�ֵĴ���

/*****	��ʾũ���յĸ�λ  *****/

	if (moon.day_moon % 10 == 1) { Show_Str(188, 130, 24, 24, "һ", 24, 0); }    //
	if (moon.day_moon % 10 == 2) { Show_Str(188, 130, 24, 24, "��", 24, 0); }    //
	if (moon.day_moon % 10 == 3) { Show_Str(188, 130, 24, 24, "��", 24, 0); }	  //"��"�ֵĴ���
	if (moon.day_moon % 10 == 4) { Show_Str(188, 130, 24, 24, "��", 24, 0); }    //
	if (moon.day_moon % 10 == 5) { Show_Str(188, 130, 24, 24, "��", 24, 0); }    //
	if (moon.day_moon % 10 == 6) { Show_Str(188, 130, 24, 24, "��", 24, 0); }    //
	if (moon.day_moon % 10 == 7) { Show_Str(188, 130, 24, 24, "��", 24, 0); }    //
	if (moon.day_moon % 10 == 8) { Show_Str(188, 130, 24, 24, "��", 24, 0); }    //
	if (moon.day_moon % 10 == 9) { Show_Str(188, 130, 24, 24, "��", 24, 0); }    //
	if (moon.day_moon % 10 == 0) { Show_Str(188, 130, 24, 24, "ʮ", 24, 0); }    //
}

/*----------��ʾ����---------------------------------------------*/
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
	if (j == 1) { Show_Str(220, 130, 96, 24, "����С��", 24, 0); }
	if (j == 2) { Show_Str(220, 130, 96, 24, "�����", 24, 0); }
	if (j == 3) { Show_Str(220, 130, 96, 24, "��������", 24, 0); }
	if (j == 4) { Show_Str(220, 130, 96, 24, "������ˮ", 24, 0); }
	if (j == 5) { Show_Str(220, 130, 96, 24, "���쾪��", 24, 0); }
	if (j == 6) { Show_Str(220, 130, 96, 24, "���촺��", 24, 0); }
	if (j == 7) { Show_Str(220, 130, 96, 24, "��������", 24, 0); }
	if (j == 8) { Show_Str(220, 130, 96, 24, "�������", 24, 0); }
	if (j == 9) { Show_Str(220, 130, 96, 24, "��������", 24, 0); }
	if (j == 10) { Show_Str(220, 130, 96, 24, "����С��", 24, 0); }
	if (j == 11) { Show_Str(220, 130, 96, 24, "����â��", 24, 0); }
	if (j == 12) { Show_Str(220, 130, 96, 24, "��������", 24, 0); }
	if (j == 13) { Show_Str(220, 130, 96, 24, "����С��", 24, 0); }
	if (j == 14) { Show_Str(220, 130, 96, 24, "�������", 24, 0); }
	if (j == 15) { Show_Str(220, 130, 96, 24, "��������", 24, 0); }
	if (j == 16) { Show_Str(220, 130, 96, 24, "���촦��", 24, 0); }
	if (j == 17) { Show_Str(220, 130, 96, 24, "�����¶", 24, 0); }
	if (j == 18) { Show_Str(220, 130, 96, 24, "�������", 24, 0); }
	if (j == 19) { Show_Str(220, 130, 96, 24, "���캮¶", 24, 0); }
	if (j == 20) { Show_Str(220, 130, 96, 24, "����˪��", 24, 0); }
	if (j == 21) { Show_Str(220, 130, 96, 24, "��������", 24, 0); }
	if (j == 22) { Show_Str(220, 130, 96, 24, "����Сѩ", 24, 0); }
	if (j == 23) { Show_Str(220, 130, 96, 24, "�����ѩ", 24, 0); }
	if (j == 24) { Show_Str(220, 130, 96, 24, "���춬��", 24, 0); }
	if (j == 0) { Show_Str(224, 130, 96, 24, "�޽���", 24, 0); }
}

void timeplay(void)
{
	//static u8 h = 0, m = 0;
	Show_Str(58, 0, 24, 24, "��", 24, 0);
	Show_Str(114, 0, 24, 24, "��", 24, 0);
	Show_Str(170, 0, 24, 24, "��", 24, 0);
	Show_Str(220, 0, 48, 24, "����", 24, 0);
	LCD_ShowxNum(10, 0, calendar.w_year, 4, 24, 0);//��ʾ��
	LCD_ShowxNum(90, 0, calendar.w_month, 2, 24, 0);//��ʾ��
	LCD_ShowxNum(146, 0, calendar.w_date, 2, 24, 0);//��ʾ��

	switch (calendar.week)
	{
	case 0:
		Show_Font(268, 0, "��", 24, 0);
		break;
	case 1:
		Show_Font(268, 0, "һ", 24, 0);
		break;
	case 2:
		Show_Font(268, 0, "��", 24, 0);
		break;
	case 3:
		Show_Font(268, 0, "��", 24, 0);
		break;
	case 4:
		Show_Font(268, 0, "��", 24, 0);
		break;
	case 5:
		Show_Font(268, 0, "��", 24, 0);
		break;
	case 6:
		Show_Font(268, 0, "��", 24, 0);
		break;
	}

	/*
	if (h != calendar.hour)
	{
		if (calendar.hour / 10 != 0)
		{
			image_display(0, 30, (u8*)gImage[calendar.hour / 10]);//��ָ����ַ��ʾͼ?
		}
		else { image_display(0, 30, (u8*)gImage[11]); }

		image_display(50, 30, (u8*)gImage[calendar.hour % 10]);//��ָ����ַ��ʾͼƬ?
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
��������
**********************************************************/
void time_go(void)
{
	timeplay();
	displaynl();
	displayjieqi();
	displaysx();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//��ʾһ������
//x,y,width,height:λ�úͳߴ�
//curval:��ֵ
//topval:���ֵ
void fft_show_oneband(u16 x, u16 y, u16 width, u16 height, u16 curval, u16 topval)
{
	gui_fill_rectangle(x, y, width, height - curval, BLACK);					//��䱳��ɫ
	gui_fill_rectangle(x, y + height - curval, width, curval, FFT_BANDS_COLOR);	//�����״ɫ
	gui_draw_hline(x, y + height - topval - 1, width, FFT_TOP_COLOR);
}

FIL *f_jpeg;			//JPEG�ļ�ָ��
JDEC *jpeg_dev;   		//���������ṹ��ָ��  
u8  *jpg_buffer;    	//����jpeg���빤������С(������Ҫ3092�ֽ�)����Ϊ��ѹ������������4�ֽڶ���

//��ռ�ڴ�������/�ṹ�������ڴ�
u8 jpeg_mallocall(void)
{
	f_jpeg=(FIL*)mymalloc(sizeof(FIL));
	if(f_jpeg==NULL)return PIC_MEM_ERR;			//�����ڴ�ʧ��.	  
	jpeg_dev=(JDEC*)mymalloc(sizeof(JDEC));
	if(jpeg_dev==NULL)return PIC_MEM_ERR;		//�����ڴ�ʧ��.
	jpg_buffer=(u8*)mymalloc(JPEG_WBUF_SIZE);
	if(jpg_buffer==NULL)return PIC_MEM_ERR;		//�����ڴ�ʧ��. 
	return 0;
}
//�ͷ��ڴ�
void jpeg_freeall(void)
{
	myfree(f_jpeg);			//�ͷ�f_jpeg���뵽���ڴ�
	myfree(jpeg_dev);		//�ͷ�jpeg_dev���뵽���ڴ�
	myfree(jpg_buffer);		//�ͷ�jpg_buffer���뵽���ڴ�
}

void show_mp3_pic(void *pdata)
{
	u8 res;
	UINT(*outfun)(JDEC*, void*, JRECT*);
	u8 scale;	//ͼ��������� 0,1/2,1/4,1/8  
	CPU_SR_ALLOC();
	while (1)
	{
		if (info.pic_show == 1 && show_mode == 1)
		{
			OS_CRITICAL_ENTER();	//�����ٽ���
			//�õ���ʾ�����С	  	 
			picinfo.S_Height = pic_show_size;
			picinfo.S_Width = pic_show_size;
			//��ʾ�Ŀ�ʼ�����
			picinfo.S_YOFF = pic_show_y;
			picinfo.S_XOFF = pic_show_x;

			res = jpeg_mallocall();
			if (res == 0)
			{
				f_lseek(fmp3, info.pic_local);				//����ͷ
				//�õ�JPEG/JPGͼƬ�Ŀ�ʼ��Ϣ		 
				if (res == FR_OK)//���ļ��ɹ�
				{
					res = jd_prepare(jpeg_dev, jpeg_in_func, jpg_buffer, JPEG_WBUF_SIZE, fmp3);//ִ�н����׼������������TjpgDecģ���jd_prepare����
					outfun = jpeg_out_func_point;//Ĭ�ϲ��û���ķ�ʽ��ʾ
					if (res == JDR_OK)//׼������ɹ� 
					{
						for (scale = 0; scale < 4; scale++)//ȷ�����ͼ��ı�������
						{
							if ((jpeg_dev->width >> scale) <= picinfo.S_Width && (jpeg_dev->height >> scale) <= picinfo.S_Height)//��Ŀ��������
							{
								if (((jpeg_dev->width >> scale) != picinfo.S_Width) && ((jpeg_dev->height >> scale) != picinfo.S_Height&&scale))scale = 0;//��������,������
								else outfun = jpeg_out_func_fill;	//����ʾ�ߴ�����,���Բ������ķ�ʽ��ʾ 
								break;
							}
						}
						if (scale == 4)scale = 0;//����
						picinfo.ImgHeight = jpeg_dev->height >> scale;	//���ź��ͼƬ�ߴ�
						picinfo.ImgWidth = jpeg_dev->width >> scale;	//���ź��ͼƬ�ߴ� 
						ai_draw_init();								//��ʼ�����ܻ�ͼ 
						//ִ�н��빤��������TjpgDecģ���jd_decomp����
						OS_CRITICAL_EXIT();	//�����ٽ���
						res = jd_decomp(jpeg_dev, outfun, scale);
						OS_CRITICAL_ENTER();	//�����ٽ���
					}
				}
			}
			jpeg_freeall();		//�ͷ��ڴ�
			info.pic_show = 0;
			if (write_bit == 0x20)
			{
				write_bit = 0x30;
			}
			else if (write_bit == 0x10)
			{
				write_bit = 0x30;
			}
			OS_CRITICAL_EXIT();	//�����ٽ���			
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
		VS_Get_Spec(info.FFTbuf); //��ȡƵ������
		FFT_post(info.FFTbuf);	  //����Ƶ��Ч����ʾ
	}
	RTC_Get();
	info.time = VS_Get_DecodeTime(); //�õ�����ʱ��
	if (sec != calendar.sec && lcd_bit == 1)
	{
		sec = calendar.sec;
		fn = (u8*)(*info.mp3fileinfo.lfname ? info.mp3fileinfo.lfname : info.mp3fileinfo.fname);
		temp = VS_Get_HeadInfo(); //��ñ�����
		if (info.kbps == 0xffff)//δ���¹�
		{
			info.playtime = 0;
			info.kbps = VS_Get_HeadInfo();//��ñ�����
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
					Show_Str(0, 162, 320, 16, info.TIT2, 16, 0);				//��ʾ�������� 
				}
				else if (info.TIT2 != NULL && info.TIT2[0] != 0x00)
				{
					Show_Str(0, 162, 320, 16, info.TIT2, 16, 0);				//��ʾ�������� 
				}
				else
					Show_Str(0, 162, 320, 16, fn, 16, 0);				//��ʾ�������� 
			}
			else
			{
				Show_Str(0, 162, 320, 16, fn, 16, 0);				//��ʾ�������� 
			}
			LCD_ShowxNum(0, 182, info.curindex + 1, 3, 16, 0X80);		//����
			LCD_ShowChar(24, 182, '/', 16, 0);
			LCD_ShowxNum(32, 182, info.totmp3num, 3, 16, 0X80); 	//����Ŀ	
			LCD_ShowString(32 + 30, 182, 32, 16, 16, "VOL:");
			LCD_ShowxNum(64 + 30, 182, (vsset.mvol - 100) / 5, 2, 16, 0X80); 	//��ʾ����		

			if (temp != info.kbps)
			{
				info.kbps = temp;//����KBPS	  				     
			}
			//��ʾ����ʱ��			 
			LCD_ShowxNum(118, 182, info.time / 60, 2, 16, 0X80);		//����
			LCD_ShowChar(118 + 16, 182, ':', 16, 0);
			LCD_ShowxNum(118 + 24, 182, info.time % 60, 2, 16, 0X80);	//����		
			LCD_ShowChar(118 + 40, 182, '/', 16, 0);
			//��ʾ��ʱ��
			if (info.kbps)info.time = (info.fmp3->fsize / info.kbps) / 125;//�õ�������   (�ļ�����(�ֽ�)/(1000/8)/������=����������    	  
			else info.time = 0;//�Ƿ�λ��	  
			LCD_ShowxNum(118 + 48, 182, info.time / 60, 2, 16, 0X80);	//����
			LCD_ShowChar(118 + 64, 182, ':', 16, 0);
			LCD_ShowxNum(118 + 72, 182, info.time % 60, 2, 16, 0X80);	//����	  		    
			//��ʾλ��			   
			LCD_ShowxNum(214, 182, info.kbps, 3, 16, 0X80); 	//��ʾλ��	 
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
				Show_Str(0, 0, 320, 16, fn, 16, 0);				//��ʾ�������� 
			}
			LCD_ShowString(0, 256, 32, 16, 16, "VOL:");
			LCD_ShowxNum(32, 256, (vsset.mvol - 100) / 5, 2, 16, 0X80); 	//��ʾ����	 
			LCD_ShowxNum(0, 273, info.curindex + 1, 3, 16, 0X80);		//����
			LCD_ShowChar(24, 273, '/', 16, 0);
			LCD_ShowxNum(32, 273, info.totmp3num, 3, 16, 0X80); 	//����Ŀ	

			if (info.playtime == 0)info.playtime = info.time;
			else if ((info.time != info.playtime) && (info.time != 0))//1sʱ�䵽,������ʾ����
			{
				info.playtime = info.time;//����ʱ�� 	 			
				temp = VS_Get_HeadInfo(); //��ñ�����	   				 
				if (temp != info.kbps)
				{
					info.kbps = temp;//����KBPS	  				     
				}
				//��ʾ����ʱ��			 
				LCD_ShowxNum(0, 290, info.time / 60, 2, 16, 0X80);		//����
				LCD_ShowChar(16, 290, ':', 16, 0);
				LCD_ShowxNum(24, 290, info.time % 60, 2, 16, 0X80);	//����		
				LCD_ShowChar(40, 290, '/', 16, 0);
				//��ʾ��ʱ��
				if (info.kbps)info.time = (info.fmp3->fsize / info.kbps) / 125;//�õ�������   (�ļ�����(�ֽ�)/(1000/8)/������=����������    	  
				else info.time = 0;//�Ƿ�λ��	  
				LCD_ShowxNum(48, 290, info.time / 60, 2, 16, 0X80);	//����
				LCD_ShowChar(64, 290, ':', 16, 0);
				LCD_ShowxNum(72, 290, info.time % 60, 2, 16, 0X80);	//����	  		    
				//��ʾλ��			   
				LCD_ShowxNum(65, 256, info.kbps, 3, 16, 0X80); 	//��ʾλ��	 
				LCD_ShowString(65 + 24, 256, 200, 16, 16, "Kbps");
			}
		}
	}
}




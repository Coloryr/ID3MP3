#include "show.h"
#include "lunar.h"
#include "image2lcd.h"
#include "image1.h"
#include "lcd.h"
#include "text.h"
#include "rtc.h"
#include "delay.h"
#include "key.h"
#include "vs10xx.h"
#include "mp3player.h"
#include "tjpgd.h"
#include "piclib.h"
#include "guix.h"

u8 lcd_bit=0;

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

	if (moon.month_moon == 1) { Show_Str(108, 130, 24, 24, "��", 24, 0); }
	if (moon.month_moon == 2) { Show_Str(108, 130, 24, 24, "��", 24, 0); }
	if (moon.month_moon == 3) { Show_Str(108, 130, 24, 24, "��", 24, 0); }
	if (moon.month_moon == 4) { Show_Str(108, 130, 24, 24, "��", 24, 0); }
	if (moon.month_moon == 5) { Show_Str(108, 130, 24, 24, "��", 24, 0); }
	if (moon.month_moon == 6) { Show_Str(108, 130, 24, 24, "��", 24, 0); }
	if (moon.month_moon == 7) { Show_Str(108, 130, 24, 24, "��", 24, 0); }
	if (moon.month_moon == 8) { Show_Str(108, 130, 24, 24, "��", 24, 0); }
	if (moon.month_moon == 9) { Show_Str(108, 130, 24, 24, "��", 24, 0); }
	if (moon.month_moon == 10) { Show_Str(108, 130, 24, 24, "ʮ", 24, 0); }
	if (moon.month_moon == 11) { Show_Str(108, 130, 24, 24, "��", 24, 0); }
	if (moon.month_moon == 12) { Show_Str(108, 130, 24, 24, "��", 24, 0); }
	Show_Str(132, 130, 24, 24, "��", 24, 0);

	/*****	��ʾũ���յ�ʮλ  *****/
	if (moon.day_moon / 10 == 0) { Show_Str(156, 130, 24, 24, "��", 24, 0); }    //
	if (moon.day_moon / 10 == 1)
		if (moon.day_moon / 10 == 1 & moon.day_moon % 10 == 0) { Show_Str(156, 130, 24, 24, "��", 24, 0); }
		else { Show_Str(156, 130, 24, 24, "ʮ", 24, 0); }    //
	if (moon.day_moon / 10 == 2)
		if (moon.day_moon / 10 == 2 & moon.day_moon % 10 == 0) { Show_Str(156, 130, 24, 24, "��", 24, 0); }
		else { Show_Str(156, 130, 24, 24, "إ", 24, 0); }   //
	if (moon.day_moon / 10 == 3) { Show_Str(156, 130, 24, 24, "��", 24, 0); }	  //"��"�ֵĴ���

/*****	��ʾũ���յĸ�λ  *****/

	if (moon.day_moon % 10 == 1) { Show_Str(180, 130, 24, 24, "һ", 24, 0); }    //
	if (moon.day_moon % 10 == 2) { Show_Str(180, 130, 24, 24, "��", 24, 0); }    //
	if (moon.day_moon % 10 == 3) { Show_Str(180, 130, 24, 24, "��", 24, 0); }	  //"��"�ֵĴ���
	if (moon.day_moon % 10 == 4) { Show_Str(180, 130, 24, 24, "��", 24, 0); }    //
	if (moon.day_moon % 10 == 5) { Show_Str(180, 130, 24, 24, "��", 24, 0); }    //
	if (moon.day_moon % 10 == 6) { Show_Str(180, 130, 24, 24, "��", 24, 0); }    //
	if (moon.day_moon % 10 == 7) { Show_Str(180, 130, 24, 24, "��", 24, 0); }    //
	if (moon.day_moon % 10 == 8) { Show_Str(180, 130, 24, 24, "��", 24, 0); }    //
	if (moon.day_moon % 10 == 9) { Show_Str(180, 130, 24, 24, "��", 24, 0); }    //
	if (moon.day_moon % 10 == 0) { Show_Str(180, 130, 24, 24, "ʮ", 24, 0); }    //
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
	if (j == 1) { Show_Str(204, 130, 96, 24, "����С��", 24, 0); }
	if (j == 2) { Show_Str(204, 130, 96, 24, "�����", 24, 0); }
	if (j == 3) { Show_Str(204, 130, 96, 24, "��������", 24, 0); }
	if (j == 4) { Show_Str(204, 130, 96, 24, "������ˮ", 24, 0); }
	if (j == 5) { Show_Str(204, 130, 96, 24, "���쾪��", 24, 0); }
	if (j == 6) { Show_Str(204, 130, 96, 24, "���촺��", 24, 0); }
	if (j == 7) { Show_Str(204, 130, 96, 24, "��������", 24, 0); }
	if (j == 8) { Show_Str(204, 130, 96, 24, "�������", 24, 0); }
	if (j == 9) { Show_Str(204, 130, 96, 24, "��������", 24, 0); }
	if (j == 10) { Show_Str(204, 130, 96, 24, "����С��", 24, 0); }
	if (j == 11) { Show_Str(204, 130, 96, 24, "����â��", 24, 0); }
	if (j == 12) { Show_Str(204, 130, 96, 24, "��������", 24, 0); }
	if (j == 13) { Show_Str(204, 130, 96, 24, "����С��", 24, 0); }
	if (j == 14) { Show_Str(204, 130, 96, 24, "�������", 24, 0); }
	if (j == 15) { Show_Str(204, 130, 96, 24, "��������", 24, 0); }
	if (j == 16) { Show_Str(204, 130, 96, 24, "���촦��", 24, 0); }
	if (j == 17) { Show_Str(204, 130, 96, 24, "�����¶", 24, 0); }
	if (j == 18) { Show_Str(204, 130, 96, 24, "�������", 24, 0); }
	if (j == 19) { Show_Str(204, 130, 96, 24, "���캮¶", 24, 0); }
	if (j == 20) { Show_Str(204, 130, 96, 24, "����˪��", 24, 0); }
	if (j == 21) { Show_Str(204, 130, 96, 24, "��������", 24, 0); }
	if (j == 22) { Show_Str(204, 130, 96, 24, "����Сѩ", 24, 0); }
	if (j == 23) { Show_Str(204, 130, 96, 24, "�����ѩ", 24, 0); }
	if (j == 24) { Show_Str(204, 130, 96, 24, "���춬��", 24, 0); }
	if (j == 0) { Show_Str(216, 130, 96, 24, "�޽���", 24, 0); }
}

void timeplay(void)
{
	Show_Str(58, 0, 24, 24, "��", 24, 0);
	Show_Str(106, 0, 24, 24, "��", 24, 0);
	Show_Str(154, 0, 24, 24, "��", 24, 0);
	Show_Str(180, 0, 48, 24, "����", 24, 0);
	LCD_ShowxNum(10, 0, calendar.w_year, 4, 24, 0);//��ʾ��
	LCD_ShowxNum(82, 0, calendar.w_month, 2, 24, 0);//��ʾ��
	LCD_ShowxNum(130, 0, calendar.w_date, 2, 24, 0);//��ʾ��

	switch (calendar.week)
	{
	case 0:
		Show_Font(228, 0, "��", 24, 0);
		break;
	case 1:
		Show_Font(228, 0, "һ", 24, 0);
		break;
	case 2:
		Show_Font(228, 0, "��", 24, 0);
		break;
	case 3:
		Show_Font(228, 0, "��", 24, 0);
		break;
	case 4:
		Show_Font(228, 0, "��", 24, 0);
		break;
	case 5:
		Show_Font(228, 0, "��", 24, 0);
		break;
	case 6:
		Show_Font(228, 0, "��", 24, 0);
		break;
	}

	if (calendar.hour / 10 != 0)
	{
		image_display(0, 30, (u8*)gImage[calendar.hour / 10]);//��ָ����ַ��ʾͼ?
	}
	else { image_display(0, 30, (u8*)gImage[11]); }

	image_display(50, 30, (u8*)gImage[calendar.hour % 10]);//��ָ����ַ��ʾͼƬ?

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

	image_display(150, 30, (u8*)gImage[calendar.min / 10]);
	image_display(200, 30, (u8*)gImage[calendar.min % 10]);

	LCD_ShowChar(250, 86, ':', 24, 0);
	LCD_ShowxNum(262, 86, calendar.sec, 2, 24, 0x80);
}
          
/**********************************************************
��������
**********************************************************/
void time_go(void)
{
	RTC_Get();
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

//��ʾ��Ŀ����
//index:��ǰ����
//total:���ļ���
void mp3_index_show(u16 index, u16 total)
{
	//��ʾ��ǰ��Ŀ������,������Ŀ��
	if (lcd_bit ==1)
	{
		LCD_ShowxNum(224 + 0, 20, index, 3, 16, 0X80);		//����
		LCD_ShowChar(224 + 24, 20, '/', 16, 0);
		LCD_ShowxNum(224 + 32, 20, total, 3, 16, 0X80); 	//����Ŀ	
	}
}
//��ʾ��ǰ����
void mp3_vol_show(u8 vol)
{
	if (lcd_bit ==1 )
	{
		LCD_ShowString(224, 40, 32, 16, 16, "VOL:");
		LCD_ShowxNum(224 + 32, 40, vol, 2, 16, 0X80); 	//��ʾ����	 
	}
}
u16 f_kbps=0;//�����ļ�λ��	
//��ʾ����ʱ��,������ ��Ϣ 
//lenth:�����ܳ���
void mp3_msg_show(u32 lenth)
{
	static u16 playtime = 0;//����ʱ����
	u16 time = 0;// ʱ�����
	u16 temp = 0;
	if (lcd_bit == 1)
	{
		if (f_kbps == 0xffff)//δ���¹�
		{
			playtime = 0;
			f_kbps = VS_Get_HeadInfo();//��ñ�����
		}
		time = VS_Get_DecodeTime(); //�õ�����ʱ��

		if (playtime == 0)playtime = time;
		else if ((time != playtime) && (time != 0))//1sʱ�䵽,������ʾ����
		{
			playtime = time;//����ʱ�� 	 			
			temp = VS_Get_HeadInfo(); //��ñ�����	   				 
			if (temp != f_kbps)
			{
				f_kbps = temp;//����KBPS	  				     
			}
			//��ʾ����ʱ��			 
			LCD_ShowxNum(224, 60, time / 60, 2, 16, 0X80);		//����
			LCD_ShowChar(224 + 16, 60, ':', 16, 0);
			LCD_ShowxNum(224 + 24, 60, time % 60, 2, 16, 0X80);	//����		
			LCD_ShowChar(224 + 40, 60, '/', 16, 0);
			//��ʾ��ʱ��
			if (f_kbps)time = (lenth / f_kbps) / 125;//�õ�������   (�ļ�����(�ֽ�)/(1000/8)/������=����������    	  
			else time = 0;//�Ƿ�λ��	  
			LCD_ShowxNum(224 + 48, 60, time / 60, 2, 16, 0X80);	//����
			LCD_ShowChar(224 + 64, 60, ':', 16, 0);
			LCD_ShowxNum(224 + 72, 60, time % 60, 2, 16, 0X80);	//����	  		    
			//��ʾλ��			   
			LCD_ShowxNum(224, 80, f_kbps, 3, 16, 0X80); 	//��ʾλ��	 
			LCD_ShowString(224 + 24, 80, 200, 16, 16, "Kbps");
		}
		VS_Get_Spec(info.FFTbuf); //��ȡƵ������
		FFT_post(info.FFTbuf);	  //����Ƶ��Ч����ʾ
	}
}

//��������Ƿ�ʹ��malloc�����������ķ��䷽��.
#if JPEG_USE_MALLOC == 1 //ʹ��malloc	 

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

#else 	//��ʹ��malloc   

FIL  tf_jpeg; 
JDEC tjpeg_dev;   		  
FIL  *f_jpeg=&tf_jpeg;						//JPEG�ļ�ָ��
JDEC *jpeg_dev=&tjpeg_dev;   				//���������ṹ��ָ��   
__align(4) u8 jpg_buffer[JPEG_WBUF_SIZE];	//����jpeg���빤������С(������Ҫ3092�ֽ�)����Ϊ��ѹ������������4�ֽڶ���
	
#endif

void show_mp3_pic(void *pdata)
{
	FIL* fmp3 = 0;
	u8 res;
	UINT(*outfun)(JDEC*, void*, JRECT*);
	u8 scale;	//ͼ��������� 0,1/2,1/4,1/8  
		
	if (f_open(fmp3, (const TCHAR*)info.pname, FA_READ | FA_OPEN_EXISTING) != FR_OK)
		return;
	
	if ((pic_show_x + pic_show_size) > picinfo.lcdwidth)return;		//x���곬��Χ��.
			if ((pic_show_y + pic_show_size) > picinfo.lcdheight)return;		//y���곬��Χ��.  
			//�õ���ʾ�����С	  	 
			picinfo.S_Height = pic_show_size;
			picinfo.S_Width = pic_show_size;
			//��ʾ������Ч
			if (picinfo.S_Height == 0 || picinfo.S_Width == 0)
			{
				picinfo.S_Height = lcddev.height;
				picinfo.S_Width = lcddev.width;
				return;
			}
			//��ʾ�Ŀ�ʼ�����
			picinfo.S_YOFF = pic_show_y;
			picinfo.S_XOFF = pic_show_x;

#if JPEG_USE_MALLOC == 1	//ʹ��malloc
			res = jpeg_mallocall();
#endif
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
						res = jd_decomp(jpeg_dev, outfun, scale);
					}
				}
			}
#if JPEG_USE_MALLOC == 1//ʹ��malloc
			jpeg_freeall();		//�ͷ��ڴ�
#endif
}


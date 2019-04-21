#include "includes.h"
#include "show.h"
#include "lcd.h"
#include "ff.h"
#include "mp3player.h"
#include "text.h"
#include "vs10xx.h"
#include "string.h"
#include "guix.h"
#include "tjpgd.h"
#include "piclib.h"

u8 lcd_bit = 0;
FIL *fmp3_pic = 0;

JDEC *jpeg_dev; //���������ṹ��ָ��
u8 *jpg_buffer; //����jpeg���빤������С(������Ҫ3092�ֽ�)����Ϊ��ѹ������������4�ֽڶ���

//��ռ�ڴ�������/�ṹ�������ڴ�
u8 jpeg_mallocall(void)
{
	jpeg_dev = (JDEC *)mymalloc(sizeof(JDEC));
	if (jpeg_dev == NULL)
		return PIC_MEM_ERR; //�����ڴ�ʧ��.
	jpg_buffer = (u8 *)mymalloc(JPEG_WBUF_SIZE);
	if (jpg_buffer == NULL)
		return PIC_MEM_ERR; //�����ڴ�ʧ��.
	return 0;
}
//�ͷ��ڴ�
void jpeg_freeall(void)
{
	myfree(jpeg_dev);   //�ͷ�jpeg_dev���뵽���ڴ�
	myfree(jpg_buffer); //�ͷ�jpg_buffer���뵽���ڴ�
}

void show_mp3_pic(void *pdata)
{
	u8 res;
	UINT(*outfun)
	(JDEC *, void *, JRECT *);
	u8 scale; //ͼ��������� 0,1/2,1/4,1/8
	CPU_SR_ALLOC();
	while (1)
	{
		if (info.pic_show == 1)
		{
			OS_CRITICAL_ENTER(); //�����ٽ���
			//�õ���ʾ�����С
			picinfo.S_Height = pic_show_size;
			picinfo.S_Width = pic_show_size;
			//��ʾ�Ŀ�ʼ�����
			picinfo.S_YOFF = pic_show_y;
			picinfo.S_XOFF = pic_show_x;

			res = jpeg_mallocall();
			if (res == 0)
			{
				//�õ�JPEG/JPGͼƬ�Ŀ�ʼ��Ϣ
				if (res == FR_OK) //���ļ��ɹ�
				{
					res = jd_prepare(jpeg_dev, jpeg_in_func, jpg_buffer, JPEG_WBUF_SIZE, fmp3_pic); //ִ�н����׼������������TjpgDecģ���jd_prepare����
					outfun = jpeg_out_func_point;													//Ĭ�ϲ��û���ķ�ʽ��ʾ
					if (res == JDR_OK)																//׼������ɹ�
					{
						for (scale = 0; scale < 4; scale++) //ȷ�����ͼ��ı�������
						{
							if ((jpeg_dev->width >> scale) <= picinfo.S_Width && (jpeg_dev->height >> scale) <= picinfo.S_Height) //��Ŀ��������
							{
								if (((jpeg_dev->width >> scale) != picinfo.S_Width) && ((jpeg_dev->height >> scale) != picinfo.S_Height && scale))
									scale = 0; //��������,������
								else
									outfun = jpeg_out_func_fill; //����ʾ�ߴ�����,���Բ������ķ�ʽ��ʾ
								break;
							}
						}
						if (scale == 4) //����
						{
							scale = 0;
							Show_Str(0, 0, 240, 16, "ͼƬ����", 16, 0);
						}
						picinfo.ImgHeight = jpeg_dev->height >> scale; //���ź��ͼƬ�ߴ�
						picinfo.ImgWidth = jpeg_dev->width >> scale;   //���ź��ͼƬ�ߴ�
						ai_draw_init();								   //��ʼ�����ܻ�ͼ
						write_bit = 0x00;
						//ִ�н��빤��������TjpgDecģ���jd_decomp����
						OS_CRITICAL_EXIT(); //�����ٽ���
						res = jd_decomp(jpeg_dev, outfun, scale);
						OS_CRITICAL_ENTER(); //�����ٽ���
					}
				}
			}
			jpeg_freeall(); //�ͷ��ڴ�
			info.pic_show = 0;
			write_bit = 0x20;
			OS_CRITICAL_EXIT(); //�����ٽ���
		}
		else if (info.pic_show == 2)
		{
			OS_CRITICAL_ENTER(); //�����ٽ���
			LCD_Fill(0, 0, 240, (240 + 16 * 3) - 1, BLACK);
			show_all(1); //��ʾһ�θ���
			show_all(3); //��ʾ������Ϣ
			Show_Str(0, 0, 240, 16, "PNG��ʽͼƬ��֧��", 16, 0);
			info.pic_show = 0;
			OS_CRITICAL_EXIT(); //�����ٽ���
		}
		else if (info.pic_show == 3)
		{
			OS_CRITICAL_ENTER(); //�����ٽ���
			LCD_Fill(0, 0, 240, (240 + 16 * 3) - 1, BLACK);
			show_all(1); //��ʾһ�θ���
			show_all(3); //��ʾ������Ϣ
			Show_Str(0, 0, 240, 16, "û��ͼƬ", 16, 0);
			info.pic_show = 0;
			OS_CRITICAL_EXIT(); //�����ٽ���
		}
	}
}

void show_all(u8 mode)
{
	u16 temp = 0;
	info.time = VS_Get_DecodeTime(); //�õ�����ʱ��
	if (mode == 1)
	{
		if (info.size != 0)
		{
			if (info.TIT2[0] != 0)
				Show_Str(0, 240, 240, 16, info.TIT2, 16, 0); //��ʾ��������
			if (info.TPE1[0] != 0)
				Show_Str(0, 240 + 17, 240, 16, info.TPE1, 16, 0); //��ʾ��������
			if (info.TALB[0] != 0)
				Show_Str(0, 240 + 34, 240, 16, info.TALB, 16, 0); //��ʾ����ר��
			if (info.TIT2[0] == 0 && info.TPE1[0] == 0 && info.TALB[0] == 0)
				Show_Str(0, 240, 240, 16, info.fn, 16, 0); //��ʾ��������
		}
		else
		{
			Show_Str(0, 0, 320, 16, info.fn, 16, 0); //��ʾ��������
		}
		//Show_Str(186, 292, 48, 24, "����", 24, 0);
	}
	else if (((info.pic_show == 0 && mode == 0 && lcd_bit == 1) || mode == 3))
	{
		if (info.playtime == 0)
			info.playtime = info.time;
		else if (((info.time != info.playtime) && (info.time != 0)) || mode == 3) //1sʱ�䵽,������ʾ����
		{
			info.playtime = info.time; //����ʱ��
			temp = VS_Get_HeadInfo();  //��ñ�����
			if (temp != info.kbps)
				info.kbps = temp; //����KBPS
			//��ʾ����ʱ��
			LCD_ShowxNum(0, 289, info.time / 60, 2, 16, 0X80); //����
			LCD_ShowChar(16, 289, ':', 16, 0);
			LCD_ShowxNum(24, 289, info.time % 60, 2, 16, 0X80); //����
			LCD_ShowChar(40, 289, '/', 16, 0);
			//��ʾ��ʱ��
			if (info.kbps)
				info.time = ((info.fmp3->fsize - info.size) / info.kbps) / 125; //�õ�������   (�ļ�����(�ֽ�)/(1000/8)/������=����������
			else
				info.time = 0;									//�Ƿ�λ��
			LCD_ShowxNum(48, 289, info.time / 60, 2, 16, 0X80); //����
			LCD_ShowChar(64, 289, ':', 16, 0);
			LCD_ShowxNum(72, 289, info.time % 60, 2, 16, 0X80); //����
			//��ʾλ��
			LCD_ShowxNum(62, 303, info.kbps, 3, 16, 0X80); //��ʾλ��
			LCD_ShowString(62 + 24, 303, 200, 16, 16, "Kbps");
		}
		LCD_ShowString(0, 303, 32, 16, 16, "VOL:");
		LCD_ShowxNum(0 + 32, 303, (vsset.mvol - 100) / 10, 2, 16, 0X80); //��ʾ����
		LCD_ShowxNum(95, 289, info.curindex + 1, 3, 16, 0X80);			 //����
		LCD_ShowChar(95 + 24, 289, '/', 16, 0);
		LCD_ShowxNum(95 + 32, 289, info.totmp3num, 3, 16, 0X80); //����Ŀ
	}
	else if (mode == 2)
	{
		Show_Str(88, 20, 64, 16, "��Ч����", 16, 0);

		Show_Str(10, 44, 64, 16, "������С", 16, 0);
		Show_Str(10, 76, 64, 16, "����Ƶ��", 16, 0);
		Show_Str(10, 108, 64, 16, "��������", 16, 0);
		Show_Str(10, 140, 64, 16, "����Ƶ��", 16, 0);
		Show_Str(10, 172, 64, 16, "��������", 16, 0);
		Show_Str(10, 204, 64, 16, "�ռ�Ч��", 16, 0);

		LCD_ShowString(94, 40, 32, 24, 24, "<");
		LCD_ShowString(94, 72, 32, 24, 24, "<");
		LCD_ShowString(94, 104, 32, 24, 24, "<");
		LCD_ShowString(94, 136, 32, 24, 24, "<");
		LCD_ShowString(94, 168, 32, 24, 24, "<");
		LCD_ShowString(94, 200, 32, 24, 24, "<");

		LCD_ShowxNum(126, 44, (vsset.mvol - 100) / 10, 2, 16, 0X80);
		LCD_ShowxNum(126, 76, vsset.bflimit, 2, 16, 0X80);
		LCD_ShowxNum(126, 108, vsset.bass, 2, 16, 0X80);
		LCD_ShowxNum(126, 140, vsset.tflimit, 2, 16, 0X80);
		LCD_ShowxNum(126, 172, vsset.treble, 2, 16, 0X80);
		LCD_ShowxNum(130, 204, vsset.effect, 1, 16, 0X80);

		LCD_ShowString(162, 40, 32, 24, 24, ">");
		LCD_ShowString(162, 72, 32, 24, 24, ">");
		LCD_ShowString(162, 104, 32, 24, 24, ">");
		LCD_ShowString(162, 136, 32, 24, 24, ">");
		LCD_ShowString(162, 168, 32, 24, 24, ">");
		LCD_ShowString(162, 200, 32, 24, 24, ">");

		Show_Str(186, 292, 48, 24, "����", 24, 0);
	}
}

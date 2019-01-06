#include "mp3id3.h" 
#include "exfuns.h"
#include "ff.h"
#include "malloc.h" 
#include "piclib.h"
#include "lcd.h"
#include "tjpgd.h"
#include "text.h"
#include "delay.h"
#include "flash.h"

u16 size;

u8 *TIT2;						//����
u8 *TPE1;						//����

u16 UNICODEtoGBK(u16 unicode)  //???????
{
	u16 t[2];
	u16 c;
	u32 i, li, hi;
	u16 n;
	u32 gbk2uni_offset = 0;

	if (unicode < 0x80)c = unicode;//ASCII,ֱ�Ӳ���ת��.
	else
	{
		//UNICODE 2 GBK   
		gbk2uni_offset = 0;
		/* Unicode to OEMCP */
		hi = ftinfo.ugbksize / 2;//�԰뿪.
		hi = hi / 4 - 1;
		li = 0;
		for (n = 16; n; n--)
		{
			i = li + (hi - li) / 2;
			SPI_Flash_Read((u8*)&t, ftinfo.ugbkaddr + i * 4 + gbk2uni_offset, 4);//����4���ֽ�  
			if (unicode == t[0]) break;
			if (unicode > t[0])li = i;
			else hi = i;
		}
		c = n ? t[1] : 0;
	}
	return c;
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

u16 mp3id3_is(const TCHAR* path, u8 pic_show)
{
	FIL* fmp3 = 0;
	u16 i = 0;
	u8 res = 0;
	u8 *databuf = 0;
	u8 img = 1;
	u8 temp;
	u16 tag_size;
	u8 code_type;
	u8 scale;	//ͼ��������� 0,1/2,1/4,1/8  
	u16 a = 0;
	u8 temp1, temp2;
	UINT(*outfun)(JDEC*, void*, JRECT*);

	databuf = (u8*)mymalloc(READ_buff_size);
	fmp3 = (FIL*)mymalloc(sizeof(FIL));
	if (TIT2 == NULL)
		TIT2 = (u8*)mymalloc(40);
	if (TPE1 == NULL)
		TPE1 = (u8*)mymalloc(40);
	for (temp = 0; temp < 40; temp++)
	{
		TIT2[temp] = 0;
		TPE1[temp] = 0;
	}
	if (databuf == NULL || fmp3 == NULL)//�ڴ�����ʧ��.
		while (1)
		{
			Show_Str(30, 20, 160, 16, "MP3ID3���ڴ�����ʧ��", 16, 0);
			delay_ms(200);
			LCD_Fill(30, 20, 160, 16, BLACK);//�����ʾ	     
			delay_ms(200);
		}
	while (f_open(fmp3, path, FA_READ | FA_OPEN_EXISTING))
	{
		Show_Str(30, 20, 160, 16, "MP3ID3���ļ���ȡ����", 16, 0);
		delay_ms(200);
		LCD_Fill(30, 20, 160, 16, BLACK);//�����ʾ	     
		delay_ms(200);
	}
	res = f_read(fmp3, databuf, 10, (UINT*)&br);//����mp3id3ͷ
	if (databuf[0] == 0x49 && databuf[1] == 0x44 && databuf[2] == 0x33)
	{
		//�����С
		size = databuf[6] & 0x7f | ((databuf[7] & 0x7f) << 7)
			| ((databuf[8] & 0x7f) << 14) | ((databuf[9] & 0x7f) << 21);
		res = f_read(fmp3, databuf, READ_buff_size, (UINT*)&br);
		i = 0;
		while (img)							//���Ҹ���
		{
			if (databuf[i] == 0x54 && databuf[i + 1] == 0x49 && databuf[i + 2] == 0x54 && databuf[i + 3] == 0x32)
			{	//�ҵ�λ��
				tag_size = databuf[i + 4] << 24
					| databuf[i + 5] << 16
					| databuf[i + 6] << 8
					| databuf[i + 7];
				if (databuf[i + 11] == 0xFE && databuf[i + 12] == 0xFF && databuf[i + 10] == 0x01)
				{
					code_type = 2;							//UTF-16BE
					i += 13;
					tag_size -= 3;
				}
				else if (databuf[i + 11] == 0xFF && databuf[i + 12] == 0xFE && databuf[i + 10] == 0x01)
				{
					code_type = 1;							//UTF-16LE
					i += 13;
					tag_size -= 3;
				}
				else if (databuf[i + 10] == 0x00)
				{
					code_type = 0;							//iso-8859-1
					i += 11;
					tag_size -= 1;
				}
				else if (databuf[i + 10] == 0x03)
				{
					code_type = 3;							//UTF-8
					i += 11;
					tag_size -= 1;
				}
				if (code_type == 3)			//UTF-8
				{
					for (temp = 0; temp < tag_size;)
					{
						a = (databuf[i + temp] << 8) | databuf[i + temp + 1];
						if (a < 0x80) {				/* 7-bit */
							TIT2[temp++] = (BYTE)a;
						}
						else {
							if (a < 0x800) {		/* 11-bit */
								TIT2[temp++] = (BYTE)(0xC0 | a >> 6);
							}
							else {				/* 16-bit */
								TIT2[temp++] = (BYTE)(0xE0 | a >> 12);
								TIT2[temp++] = (BYTE)(0x80 | (a >> 6 & 0x3F));
							}
							TIT2[temp++] = (BYTE)(0x80 | (a & 0x3F));
						}
					}
				}
				else if (code_type == 2)			//UTF-16BE
				{
					temp2 = 0;
					for (temp = 0; temp < tag_size;)
					{
						a = (databuf[i + temp] << 8) | databuf[i + temp + 1];
						a = UNICODEtoGBK(a);
						temp1 = (a >> 8) & 0xff;
						if (temp1 != 0x00)
						{
							TIT2[temp2] = temp1;
							temp2++;
						}
						temp1 = a & 0xff;
						if (temp1 != 0x00)
						{
							TIT2[temp2] = temp1;
							temp2++;
						}
						temp += 2;
					}
				}
				else if (code_type == 1)			//UTF-16LE
				{
					temp2 = 0;
					for (temp = 0; temp < tag_size;)
					{
						a = (databuf[i + temp + 1] << 8) | databuf[i + temp];
						a = UNICODEtoGBK(a);
						temp1 = (a >> 8) & 0xff;
						if (temp1 != 0x00)
						{
							TIT2[temp2] = temp1;
							temp2++;
						}
						temp1 = a & 0xff;
						if (temp1 != 0x00)
						{
							TIT2[temp2] = temp1;
							temp2++;
						}
						temp += 2;
					}
				}
				else if (code_type == 0)			//iso-8859-1
				{
					for (temp = 0; temp < tag_size;)
					{
						temp1 = databuf[i + temp];
						if (temp1 != 0x00)
							TIT2[temp++] = temp1;
						else
							temp++;
					}
				}
				img = 0;
			}
			else
				i++;
			if (i >= 4096)								//�Ҳ���λ��
			{
				TPE1 = 0;
				img = 0;
				code_type = 4;
			}
		}
		img = 1;
		i = 0;
		while (img)							//��������
		{
			myfree(TPE1);
			if (databuf[i] == 0x54 && databuf[i + 1] == 0x50 && databuf[i + 2] == 0x45 && databuf[i + 3] == 0x31)
			{	//�ҵ�λ��
				tag_size = databuf[i + 4] << 24
					| databuf[i + 5] << 16
					| databuf[i + 6] << 8
					| databuf[i + 7];
				if (databuf[i + 11] == 0xFE && databuf[i + 12] == 0xFF && databuf[i + 10] == 0x01)
				{
					code_type = 2;							//UTF-16BE
					i += 13;
					tag_size -= 3;
				}
				else if (databuf[i + 11] == 0xFF && databuf[i + 12] == 0xFE && databuf[i + 10] == 0x01)
				{
					code_type = 1;							//UTF-16LE
					i += 13;
					tag_size -= 3;
				}
				else if (databuf[i + 10] == 0x00)
				{
					code_type = 0;							//iso-8859-1
					i += 11;
					tag_size -= 1;
				}
				else if (databuf[i + 10] == 0x03)
				{
					code_type = 3;							//UTF-8
					i += 11;
					tag_size -= 1;
				}
				TPE1 = (u8*)mymalloc(tag_size);

				if (code_type == 3)			//UTF-8
				{
					for (temp = 0; temp < tag_size;)
					{
						a = (databuf[i + temp] << 8) | databuf[i + temp + 1];
						if (a < 0x80) {				/* 7-bit */
							TPE1[temp++] = (BYTE)a;
						}
						else {
							if (a < 0x800) {		/* 11-bit */
								TPE1[temp++] = (BYTE)(0xC0 | a >> 6);
							}
							else {				/* 16-bit */
								TPE1[temp++] = (BYTE)(0xE0 | a >> 12);
								TPE1[temp++] = (BYTE)(0x80 | (a >> 6 & 0x3F));
							}
							TPE1[temp++] = (BYTE)(0x80 | (a & 0x3F));
						}
					}
				}
				else if (code_type == 2)			//UTF-16BE
				{
					temp2 = 0;
					for (temp = 0; temp < tag_size;)
					{
						a = (databuf[i + temp] << 8) | databuf[i + temp + 1];
						a = UNICODEtoGBK(a);
						temp1 = (a >> 8) & 0xff;
						if (temp1 != 0x00)
						{
							TPE1[temp2] = temp1;
							temp2++;
						}
						temp1 = a & 0xff;
						if (temp1 != 0x00)
						{
							TPE1[temp2] = temp1;
							temp2++;
						}
						temp += 2;
					}
				}
				else if (code_type == 1)			//UTF-16LE
				{
					temp2 = 0;
					for (temp = 0; temp < tag_size;)
					{
						a = (databuf[i + temp + 1] << 8) | databuf[i + temp];
						a = UNICODEtoGBK(a);
						temp1 = (a >> 8) & 0xff;
						if (temp1 != 0x00)
						{
							TPE1[temp2] = temp1;
							temp2++;
						}
						temp1 = a & 0xff;
						if (temp1 != 0x00)
						{
							TPE1[temp2] = temp1;
							temp2++;
						}
						temp += 2;
					}
				}
				else if (code_type == 0)			//iso-8859-1
				{
					for (temp = 0; temp < tag_size;)
					{
						temp1 = databuf[i + temp];
						if (temp1 != 0x00)
						{
							TPE1[temp] = temp1;
							temp++;
						}
						else
							temp++;
					}
				}
				img = 0;
			}
			else
				i++;
			if (i >= 4096)								//�Ҳ���λ��
			{
				TPE1 = 0;
				code_type = 4;
				img = 0;
			}
		}
		img = 1;
		i = 0;
		while (img)
		{
			if (databuf[i] == 0x41 && databuf[i + 1] == 0x50 && databuf[i + 2] == 0x49 && databuf[i + 3] == 0x43)
			{	//�ҵ�λ��
				if (databuf[i + 24] == 0xff && databuf[i + 25] == 0xd8 && databuf[i + 26] == 0xff && databuf[i + 27] == 0xe0)
				{
					code_type = 0; //JPG
				}
				if (databuf[i + 24] == 0x89 && databuf[i + 25] == 0x50 && databuf[i + 26] == 0x4e && databuf[i + 27] == 0x47)
				{
					code_type = 1; //PNG
				}
				img = 0;
			}
			else
				i++;
			if (i >= 4096)								//�Ҳ���λ��
			{
				code_type = 2;
				code_type = 2;
				img = 0;
			}
		}
		if (pic_show == 1 && code_type == 0)
		{
			if ((pic_show_x + pic_show_size) > picinfo.lcdwidth)return PIC_WINDOW_ERR;		//x���곬��Χ��.
			if ((pic_show_y + pic_show_size) > picinfo.lcdheight)return PIC_WINDOW_ERR;		//y���곬��Χ��.  
			//�õ���ʾ�����С	  	 
			picinfo.S_Height = pic_show_size;
			picinfo.S_Width = pic_show_size;
			//��ʾ������Ч
			if (picinfo.S_Height == 0 || picinfo.S_Width == 0)
			{
				picinfo.S_Height = lcddev.height;
				picinfo.S_Width = lcddev.width;
				return FALSE;
			}
			//��ʾ�Ŀ�ʼ�����
			picinfo.S_YOFF = pic_show_y;
			picinfo.S_XOFF = pic_show_x;

#if JPEG_USE_MALLOC == 1	//ʹ��malloc
			res = jpeg_mallocall();
#endif
			if (res == 0)
			{
				i += 14 + 20;
				f_lseek(fmp3, i);				//����ͷ

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
		else if (code_type == 1 && pic_show == 1)
		{
			i += 14 + 20;
			f_lseek(fmp3, i);				//����ͷ
		}
		else if (code_type == 2 && pic_show == 1)
		{
			LCD_Fill(pic_show_x, pic_show_y, pic_show_x + pic_show_size,
				pic_show_y + pic_show_size, BACK_COLOR);
		}
		f_close(fmp3);
		myfree(fmp3);
		myfree(databuf);				//�ͷ��ڴ�			    
		return size;
	}
	f_close(fmp3);
	myfree(fmp3);
	myfree(databuf);						//�ͷ��ڴ�			     
	return 0;
}








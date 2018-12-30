#include "sys.h" 
#include "fontupd.h"
#include "flash.h"
#include "lcd.h"
#include "text.h"	
#include "string.h"												   
						            
//code �ַ�ָ�뿪ʼ
//���ֿ��в��ҳ���ģ
//code �ַ����Ŀ�ʼ��ַ,GBK��
//mat  ���ݴ�ŵ�ַ size*2 bytes��С	 
void Get_HzMat(unsigned char *code, unsigned char *mat, u8 size)
{
	unsigned char qh, ql;
	unsigned char i;
	unsigned long foffset;
	qh = *code;
	ql = *(++code);
	if (qh < 0x81 || ql < 0x40 || ql == 0xff || qh == 0xff)//�� ���ú���
	{
		if (size == 12 || size == 16)
		{
			for (i = 0; i < (size * 2); i++)*mat++ = 0x00;//�������
			return; //��������
		}
		else if (size == 24)
		{
			for (i = 0; i < (size * 3); i++)*mat++ = 0x00;//�������
			return; //��������
		}
		else
		{
			for (i = 0; i < (size * 4); i++)*mat++ = 0x00;//�������
			return; //��������
		}

	}
	if (ql < 0x7f)ql -= 0x40;//ע��!
	else ql -= 0x41;
	qh -= 0x81;

	if (size == 12 || size == 16)
	{
		foffset = ((unsigned long)190 * qh + ql)*(size * 2);//�õ��ֿ��е��ֽ�ƫ���� 
	}
	else if (size == 24)
	{
		foffset = ((unsigned long)190 * qh + ql)*(size * 3);//�õ��ֿ��е��ֽ�ƫ���� 
	}
	else
	{
		foffset = ((unsigned long)190 * qh + ql)*(size * 4);//�õ��ֿ��е��ֽ�ƫ���� 
	}

	if (size == 16)
	{
		SPI_Flash_Read(mat, foffset + ftinfo.f16addr, 32);
	}
	else if (size == 12)
	{
		SPI_Flash_Read(mat, foffset + ftinfo.f12addr, 24);
	}
	else if (size == 24)
	{
		SPI_Flash_Read(mat, foffset + ftinfo.f24addr, 72);
	}
	else
	{
		SPI_Flash_Read(mat, foffset + ftinfo.f32addr, 128);
	}
}
//��ʾһ��ָ����С�ĺ���
//x,y :���ֵ�����
//font:����GBK��
//size:�����С
//mode:0,������ʾ,1,������ʾ	   
void Show_Font(u16 x, u16 y, u8 *font, u8 size, u8 mode)
{
	u8 temp, t, t1, size1;
	u16 y0 = y;
	u8 dzk[128];
	u16 tempcolor;
	if (size != 12 && size != 16 && size != 24 && size != 32)return;//��֧�ֵ�size
	Get_HzMat(font, dzk, size);//�õ���Ӧ��С�ĵ�������
	if (mode == 0)//������ʾ
	{
		if (size == 12 || size == 16)
		{
			size1 = size * 2;
		}
		else if (size == 24)
		{
			size1 = size * 3;
		}
		else
		{
			size1 = size * 4;
		}
		for (t = 0; t < size1; t++)
		{
			temp = dzk[t];//�õ�12����                          
			for (t1 = 0; t1 < 8; t1++)
			{
				if (temp & 0x80)LCD_DrawPoint(x, y);
				else
				{
					tempcolor = POINT_COLOR;
					POINT_COLOR = BACK_COLOR;
					LCD_DrawPoint(x, y);
					POINT_COLOR = tempcolor;//��ԭ
				}
				temp <<= 1;
				y++;
				if ((y - y0) == size)
				{
					y = y0;
					x++;
					break;
				}
			}
		}
	}
	else//������ʾ
	{
		if (size == 12 || size == 16)
		{
			size1 = size * 2;
		}
		else if (size == 24)
		{
			size1 = size * 3;
		}
		else
		{
			size1 = size * 4;
		}
		for (t = 0; t < size1; t++)
		{
			temp = dzk[t];//�õ�12����                          
			for (t1 = 0; t1 < 8; t1++)
			{
				if (temp & 0x80)LCD_DrawPoint(x, y);
				temp <<= 1;
				y++;
				if ((y - y0) == size)
				{
					y = y0;
					x++;
					break;
				}
			}
		}
	}
}
//��ָ��λ�ÿ�ʼ��ʾһ���ַ���	    
//֧���Զ�����
//(x,y):��ʼ����
//width,height:����
//str  :�ַ���
//size :�����С
//mode:0,�ǵ��ӷ�ʽ;1,���ӷ�ʽ    	   		   
void Show_Str(u16 x, u16 y, u16 width, u16 height, u8*str, u8 size, u8 mode)
{
	u16 x0 = x;
	u16 y0 = y;
	u8 bHz = 0;     //�ַ���������  	    				    				  	  
	while (*str != 0)//����δ����
	{
		if (!bHz)
		{
			if (*str > 0x80)bHz = 1;//���� 
			else              //�ַ�
			{      								//�ַ���ʱ����
				if (x > (x0 + width - size / 2))//����
				{
					y += size;
					x = x0;
				}
				if (y > (y0 + height - size))break;//Խ�緵��      
				if (*str == 13)//���з���
				{
					y += size;
					x = x0;
					str++;
				}
				else LCD_ShowChar(x, y, *str, size, mode);//��Ч����д�� 
				str++;
				x += size / 2; //�ַ�,Ϊȫ�ֵ�һ�� 
			}
		}
		else//���� 
		{
			bHz = 0;//�к��ֿ�    
			if (x > (x0 + width - size))//����
			{
				y += size;
				x = x0;
			}
			if (y > (y0 + height - size))break;//Խ�緵��  						     
			Show_Font(x, y, str, size, mode); //��ʾ�������,������ʾ 
			str += 2;
			x += size;//��һ������ƫ��	    
		}
	}
}
//��ָ����ȵ��м���ʾ�ַ���
//����ַ����ȳ�����len,����Show_Str��ʾ
//len:ָ��Ҫ��ʾ�Ŀ��			  
void Show_Str_Mid(u16 x, u16 y, u8*str, u8 size, u8 len)
{
	u16 strlenth = 0;
	strlenth = strlen((const char*)str);
	strlenth *= size / 2;
	if (strlenth > len)Show_Str(x, y, 240, 320, str, size, 1);
	else
	{
		strlenth = (len - strlenth) / 2;
		Show_Str(strlenth + x, y, 240, 320, str, size, 1);
	}
}

























		  







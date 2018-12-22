#include "sys.h"
#include "lcd.h"
#include "image2lcd.h"
//��8λ���ݻ��16λ��ɫ
//mode:0,��λ��ǰ,��λ�ں�.
//     1,��λ��ǰ,��λ�ں�.
//str:����
u16 image_getcolor(u8 mode, u8 *str)
{
	u16 color;
	if (mode)
	{
		color = ((u16)*str++) << 8;
		color |= *str;
	}
	else
	{
		color = *str++;
		color |= ((u16)*str) << 8;
	}
	return color;
}
//��Һ���ϻ�ͼ����֧�֣������ң����ϵ��� or ���ϵ��£������� ��ɨ�跽ʽ����	   
//xsta,ysta,width,height:��ͼ����
//scan:��image2lcd V2.9��˵��.
//*p:ͼ������
void image_show(u16 xsta, u16 ysta, u16 width, u16 height, u8 scan, u8 *p)
{
	u32 i;
	u32 len = 0;
	if ((scan & 0x03) == 0)//ˮƽɨ��
	{
		LCD_Scan_Dir(D2U_L2R);//���ϵ���,������
		LCD_Set_Window(xsta, ysta, width, height);
		LCD_SetCursor(xsta, ysta);//���ù��λ��  		

	}
	else  //��ֱɨ��
	{
		LCD_Scan_Dir(L2R_U2D);//������,���ϵ���
		LCD_Set_Window(xsta, ysta, width, height);
		LCD_SetCursor(xsta, ysta);//���ù��λ�� 
	}
	LCD_WriteRAM_Prepare();   	//��ʼд��GRAM
	len = width * height;			//д������ݳ���
	for (i = 0; i < len; i++)
	{
		LCD_WR_DATA(image_getcolor(scan&(1 << 4), p));
		p += 2;
	}
	LCD_Set_Window(0, 0, lcddev.width, lcddev.height);
}

//��ָ����λ����ʾһ��ͼƬ
//�˺���������ʾimage2lcd������ɵ�����16λ���ɫͼƬ.
//����:1,�ߴ粻�ܳ�����Ļ������.
//     2,��������ʱ���ܹ�ѡ:��λ��ǰ(MSB First)
//     3,�������ͼƬ��Ϣͷ����
//x,y:ָ��λ��
//imgx:ͼƬ����(�������ͼƬ��Ϣͷ,"4096ɫ/16λ���ɫ/18λ���ɫ/24λ���ɫ/32λ���ɫ����ͼ������ͷ)
//ע��:���STM32,����ѡ��image2lcd��"��λ��ǰ(MSB First)"ѡ��,����imginfo�����ݽ�����ȷ!!
void image_display(u16 x, u16 y, u8 * imgx)
{
	HEADCOLOR *imginfo;
	u8 ifosize = sizeof(HEADCOLOR);//�õ�HEADCOLOR�ṹ��Ĵ�С
	imginfo = (HEADCOLOR*)imgx;
	image_show(x, y, imginfo->w, imginfo->h, imginfo->scan, imgx + ifosize);
}
















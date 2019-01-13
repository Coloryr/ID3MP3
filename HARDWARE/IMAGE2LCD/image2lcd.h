#ifndef __IMAGE2LCD_H
#define __IMAGE2LCD_H
#include "sys.h"

//4096ɫ/16λ���ɫ/18λ���ɫ/24λ���ɫ/32λ���ɫ
//ͼ������ͷ�ṹ��  
__packed typedef struct _HEADCOLOR
{
	unsigned char scan;
	unsigned char gray;
	unsigned short w;
	unsigned short h;
	unsigned char is565;
	unsigned char rgb;
}HEADCOLOR;
//scan: ɨ��ģʽ 	    
//Bit7: 0:��������ɨ�裬1:��������ɨ�衣 
//Bit6: 0:�Զ�����ɨ�裬1:�Ե�����ɨ�衣 
//Bit5: 0:�ֽ����������ݴӸ�λ����λ���У�1:�ֽ����������ݴӵ�λ����λ���С� 
//Bit4: 0:WORD���͸ߵ�λ�ֽ�˳����PC��ͬ��1:WORD���͸ߵ�λ�ֽ�˳����PC�෴�� 
//Bit3~2: ������ 
//Bit1~0: [00]ˮƽɨ�裬[01]��ֱɨ�裬[10]����ˮƽ,�ֽڴ�ֱ��[11]���ݴ�ֱ,�ֽ�ˮƽ�� 
//gray: �Ҷ�ֵ 
//   �Ҷ�ֵ��1:��ɫ��2:�Ļң�4:ʮ���ң�8:256ɫ��12:4096ɫ��16:16λ��ɫ��24:24λ��ɫ��32:32λ��ɫ��
//w: ͼ��Ŀ�ȡ�	 
//h: ͼ��ĸ߶ȡ�
//is565: ��4096ɫģʽ��Ϊ0��ʾʹ��[16bits(WORD)]��ʽ����ʱͼ��������ÿ��WORD��ʾһ�����أ�Ϊ1��ʾʹ��[12bits(�����ֽ���)]��ʽ����ʱ�������е�ÿ12Bits����һ�����ء�
//��16λ��ɫģʽ��Ϊ0��ʾR G B��ɫ������ռ�õ�λ����Ϊ5Bits��Ϊ1��ʾR G B��ɫ������ռ�õ�λ���ֱ�Ϊ5Bits,6Bits,5Bits��
//��18λ��ɫģʽ��Ϊ0��ʾ"6Bits in Low Byte"��Ϊ1��ʾ"6Bits in High Byte"��
//��24λ��ɫ��32λ��ɫģʽ��is565��Ч��	  
//rgb: ����R G B��ɫ����������˳��rgb��ÿ2Bits��ʾһ����ɫ������[00]��ʾ�հף�[01]��ʾRed��[10]��ʾGreen��[11]��ʾBlue��

void image_display(u16 x,u16 y,u8 * imgx);//��ָ��λ����ʾͼƬ
void image_show(u16 xsta,u16 ysta,u16 xend,u16 yend,u8 scan,u8 *p);//��ָ������ʼ��ʾͼƬ
u16 image_getcolor(u8 mode,u8 *str);//��ȡ��ɫ


#endif














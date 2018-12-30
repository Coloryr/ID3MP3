#ifndef __JPEG_H__
#define __JPEG_H__
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//JPEG���� ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/19
//�汾��V1.0			   								  
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////�û�������//////////////////////////////////
#define JPEG_USE_MALLOC		1	//�����Ƿ�ʹ��malloc,��������ѡ��ʹ��malloc
//////////////////////////////////////////////END/////////////////////////////////////


#define M_SOF0  0xc0
#define M_DHT   0xc4
#define M_EOI   0xd9
#define M_SOS   0xda
#define M_DQT   0xdb
#define M_DRI   0xdd
#define M_APP0  0xe0  	  


#define W1 		2841 		// 2048*sqrt(2)*cos(1*pi/16) 
#define W2 		2676 		// 2048*sqrt(2)*cos(2*pi/16)
#define W3 		2408 		// 2048*sqrt(2)*cos(3*pi/16) 
#define W5 		1609 		// 2048*sqrt(2)*cos(5*pi/16) 
#define W6 		1108 		// 2048*sqrt(2)*cos(6*pi/16)
#define W7 		565  		// 2048*sqrt(2)*cos(7*pi/16)
												 
	  					    		  
#define MAKEWORD(a, b)		((WORD)(((u8)(a)) | ((WORD)((u8)(b))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
#define LOWORD(l)           ((WORD)(l))
#define HIWORD(l)           ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#define LOu8(w)           	((u8)(w))
#define HIu8(w)           	((u8)(((WORD)(w) >> 8) & 0xFF))
	   
//�궨��					   
#define WIDTHu8S(i)    ((i+31)/32*4)//
#define PI 3.1415926535				

#define PIC_FORMAT_ERR		0x27	//��ʽ����
#define PIC_SIZE_ERR		0x28	//ͼƬ�ߴ����
#define PIC_WINDOW_ERR		0x29	//�����趨����
#define PIC_MEM_ERR			0x11	//�ڴ����

//jpeg���뺯������ֵ����
#define FUNC_OK 0
#define FUNC_MEMORY_ERROR 1
#define FUNC_FILE_ERROR 2
#define FUNC_FORMAT_ERROR 3					    			       

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//u8 jpg_decode(const u8 *filename);					//����ĳ��jpeg/jpg�ļ�
u8 jpg_decode(const u8 *file);
int  jpeg_inittag(void);
void jpeg_inittable(void);					   		//��ʼ�����ݱ�
int  jpeg_decode(void);							   	//����
int  jpeg_decodemcublock(void);
int  jpeg_hufblock(u8 dchufindex,u8 achufindex);	//����������
int  jpeg_decodeelement(void);					  	//����һ������
void jpeg_iqtizzmcucomponent(short flag);		   	//������
void jpeg_iqtizzblock(short  *s ,short * d,short flag);
void jpeg_getyuv(short flag);		 				//ɫ��ת����ʵ��,�õ�ɫ�ʿռ�����
void jpeg_storebuffer(void);
u8   jpeg_readu8(void);             				//���ļ������ȡһ���ֽڳ���
void jpeg_initfastidct(void); 						//��ʼ������ɢ����Ҷ�任
void jpeg_fastidct(int * block);	 				//���ٷ���ɢ����Ҷ�任
void jpeg_idctrow(int * blk);
void jpeg_idctcol(int * blk);  									    	    
u8 jpeg_mallocall(void);
void jpeg_freeall(void);		 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif

















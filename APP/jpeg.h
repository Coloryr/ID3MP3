#ifndef __JPEG_H__
#define __JPEG_H__
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//JPEG解码 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/19
//版本：V1.0			   								  
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////用户配置区//////////////////////////////////
#define JPEG_USE_MALLOC		1	//定义是否使用malloc,这里我们选择使用malloc
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
	   
//宏定义					   
#define WIDTHu8S(i)    ((i+31)/32*4)//
#define PI 3.1415926535				

#define PIC_FORMAT_ERR		0x27	//格式错误
#define PIC_SIZE_ERR		0x28	//图片尺寸错误
#define PIC_WINDOW_ERR		0x29	//窗口设定错误
#define PIC_MEM_ERR			0x11	//内存错误

//jpeg解码函数返回值定义
#define FUNC_OK 0
#define FUNC_MEMORY_ERROR 1
#define FUNC_FILE_ERROR 2
#define FUNC_FORMAT_ERROR 3					    			       

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//u8 jpg_decode(const u8 *filename);					//解码某个jpeg/jpg文件
u8 jpg_decode(const u8 *file);
int  jpeg_inittag(void);
void jpeg_inittable(void);					   		//初始化数据表
int  jpeg_decode(void);							   	//解码
int  jpeg_decodemcublock(void);
int  jpeg_hufblock(u8 dchufindex,u8 achufindex);	//哈夫曼解码
int  jpeg_decodeelement(void);					  	//解码一个像素
void jpeg_iqtizzmcucomponent(short flag);		   	//反量化
void jpeg_iqtizzblock(short  *s ,short * d,short flag);
void jpeg_getyuv(short flag);		 				//色彩转换的实现,得到色彩空间数据
void jpeg_storebuffer(void);
u8   jpeg_readu8(void);             				//从文件里面读取一个字节出来
void jpeg_initfastidct(void); 						//初始化反离散傅立叶变换
void jpeg_fastidct(int * block);	 				//快速反离散傅立叶变换
void jpeg_idctrow(int * blk);
void jpeg_idctcol(int * blk);  									    	    
u8 jpeg_mallocall(void);
void jpeg_freeall(void);		 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif

















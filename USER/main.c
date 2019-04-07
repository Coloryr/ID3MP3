#include "sys.h" 
#include "lcd.h"
#include "sdio_sdcard.h" 
#include "ff.h" 
#include "flash.h"
#include "diskio.h"
#include "malloc.h"
#include "exfuns.h"
#include "app_start.h"
#include "includes.h"
#include "touch.h"
#include "delay.h"
#include "init.h"
#include "spi.h"
#include "key.h"
#include "fontupd.h"
#include "text.h"
#include "guix.h"
#include "piclib.h" 
#include "show.h" 
#include "WM.h"
#include "DIALOG.h"

static FIL PNGFile;

//static char pngBuffer[PNGPERLINESIZE];
/*******************************************************************
*
*       Static functions
*
********************************************************************
*/
/*********************************************************************
*
*       PngGetData
*
* Function description
*   This routine is called by GUI_PNG_DrawEx(). The routine is responsible
*   for setting the data pointer to a valid data location with at least
*   one valid byte.
*
* Parameters:
*   p           - Pointer to application defined data.
*   NumBytesReq - Number of bytes requested.
*   ppData      - Pointer to data pointer. This pointer should be set to
*                 a valid location.
*   StartOfFile - If this flag is 1, the data pointer should be set to the
*                 beginning of the data stream.
*
* Return value:
*   Number of data bytes available.
*/
static int PngGetData(void * p, const U8 ** ppData, unsigned NumBytesReq, U32 Off) 
{
	static int readaddress=0;
	FIL * phFile;
	U8 *pData;
	UINT NumBytesRead;
	#if SYSTEM_SUPPORT_UCOS
		OS_CPU_SR cpu_sr;
	#endif
	
	pData = (U8*)*ppData;
	phFile = (FIL *)p;
	
	//移动指针到应该读取的位置
	if(Off == 1) readaddress = 0;
	else readaddress=Off;
	
	#if SYSTEM_SUPPORT_UCOS
		OS_ENTER_CRITICAL();	//临界区
	#endif
	f_lseek(phFile,readaddress); 
	
	//读取数据到缓冲区中
	f_read(phFile,pData,NumBytesReq,&NumBytesRead);
	
		
	#if SYSTEM_SUPPORT_UCOS
		OS_EXIT_CRITICAL();	//退出临界区
	#endif
	return NumBytesRead;//返回读取到的字节数
}

//在指定位置显示无需加载到RAM中的PNG图片(需文件系统支持！对于小RAM，推荐使用此方法！PNG图片不能缩放！)
//PNGFileName:图片在SD卡或者其他存储设备中的路径
//mode:显示模式
//		0 在指定位置显示，有参数x,y确定显示位置
//		1 在LCD中间显示图片，当选择此模式的时候参数x,y无效。
//x:图片左上角在LCD中的x轴位置(当参数mode为1时，此参数无效)
//y:图片左上角在LCD中的y轴位置(当参数mode为1时，此参数无效)
//返回值:0 显示正常,其他 失败
int displaypngex(char *PNGFileName,u8 mode,u32 x,u32 y)
{
	char result;
	int XSize,YSize;

	result = f_open(&PNGFile,(const TCHAR*)PNGFileName,FA_READ);	//打开文件
	//文件打开错误
	if(result != FR_OK)	return 1;
	
	XSize = GUI_PNG_GetXSizeEx(PngGetData,&PNGFile);//PNG图片X大小
	YSize = GUI_PNG_GetYSizeEx(PngGetData,&PNGFile);//PNG图片Y大小
	switch(mode)
	{
		case 0:	//在指定位置显示图片
			GUI_PNG_DrawEx(PngGetData,&PNGFile,x,y);
			break;
		case 1:	//在LCD中间显示图片
			GUI_PNG_DrawEx(PngGetData,&PNGFile,(lcddev.width-XSize)/2-1,(lcddev.height-YSize)/2-1);
			break;
	}
	f_close(&PNGFile);	//关闭PNGFile文件
	return 0;
}

void test(void)
{
  BUTTON_SetDefaultSkin(BUTTON_SKIN_FLEX);
	CHECKBOX_SetDefaultSkin(CHECKBOX_SKIN_FLEX);
	DROPDOWN_SetDefaultSkin(DROPDOWN_SKIN_FLEX);
	FRAMEWIN_SetDefaultSkin(FRAMEWIN_SKIN_FLEX);
	HEADER_SetDefaultSkin(HEADER_SKIN_FLEX);
	MENU_SetDefaultSkin(MENU_SKIN_FLEX);
	MULTIPAGE_SetDefaultSkin(MULTIPAGE_SKIN_FLEX);
	PROGBAR_SetDefaultSkin(PROGBAR_SKIN_FLEX);
	RADIO_SetDefaultSkin(RADIO_SKIN_FLEX);
	SCROLLBAR_SetDefaultSkin(SCROLLBAR_SKIN_FLEX);
	SLIDER_SetDefaultSkin(SLIDER_SKIN_FLEX);
	SPINBOX_SetDefaultSkin(SPINBOX_SKIN_FLEX);
	GUI_CURSOR_Show();
	GUI_SetBkColor(GUI_BLUE);
	//GUI_SetFont(&GUI_FontHZ16);
	GUI_SetColor(GUI_RED);
	GUI_Clear();
	displaypngex("0:/test.png",0,0,0);
}

int main(void)
{
	delay_init(72);	    			//延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//中断分组配置
	init();										//GPIO等初始化
	PWM_Init(899,0);					//不分频。PWM频率=72000/(899+1)=80Khz 	
	TIM_SetCompare3(TIM3,500);
	LCD_Init();								//LCD初始化	
	mem_init();								//初始化内存池	 
	piclib_init();
	gui_init();
	SPI1_SetSpeed(SPI_BaudRatePrescaler_2);//设置为18M时钟,高速模式		
	//TP_Init();								//触摸屏初始化
	POINT_COLOR = RED;
	BACK_COLOR = BLACK;
	LCD_Clear(BLACK);//清屏 
	
	while (exfuns_init())			//为fatfs相关变量申请内存 					
	{
		LCD_ShowString(30, 20, 320, 16, 16,"Fatfs -> ERROR!");
	}
	while (SD_Init())					//SD卡初始化				
	{
		LCD_ShowString(30, 20, 320, 16, 16,"TFcard -> ERROR!");
	}
	
	f_mount(fs[0], "0:", 1); 	//挂载SD卡 
	f_mount(fs[1], "1:", 1); 	//挂载FLASH.

	if (font_init() == 1 || KEY_Scan(0) == 1)
	{
	a:
		update_font(30, 20, 16, 0);
		if (font_init() != 0)
		{
			goto a;
		}
		LCD_Clear(WHITE);//清屏
		TP_Adjust();  	//屏幕校准
		POINT_COLOR = RED;
		BACK_COLOR = BLACK;
	}
	if (KEY_Scan(0) == 2)
	{
		LCD_Clear(WHITE);//清屏
		TP_Adjust();  	//屏幕校准
		POINT_COLOR = RED;
		BACK_COLOR = BLACK;
		LCD_Clear(WHITE);//清屏
	}
	test();
	APP_start();
}














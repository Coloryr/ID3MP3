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
	
	//�ƶ�ָ�뵽Ӧ�ö�ȡ��λ��
	if(Off == 1) readaddress = 0;
	else readaddress=Off;
	
	#if SYSTEM_SUPPORT_UCOS
		OS_ENTER_CRITICAL();	//�ٽ���
	#endif
	f_lseek(phFile,readaddress); 
	
	//��ȡ���ݵ���������
	f_read(phFile,pData,NumBytesReq,&NumBytesRead);
	
		
	#if SYSTEM_SUPPORT_UCOS
		OS_EXIT_CRITICAL();	//�˳��ٽ���
	#endif
	return NumBytesRead;//���ض�ȡ�����ֽ���
}

//��ָ��λ����ʾ������ص�RAM�е�PNGͼƬ(���ļ�ϵͳ֧�֣�����СRAM���Ƽ�ʹ�ô˷�����PNGͼƬ�������ţ�)
//PNGFileName:ͼƬ��SD�����������洢�豸�е�·��
//mode:��ʾģʽ
//		0 ��ָ��λ����ʾ���в���x,yȷ����ʾλ��
//		1 ��LCD�м���ʾͼƬ����ѡ���ģʽ��ʱ�����x,y��Ч��
//x:ͼƬ���Ͻ���LCD�е�x��λ��(������modeΪ1ʱ���˲�����Ч)
//y:ͼƬ���Ͻ���LCD�е�y��λ��(������modeΪ1ʱ���˲�����Ч)
//����ֵ:0 ��ʾ����,���� ʧ��
int displaypngex(char *PNGFileName,u8 mode,u32 x,u32 y)
{
	char result;
	int XSize,YSize;

	result = f_open(&PNGFile,(const TCHAR*)PNGFileName,FA_READ);	//���ļ�
	//�ļ��򿪴���
	if(result != FR_OK)	return 1;
	
	XSize = GUI_PNG_GetXSizeEx(PngGetData,&PNGFile);//PNGͼƬX��С
	YSize = GUI_PNG_GetYSizeEx(PngGetData,&PNGFile);//PNGͼƬY��С
	switch(mode)
	{
		case 0:	//��ָ��λ����ʾͼƬ
			GUI_PNG_DrawEx(PngGetData,&PNGFile,x,y);
			break;
		case 1:	//��LCD�м���ʾͼƬ
			GUI_PNG_DrawEx(PngGetData,&PNGFile,(lcddev.width-XSize)/2-1,(lcddev.height-YSize)/2-1);
			break;
	}
	f_close(&PNGFile);	//�ر�PNGFile�ļ�
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
	delay_init(72);	    			//��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�жϷ�������
	init();										//GPIO�ȳ�ʼ��
	PWM_Init(899,0);					//����Ƶ��PWMƵ��=72000/(899+1)=80Khz 	
	TIM_SetCompare3(TIM3,500);
	LCD_Init();								//LCD��ʼ��	
	mem_init();								//��ʼ���ڴ��	 
	piclib_init();
	gui_init();
	SPI1_SetSpeed(SPI_BaudRatePrescaler_2);//����Ϊ18Mʱ��,����ģʽ		
	//TP_Init();								//��������ʼ��
	POINT_COLOR = RED;
	BACK_COLOR = BLACK;
	LCD_Clear(BLACK);//���� 
	
	while (exfuns_init())			//Ϊfatfs��ر��������ڴ� 					
	{
		LCD_ShowString(30, 20, 320, 16, 16,"Fatfs -> ERROR!");
	}
	while (SD_Init())					//SD����ʼ��				
	{
		LCD_ShowString(30, 20, 320, 16, 16,"TFcard -> ERROR!");
	}
	
	f_mount(fs[0], "0:", 1); 	//����SD�� 
	f_mount(fs[1], "1:", 1); 	//����FLASH.

	if (font_init() == 1 || KEY_Scan(0) == 1)
	{
	a:
		update_font(30, 20, 16, 0);
		if (font_init() != 0)
		{
			goto a;
		}
		LCD_Clear(WHITE);//����
		TP_Adjust();  	//��ĻУ׼
		POINT_COLOR = RED;
		BACK_COLOR = BLACK;
	}
	if (KEY_Scan(0) == 2)
	{
		LCD_Clear(WHITE);//����
		TP_Adjust();  	//��ĻУ׼
		POINT_COLOR = RED;
		BACK_COLOR = BLACK;
		LCD_Clear(WHITE);//����
	}
	test();
	APP_start();
}














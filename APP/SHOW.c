#include "show.h"
#include "lunar.h"
#include "image2lcd.h"
#include "image1.h"
#include "lcd.h"
#include "text.h"
#include "rtc.h"
#include "delay.h"
#include "key.h"

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







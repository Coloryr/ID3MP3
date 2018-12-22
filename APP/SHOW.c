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
/*显示生肖-----------------------------------------------------*/
void displaysx(void) {
	unsigned char LunarYearD, ReYear, tiangan, dizhi;        //?????????  ??????? 
	   //?????,??????,?c_moon????,year_moon???,month_moon,day_moon????BCD??
	LunarYearD = (moon.year_moon / 16) * 10 + moon.year_moon % 16;        //??????10??? 
	ReYear = LunarYearD % 12;                             //????12,????
	tiangan = LunarYearD % 10;         //??  ???.....
	dizhi = LunarYearD % 12;         //??  ???.....

	switch (tiangan)	//天干
	{
	case 0: Show_Str(10, 130, 24, 24, "庚", 24, 0); break;   //余0即整除 天干  庚
	case 1: Show_Str(10, 130, 24, 24, "辛", 24, 0); break;   //
	case 2: Show_Str(10, 130, 24, 24, "壬", 24, 0); break;
	case 3: Show_Str(10, 130, 24, 24, "癸", 24, 0); break;
	case 4: Show_Str(10, 130, 24, 24, "甲", 24, 0); break;
	case 5: Show_Str(10, 130, 24, 24, "乙", 24, 0); break;
	case 6: Show_Str(10, 130, 24, 24, "丙", 24, 0); break;
	case 7: Show_Str(10, 130, 24, 24, "丁", 24, 0); break;
	case 8: Show_Str(10, 130, 24, 24, "戊", 24, 0); break;
	case 9: Show_Str(10, 130, 24, 24, "已", 24, 0); break;
	}

	switch (dizhi) //地支
	{
	case 0: Show_Str(34, 130, 24, 24, "辰", 24, 0); break;   //余0即整除 地支 辰
	case 1: Show_Str(34, 130, 24, 24, "巳", 24, 0); break;   //
	case 2: Show_Str(34, 130, 24, 24, "午", 24, 0); break;
	case 3: Show_Str(34, 130, 24, 24, "未", 24, 0); break;
	case 4: Show_Str(34, 130, 24, 24, "申", 24, 0); break;
	case 5: Show_Str(34, 130, 24, 24, "酉", 24, 0); break;
	case 6: Show_Str(34, 130, 24, 24, "戌", 24, 0); break;
	case 7: Show_Str(34, 130, 24, 24, "亥", 24, 0); break;
	case 8: Show_Str(34, 130, 24, 24, "子", 24, 0); break;
	case 9: Show_Str(34, 130, 24, 24, "丑", 24, 0); break;
	case 10:Show_Str(34, 130, 24, 24, "寅", 24, 0); break;
	case 11:Show_Str(34, 130, 24, 24, "卯", 24, 0); break;
	}

	switch (ReYear)	//生肖
	{
	case 0: Show_Str(58, 130, 24, 24, "龙", 24, 0); break;   //余0即整除 农历 龙年
	case 1: Show_Str(58, 130, 24, 24, "蛇", 24, 0); break;   //蛇年
	case 2: Show_Str(58, 130, 24, 24, "马", 24, 0); break;
	case 3: Show_Str(58, 130, 24, 24, "羊", 24, 0); break;
	case 4: Show_Str(58, 130, 24, 24, "猴", 24, 0); break;
	case 5: Show_Str(58, 130, 24, 24, "鸡", 24, 0); break;
	case 6: Show_Str(58, 130, 24, 24, "狗", 24, 0); break;
	case 7: Show_Str(58, 130, 24, 24, "猪", 24, 0); break;
	case 8: Show_Str(58, 130, 24, 24, "鼠", 24, 0); break;
	case 9: Show_Str(58, 130, 24, 24, "牛", 24, 0); break;
	case 10:Show_Str(58, 130, 24, 24, "虎", 24, 0); break;
	case 11:Show_Str(58, 130, 24, 24, "兔", 24, 0); break;
	}
	Show_Str(84, 130, 24, 24, "年", 24, 0);
}

/*显示农历日期---------------------------------------------------------------*/
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
/*****	显示农历月份  *****/

	if (moon.month_moon == 1) { Show_Str(108, 130, 24, 24, "正", 24, 0); }
	if (moon.month_moon == 2) { Show_Str(108, 130, 24, 24, "二", 24, 0); }
	if (moon.month_moon == 3) { Show_Str(108, 130, 24, 24, "三", 24, 0); }
	if (moon.month_moon == 4) { Show_Str(108, 130, 24, 24, "四", 24, 0); }
	if (moon.month_moon == 5) { Show_Str(108, 130, 24, 24, "五", 24, 0); }
	if (moon.month_moon == 6) { Show_Str(108, 130, 24, 24, "六", 24, 0); }
	if (moon.month_moon == 7) { Show_Str(108, 130, 24, 24, "七", 24, 0); }
	if (moon.month_moon == 8) { Show_Str(108, 130, 24, 24, "八", 24, 0); }
	if (moon.month_moon == 9) { Show_Str(108, 130, 24, 24, "九", 24, 0); }
	if (moon.month_moon == 10) { Show_Str(108, 130, 24, 24, "十", 24, 0); }
	if (moon.month_moon == 11) { Show_Str(108, 130, 24, 24, "冬", 24, 0); }
	if (moon.month_moon == 12) { Show_Str(108, 130, 24, 24, "腊", 24, 0); }
	Show_Str(132, 130, 24, 24, "月", 24, 0);

	/*****	显示农历日的十位  *****/
	if (moon.day_moon / 10 == 0) { Show_Str(156, 130, 24, 24, "初", 24, 0); }    //
	if (moon.day_moon / 10 == 1)
		if (moon.day_moon / 10 == 1 & moon.day_moon % 10 == 0) { Show_Str(156, 130, 24, 24, "初", 24, 0); }
		else { Show_Str(156, 130, 24, 24, "十", 24, 0); }    //
	if (moon.day_moon / 10 == 2)
		if (moon.day_moon / 10 == 2 & moon.day_moon % 10 == 0) { Show_Str(156, 130, 24, 24, "二", 24, 0); }
		else { Show_Str(156, 130, 24, 24, "廿", 24, 0); }   //
	if (moon.day_moon / 10 == 3) { Show_Str(156, 130, 24, 24, "三", 24, 0); }	  //"三"字的代码

/*****	显示农历日的个位  *****/

	if (moon.day_moon % 10 == 1) { Show_Str(180, 130, 24, 24, "一", 24, 0); }    //
	if (moon.day_moon % 10 == 2) { Show_Str(180, 130, 24, 24, "二", 24, 0); }    //
	if (moon.day_moon % 10 == 3) { Show_Str(180, 130, 24, 24, "三", 24, 0); }	  //"三"字的代码
	if (moon.day_moon % 10 == 4) { Show_Str(180, 130, 24, 24, "四", 24, 0); }    //
	if (moon.day_moon % 10 == 5) { Show_Str(180, 130, 24, 24, "五", 24, 0); }    //
	if (moon.day_moon % 10 == 6) { Show_Str(180, 130, 24, 24, "六", 24, 0); }    //
	if (moon.day_moon % 10 == 7) { Show_Str(180, 130, 24, 24, "七", 24, 0); }    //
	if (moon.day_moon % 10 == 8) { Show_Str(180, 130, 24, 24, "八", 24, 0); }    //
	if (moon.day_moon % 10 == 9) { Show_Str(180, 130, 24, 24, "九", 24, 0); }    //
	if (moon.day_moon % 10 == 0) { Show_Str(180, 130, 24, 24, "十", 24, 0); }    //
}

/*----------显示节气---------------------------------------------*/
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
	if (j == 1) { Show_Str(204, 130, 96, 24, "今天小寒", 24, 0); }
	if (j == 2) { Show_Str(204, 130, 96, 24, "今天大寒", 24, 0); }
	if (j == 3) { Show_Str(204, 130, 96, 24, "今天立春", 24, 0); }
	if (j == 4) { Show_Str(204, 130, 96, 24, "今天雨水", 24, 0); }
	if (j == 5) { Show_Str(204, 130, 96, 24, "今天惊蛰", 24, 0); }
	if (j == 6) { Show_Str(204, 130, 96, 24, "今天春分", 24, 0); }
	if (j == 7) { Show_Str(204, 130, 96, 24, "今天清明", 24, 0); }
	if (j == 8) { Show_Str(204, 130, 96, 24, "今天谷雨", 24, 0); }
	if (j == 9) { Show_Str(204, 130, 96, 24, "今天立夏", 24, 0); }
	if (j == 10) { Show_Str(204, 130, 96, 24, "今天小满", 24, 0); }
	if (j == 11) { Show_Str(204, 130, 96, 24, "今天芒种", 24, 0); }
	if (j == 12) { Show_Str(204, 130, 96, 24, "今天夏至", 24, 0); }
	if (j == 13) { Show_Str(204, 130, 96, 24, "今天小暑", 24, 0); }
	if (j == 14) { Show_Str(204, 130, 96, 24, "今天大暑", 24, 0); }
	if (j == 15) { Show_Str(204, 130, 96, 24, "今天立秋", 24, 0); }
	if (j == 16) { Show_Str(204, 130, 96, 24, "今天处暑", 24, 0); }
	if (j == 17) { Show_Str(204, 130, 96, 24, "今天白露", 24, 0); }
	if (j == 18) { Show_Str(204, 130, 96, 24, "今天秋分", 24, 0); }
	if (j == 19) { Show_Str(204, 130, 96, 24, "今天寒露", 24, 0); }
	if (j == 20) { Show_Str(204, 130, 96, 24, "今天霜降", 24, 0); }
	if (j == 21) { Show_Str(204, 130, 96, 24, "今天立冬", 24, 0); }
	if (j == 22) { Show_Str(204, 130, 96, 24, "今天小雪", 24, 0); }
	if (j == 23) { Show_Str(204, 130, 96, 24, "今天大雪", 24, 0); }
	if (j == 24) { Show_Str(204, 130, 96, 24, "今天冬至", 24, 0); }
	if (j == 0) { Show_Str(216, 130, 96, 24, "无节气", 24, 0); }
}

void timeplay(void)
{
	Show_Str(58, 0, 24, 24, "年", 24, 0);
	Show_Str(106, 0, 24, 24, "月", 24, 0);
	Show_Str(154, 0, 24, 24, "日", 24, 0);
	Show_Str(180, 0, 48, 24, "星期", 24, 0);
	LCD_ShowxNum(10, 0, calendar.w_year, 4, 24, 0);//显示年
	LCD_ShowxNum(82, 0, calendar.w_month, 2, 24, 0);//显示月
	LCD_ShowxNum(130, 0, calendar.w_date, 2, 24, 0);//显示日

	switch (calendar.week)
	{
	case 0:
		Show_Font(228, 0, "天", 24, 0);
		break;
	case 1:
		Show_Font(228, 0, "一", 24, 0);
		break;
	case 2:
		Show_Font(228, 0, "二", 24, 0);
		break;
	case 3:
		Show_Font(228, 0, "三", 24, 0);
		break;
	case 4:
		Show_Font(228, 0, "四", 24, 0);
		break;
	case 5:
		Show_Font(228, 0, "五", 24, 0);
		break;
	case 6:
		Show_Font(228, 0, "六", 24, 0);
		break;
	}

	if (calendar.hour / 10 != 0)
	{
		image_display(0, 30, (u8*)gImage[calendar.hour / 10]);//在指定地址显示图?
	}
	else { image_display(0, 30, (u8*)gImage[11]); }

	image_display(50, 30, (u8*)gImage[calendar.hour % 10]);//在指定地址显示图片?

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
正常界面
**********************************************************/
void time_go(void)
{
	RTC_Get();
	timeplay();
	displaynl();
	displayjieqi();
	displaysx();
}







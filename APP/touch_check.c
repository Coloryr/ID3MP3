#include "touch.h"
#include "lcd.h"
#include "show.h"
#include "data.h"
#include "touch_check.h"
#include "vs10xx.h"
#include "mp3player.h"
#include "tjpgd.h"
#include "includes.h"
#include "key.h"

u8 button_check(void)
{
	//	static u8 check = 0;
	//static u16 pwmval = 300;
	u8 temp;
	/*
	tp_dev.scan(0);
	if (tp_dev.sta&TP_PRES_DOWN)			//触摸屏被按下
	{
		if (tp_dev.x[0] < lcddev.width&&tp_dev.y[0] < lcddev.height)
		{
			if (tp_dev.x[0] > 0 && tp_dev.x[0] <= 120 && tp_dev.y[0] > 0 & tp_dev.y[0] <= 120)
				check = 1;
			if (tp_dev.x[0] >= 121 && tp_dev.x[0] <= 240 && tp_dev.y[0] > 0 && tp_dev.y[0] <= 120)
				check = 2;
			if (tp_dev.x[0] > 0 && tp_dev.x[0] <= 120 && tp_dev.y[0] >= 121 & tp_dev.y[0] <= 240)
				check = 3;
			if (tp_dev.x[0] >= 121 && tp_dev.x[0] <= 240 && tp_dev.y[0] >= 121 & tp_dev.y[0] <= 240)
				check = 4;
			if (tp_dev.x[0] > 0 && tp_dev.x[0] <= 80 && tp_dev.y[0] >= 240 & tp_dev.y[0] <= 320)
				check = 5;
			if (tp_dev.x[0] >= 81 && tp_dev.x[0] <= 160 && tp_dev.y[0] >= 240 & tp_dev.y[0] <= 320)
				check = 6;
			if (tp_dev.x[0] >= 161 && tp_dev.x[0] <= 240 && tp_dev.y[0] >= 240 & tp_dev.y[0] <= 320)
				check = 7;
		}
	}
	else
	{
		if (check != 0)
		{
			switch (check)
			{
			case 1:
				temp = 2;		//上一曲
				break;
			case 2:
				temp = 1;		//下一曲
				break;
			case 3:
				temp = 5;		//随机					
				break;
			case 4:	   		
				if (lcd_bit == 0)
				{
					lcd_bit = 1;
					TIM_SetCompare3(TIM3,1000);
				}
				else if (lcd_bit == 1)
				{
					lcd_bit = 0;
					//TIM_SetCompare3(TIM3,pwmval);
				}
				break;
			case 5:
				if (vsset.mvol >= 200)
					vsset.mvol = 100;
				else
					vsset.mvol = vsset.mvol + 10;
				VS_Set_Vol(vsset.mvol);
				break;
			case 6:
				if (vsset.mvol <= 100)
					vsset.mvol = 200;
				else
					vsset.mvol = vsset.mvol - 10;
				VS_Set_Vol(vsset.mvol);
				break;
			case 7:
				if (write_bit == 0x10)
				{
					write_bit = 0x20;
					while (write_bit == 0x20);
				}
				LCD_Fill(pic_show_x, pic_show_y, pic_show_x + pic_show_size, pic_show_y + pic_show_size, BACK_COLOR);
				show_all(2);
				info.mode = 1;
				break;
			default:
				break;
			}
			check = 0;
			data_save_bit = 1;
			return temp;
		}
	}
	if (KEY_Scan(0) == 3)
	{
		return 8;
	}*/
	switch (KEY_Scan(0))
	{
	/*
	case 1:
		if (pwmval > 10)
			pwmval = pwmval - 10;
		data_save_bit = 1;
		TIM_SetCompare3(TIM3,pwmval);
		break;
	case 2:
		if (pwmval < 890)
			pwmval = pwmval + 10;
		data_save_bit = 1;
		TIM_SetCompare3(TIM3,pwmval);
		break;
		*/
	case 1:
		temp = 1; //下一曲
		return temp;
	case 2:
		temp = 5; //随机
		return temp;
	case 3:
		if (vsset.mvol >= 200)
			vsset.mvol = 100;
		else
			vsset.mvol = vsset.mvol + 10;
		VS_Set_Vol(vsset.mvol);
		break;
	case 4:
		if (lcd_bit == 0)
		{
			lcd_bit = 1;
			TIM_SetCompare3(TIM3, 1000);
		}
		else if (lcd_bit == 1)
		{
			lcd_bit = 0;
			TIM_SetCompare3(TIM3, 850);
		}
		break;
	}
	return 0;
}

void button_check1(void)
{
	static u8 check = 0;

	/*tp_dev.scan(0);
	if (tp_dev.sta&TP_PRES_DOWN)			//触摸屏被按下
	{
		if (tp_dev.x[0] < lcddev.width&&tp_dev.y[0] < lcddev.height)
		{
			if (tp_dev.x[0] > 1 && tp_dev.x[0] <= 120 && tp_dev.y[0] > 36 & tp_dev.y[0] <= 56)
				check = 1;
			if (tp_dev.x[0] > 1 && tp_dev.x[0] <= 120 && tp_dev.y[0] > 72 && tp_dev.y[0] <= 92)
				check = 2;
			if (tp_dev.x[0] > 1 && tp_dev.x[0] <= 120 && tp_dev.y[0] >= 100 & tp_dev.y[0] <= 120)
				check = 3;
			if (tp_dev.x[0] > 1 && tp_dev.x[0] <= 120 && tp_dev.y[0] >= 132 & tp_dev.y[0] <= 152)
				check = 4;
			if (tp_dev.x[0] > 1 && tp_dev.x[0] <= 120 && tp_dev.y[0] >= 164 & tp_dev.y[0] <= 184)
				check = 5;
			if (tp_dev.x[0] > 1 && tp_dev.x[0] <= 120 && tp_dev.y[0] >= 196 & tp_dev.y[0] <= 216)
				check = 6;
			if (tp_dev.x[0] > 121 && tp_dev.x[0] <= 240 && tp_dev.y[0] > 36 & tp_dev.y[0] <= 56)
				check = 7;
			if (tp_dev.x[0] > 121 && tp_dev.x[0] <= 240 && tp_dev.y[0] > 72 && tp_dev.y[0] <= 92)
				check = 8;
			if (tp_dev.x[0] > 121 && tp_dev.x[0] <= 240 && tp_dev.y[0] >= 100 & tp_dev.y[0] <= 120)
				check = 9;
			if (tp_dev.x[0] > 121 && tp_dev.x[0] <= 240 && tp_dev.y[0] >= 132 & tp_dev.y[0] <= 152)
				check = 10;
			if (tp_dev.x[0] > 121 && tp_dev.x[0] <= 240 && tp_dev.y[0] >= 164 & tp_dev.y[0] <= 184)
				check = 11;
			if (tp_dev.x[0] > 121 && tp_dev.x[0] <= 240 && tp_dev.y[0] >= 196 & tp_dev.y[0] <= 216)
				check = 12;
			if (tp_dev.x[0] >= 161 && tp_dev.x[0] <= 240 && tp_dev.y[0] >= 240 & tp_dev.y[0] <= 320)
				check = 13;
		}
	}	
	else
	{
	*/
	if (check != 0)
	{
		switch (check)
		{
		case 1:
			if (vsset.mvol <= 100)
				vsset.mvol = 200;
			else
				vsset.mvol = vsset.mvol - 10;
			break;
		case 2:
			if (vsset.bflimit == 0)
				vsset.bflimit = 15;
			else
				vsset.bflimit--;
			break;
		case 3:
			if (vsset.bass == 0)
				vsset.bass = 15;
			else
				vsset.bass--;
			break;
		case 4:
			if (vsset.tflimit == 0)
				vsset.tflimit = 15;
			else
				vsset.tflimit--;
			break;
		case 5:
			if (vsset.treble == 0)
				vsset.treble = 15;
			else
				vsset.treble--;
			break;
		case 6:
			if (vsset.effect == 0)
				vsset.effect = 3;
			else
				vsset.effect--;
			break;
		case 7:
			if (vsset.mvol >= 200)
				vsset.mvol = 100;
			else
				vsset.mvol = vsset.mvol + 10;
			break;
		case 8:
			if (vsset.bflimit == 15)
				vsset.bflimit = 0;
			else
				vsset.bflimit++;
			break;
		case 9:
			if (vsset.bass == 15)
				vsset.bass = 0;
			else
				vsset.bass++;
			break;
		case 10:
			if (vsset.tflimit == 15)
				vsset.tflimit = 0;
			else
				vsset.tflimit++;
			break;
		case 11:
			if (vsset.treble == 15)
				vsset.treble = 0;
			else
				vsset.treble++;
			break;
		case 12:
			if (vsset.effect == 3)
				vsset.effect = 0;
			else
				vsset.effect++;
			break;
		case 13:
			LCD_Fill(pic_show_x, pic_show_y, pic_show_x + pic_show_size, pic_show_y + pic_show_size, BACK_COLOR);
			f_lseek(fmp3_pic, info.pic_local); //还原指针
			if (info.pic_type == 0)			   //JPG
				info.pic_show = 1;
			else if (info.pic_type == 1) //PNG
				info.pic_show = 2;
			info.mode = 0;
			show_all(1);
			data_save_bit = 1;
			break;
		default:
			break;
		}
		if (check != 13)
			show_all(2);
		check = 0;
		VS_Set_All();
	}
	//}
}

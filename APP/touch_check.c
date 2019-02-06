#include "touch.h"
#include "lcd.h"
#include "touch_check.h"

u8 key_now = 0;

void button_check(void)
{
	static u8 check = 0;
	tp_dev.scan(0);
	if (tp_dev.sta&TP_PRES_DOWN)			//´¥ÃþÆÁ±»°´ÏÂ
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
			key_now = check;
			check = 0;
		}
	}
}

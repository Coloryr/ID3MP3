#ifndef __LUNAR_H_
#define __LUNAR_H_
#include "sys.h"

/*-------------------------------------月份数据表----------------------------------------*/
typedef struct
{
	u8 c_moon;
	u8 century;
	u8 year_moon;
	u8 month_moon;
	u8 day_moon;
	u8 week;
}_lunar_obj;
extern _lunar_obj moon;	//日历结构体
 

void Conversion(u8 c,unsigned char year,unsigned char month,unsigned char day);
unsigned char jieqi(unsigned char y2,unsigned char m2,unsigned char d2);

#endif

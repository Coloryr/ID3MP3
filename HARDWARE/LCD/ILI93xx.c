#include "lcd.h"
#include "stdlib.h"
#include "font.h"  
#include "delay.h"	   
			 
//LCD�Ļ�����ɫ�ͱ���ɫ	   
u16 POINT_COLOR=0x0000;	//������ɫ
u16 BACK_COLOR=0xFFFF;  //����ɫ 

//����LCD��Ҫ����
//Ĭ��Ϊ����
_lcd_dev lcddev;
	
		   
//д�Ĵ�������
//regval:�Ĵ���ֵ
void LCD_WR_REG(u16 regval)
{
	LCD->LCD_REG = regval;//д��Ҫд�ļĴ������	 
}
//дLCD����
//data:Ҫд���ֵ
void LCD_WR_DATA(u16 data)
{
	LCD->LCD_RAM = data;;
}
//��LCD����
//����ֵ:������ֵ
u16 LCD_RD_DATA(void)
{
	return LCD->LCD_RAM;
}
//д�Ĵ���
//LCD_Reg:�Ĵ�����ַ
//LCD_RegValue:Ҫд�������
void LCD_WriteReg(u8 LCD_Reg, u16 LCD_RegValue)
{
	LCD->LCD_REG = LCD_Reg;		//д��Ҫд�ļĴ������	 
	LCD->LCD_RAM = LCD_RegValue;//д������	 
}
//���Ĵ���
//LCD_Reg:�Ĵ�����ַ
//����ֵ:����������
u16 LCD_ReadReg(u8 LCD_Reg)
{
	LCD_WR_REG(LCD_Reg);		//д��Ҫ���ļĴ������
	delay_us(5);
	return LCD_RD_DATA();		//���ض�����ֵ
}
//��ʼдGRAM
void LCD_WriteRAM_Prepare(void)
{
	LCD->LCD_REG = lcddev.wramcmd;
}
//LCDдGRAM
//RGB_Code:��ɫֵ
void LCD_WriteRAM(u16 RGB_Code)
{
	LCD->LCD_RAM = RGB_Code;//дʮ��λGRAM
}
//��ILI93xx����������ΪGBR��ʽ��������д���ʱ��ΪRGB��ʽ��
//ͨ���ú���ת��
//c:GBR��ʽ����ɫֵ
//����ֵ��RGB��ʽ����ɫֵ
u16 LCD_BGR2RGB(u16 c)
{
	u16  r, g, b, rgb;
	b = (c >> 0) & 0x1f;
	g = (c >> 5) & 0x3f;
	r = (c >> 11) & 0x1f;
	rgb = (b << 11) + (g << 5) + (r << 0);
	return(rgb);
}
//��mdk -O1ʱ���Ż�ʱ��Ҫ����
//��ʱi
void opt_delay(u8 i)
{
	while (i--);
}
//��ȡ��ĳ�����ɫֵ	 
//x,y:����
//����ֵ:�˵����ɫ
u16 LCD_ReadPoint(u16 x, u16 y)
{
	u16 r = 0, g = 0, b = 0;
	if (x >= lcddev.width || y >= lcddev.height)return 0;	//�����˷�Χ,ֱ�ӷ���		   
	LCD_SetCursor(x, y);
	if (lcddev.id == 0X9341 || lcddev.id == 0X6804)LCD_WR_REG(0X2E);//9341/6804 ���Ͷ�GRAMָ��
	else LCD_WR_REG(R34);      		 				//����IC���Ͷ�GRAMָ��
	if (lcddev.id == 0X9320)opt_delay(2);				//FOR 9320,��ʱ2us	    
	if (LCD->LCD_RAM)r = 0;							//dummy Read	   
	opt_delay(2);
	r = LCD->LCD_RAM;  		  						//ʵ��������ɫ
	if (lcddev.id == 0X9341)//9341Ҫ��2�ζ���
	{
		opt_delay(2);
		b = LCD->LCD_RAM;
		g = r & 0XFF;//����9341,��һ�ζ�ȡ����RG��ֵ,R��ǰ,G�ں�,��ռ8λ
		g <<= 8;
	}
	else if (lcddev.id == 0X6804)r = LCD->LCD_RAM;//6804�ڶ��ζ�ȡ�Ĳ�����ʵֵ
	if (lcddev.id == 0X9325 || lcddev.id == 0X4535 || lcddev.id == 0X4531 || lcddev.id == 0X8989 || lcddev.id == 0XB505)return r;//�⼸��ICֱ�ӷ�����ɫֵ
	else if (lcddev.id == 0X9341)return (((r >> 11) << 11) | ((g >> 10) << 5) | (b >> 11));	//ILI9341��Ҫ��ʽת��һ��
	else return LCD_BGR2RGB(r);												//����IC
}
//LCD������ʾ
void LCD_DisplayOn(void)
{
	if (lcddev.id == 0X9341 || lcddev.id == 0X6804)LCD_WR_REG(0X29);	//������ʾ
	else LCD_WriteReg(R7, 0x0173); 			//������ʾ
}
//LCD�ر���ʾ
void LCD_DisplayOff(void)
{
	if (lcddev.id == 0X9341 || lcddev.id == 0X6804)LCD_WR_REG(0X28);	//�ر���ʾ
	else LCD_WriteReg(R7, 0x0);//�ر���ʾ 
}
//���ù��λ��
//Xpos:������
//Ypos:������
void LCD_SetCursor(u16 Xpos, u16 Ypos)
{
	if (lcddev.id == 0X9341 || lcddev.id == 0X6804)
	{
		LCD_WR_REG(lcddev.setxcmd);
		LCD_WR_DATA(Xpos >> 8);
		LCD_WR_DATA(Xpos & 0XFF);
		LCD_WR_REG(lcddev.setycmd);
		LCD_WR_DATA(Ypos >> 8);
		LCD_WR_DATA(Ypos & 0XFF);
	}
	else
	{
		if (lcddev.dir == 1)Xpos = lcddev.width - 1 - Xpos;//������ʵ���ǵ�תx,y����
		LCD_WriteReg(lcddev.setxcmd, Xpos);
		LCD_WriteReg(lcddev.setycmd, Ypos);
	}
}
//���ô���,���Զ����û������굽�������Ͻ�(sx,sy).
//sx,sy:������ʼ����(���Ͻ�)
//width,height:���ڿ�Ⱥ͸߶�,�������0!!
//�����С:width*height.
//68042,����ʱ��֧�ִ�������!! 
void LCD_Set_Window(u16 sx, u16 sy, u16 width, u16 height)
{
	u8 hsareg, heareg, vsareg, veareg;
	u16 hsaval, heaval, vsaval, veaval;
	width = sx + width - 1;
	height = sy + height - 1;

	if (lcddev.dir == 1)//����
	{
		//����ֵ
		hsaval = sy;
		heaval = height;
		vsaval = lcddev.width - width - 1;
		veaval = lcddev.width - sx - 1;
	}
	else
	{
		hsaval = sx;
		heaval = width;
		vsaval = sy;
		veaval = height;
	}
	hsareg = 0X50; heareg = 0X51;//ˮƽ���򴰿ڼĴ���
	vsareg = 0X52; veareg = 0X53;//��ֱ���򴰿ڼĴ���	  							  
//���üĴ���ֵ
	LCD_WriteReg(hsareg, hsaval);
	LCD_WriteReg(heareg, heaval);
	LCD_WriteReg(vsareg, vsaval);
	LCD_WriteReg(veareg, veaval);
	LCD_SetCursor(sx, sy);	//���ù��λ��
}
//����LCD���Զ�ɨ�跽��
//ע��:�����������ܻ��ܵ��˺������õ�Ӱ��(������9341/6804����������),
//����,һ������ΪL2R_U2D����,�������Ϊ����ɨ�跽ʽ,���ܵ�����ʾ������.
//dir:0~7,����8������(���嶨���lcd.h)
//9320/9325/9328/4531/4535/1505/b505/8989/5408/9341��IC�Ѿ�ʵ�ʲ���	   	   
void LCD_Scan_Dir(u8 dir)
{
	u16 regval = 0;
	u8 dirreg = 0;
	if (lcddev.dir == 1 && lcddev.id != 0X6804)//����ʱ����6804���ı�ɨ�跽��
	{
		switch (dir)//����ת��
		{
		case 0:dir = 6; break;
		case 1:dir = 7; break;
		case 2:dir = 4; break;
		case 3:dir = 5; break;
		case 4:dir = 1; break;
		case 5:dir = 0; break;
		case 6:dir = 3; break;
		case 7:dir = 2; break;
		}
	}
	if (lcddev.id == 0x9341 || lcddev.id == 0X6804)//9341/6804,������
	{
		switch (dir)
		{
		case L2R_U2D://������,���ϵ���
			regval |= (0 << 7) | (0 << 6) | (0 << 5);
			break;
		case L2R_D2U://������,���µ���
			regval |= (1 << 7) | (0 << 6) | (0 << 5);
			break;
		case R2L_U2D://���ҵ���,���ϵ���
			regval |= (0 << 7) | (1 << 6) | (0 << 5);
			break;
		case R2L_D2U://���ҵ���,���µ���
			regval |= (1 << 7) | (1 << 6) | (0 << 5);
			break;
		case U2D_L2R://���ϵ���,������
			regval |= (0 << 7) | (0 << 6) | (1 << 5);
			break;
		case U2D_R2L://���ϵ���,���ҵ���
			regval |= (0 << 7) | (1 << 6) | (1 << 5);
			break;
		case D2U_L2R://���µ���,������
			regval |= (1 << 7) | (0 << 6) | (1 << 5);
			break;
		case D2U_R2L://���µ���,���ҵ���
			regval |= (1 << 7) | (1 << 6) | (1 << 5);
			break;
		}
		dirreg = 0X36;
		regval |= 0X08;//BGR   
		if (lcddev.id == 0X6804)regval |= 0x02;//6804��BIT6��9341�ķ���	   
		LCD_WriteReg(dirreg, regval);
		if (regval & 0X20)
		{
			LCD_WR_REG(0x2A);
			LCD_WR_DATA(0); LCD_WR_DATA(0);
			LCD_WR_DATA((lcddev.height - 1) >> 8); LCD_WR_DATA((lcddev.height - 1) & 0XFF);
			LCD_WR_REG(0x2B);
			LCD_WR_DATA(0); LCD_WR_DATA(0);
			LCD_WR_DATA((lcddev.width - 1) >> 8); LCD_WR_DATA((lcddev.width - 1) & 0XFF);
		}
		else
		{
			LCD_WR_REG(0x2A);
			LCD_WR_DATA(0); LCD_WR_DATA(0);
			LCD_WR_DATA((lcddev.width - 1) >> 8); LCD_WR_DATA((lcddev.width - 1) & 0XFF);
			LCD_WR_REG(0x2B);
			LCD_WR_DATA(0); LCD_WR_DATA(0);
			LCD_WR_DATA((lcddev.height - 1) >> 8); LCD_WR_DATA((lcddev.height - 1) & 0XFF);
		}
	}
	else
	{
		switch (dir)
		{
		case L2R_U2D://������,���ϵ���
			regval |= (1 << 5) | (1 << 4) | (0 << 3);
			break;
		case L2R_D2U://������,���µ���
			regval |= (0 << 5) | (1 << 4) | (0 << 3);
			break;
		case R2L_U2D://���ҵ���,���ϵ���
			regval |= (1 << 5) | (0 << 4) | (0 << 3);
			break;
		case R2L_D2U://���ҵ���,���µ���
			regval |= (0 << 5) | (0 << 4) | (0 << 3);
			break;
		case U2D_L2R://���ϵ���,������
			regval |= (1 << 5) | (1 << 4) | (1 << 3);
			break;
		case U2D_R2L://���ϵ���,���ҵ���
			regval |= (1 << 5) | (0 << 4) | (1 << 3);
			break;
		case D2U_L2R://���µ���,������
			regval |= (0 << 5) | (1 << 4) | (1 << 3);
			break;
		case D2U_R2L://���µ���,���ҵ���
			regval |= (0 << 5) | (0 << 4) | (1 << 3);
			break;
		}
		if (lcddev.id == 0x8989)//8989 IC
		{
			dirreg = 0X11;
			regval |= 0X6040;	//65K   
		}
		else//��������IC		  
		{
			dirreg = 0X03;
			regval |= 1 << 12;
		}
		LCD_WriteReg(dirreg, regval);
	}
}
//����
//x,y:����
//POINT_COLOR:�˵����ɫ
void LCD_DrawPoint(u16 x, u16 y)
{
	LCD_SetCursor(x, y);		//���ù��λ�� 
	LCD_WriteRAM_Prepare();	//��ʼд��GRAM
	LCD->LCD_RAM = POINT_COLOR;
}
//���ٻ���
//x,y:����
//color:��ɫ
void LCD_Fast_DrawPoint(u16 x, u16 y, u16 color)
{
	if (lcddev.id == 0X9341 || lcddev.id == 0X6804)
	{
		LCD_WR_REG(lcddev.setxcmd);
		LCD_WR_DATA(x >> 8);
		LCD_WR_DATA(x & 0XFF);
		LCD_WR_REG(lcddev.setycmd);
		LCD_WR_DATA(y >> 8);
		LCD_WR_DATA(y & 0XFF);
	}
	else
	{
		if (lcddev.dir == 1)x = lcddev.width - 1 - x;//������ʵ���ǵ�תx,y����
		LCD_WriteReg(lcddev.setxcmd, x);
		LCD_WriteReg(lcddev.setycmd, y);
	}
	LCD->LCD_REG = lcddev.wramcmd;
	LCD->LCD_RAM = color;
}

//����LCD��ʾ����6804��֧�ֺ�����ʾ��
//dir:0,������1,����
void LCD_Display_Dir(u8 dir)
{
	if (dir == 0)//����
	{
		lcddev.dir = 0;//����
		lcddev.width = 240;
		lcddev.height = 320;
		if (lcddev.id == 0X9341 || lcddev.id == 0X6804)
		{
			lcddev.wramcmd = 0X2C;
			lcddev.setxcmd = 0X2A;
			lcddev.setycmd = 0X2B;
			if (lcddev.id == 0X6804)
			{
				lcddev.width = 320;
				lcddev.height = 480;
			}
		}
		else if (lcddev.id == 0X8989)
		{
			lcddev.wramcmd = R34;
			lcddev.setxcmd = 0X4E;
			lcddev.setycmd = 0X4F;
		}
		else
		{
			lcddev.wramcmd = R34;
			lcddev.setxcmd = R32;
			lcddev.setycmd = R33;
		}
	}
	else if (lcddev.id != 0X6804)//6804��֧�ֺ�����ʾ	
	{
		lcddev.dir = 1;//����
		lcddev.width = 320;
		lcddev.height = 240;
		if (lcddev.id == 0X9341)
		{
			lcddev.wramcmd = 0X2C;
			lcddev.setxcmd = 0X2A;
			lcddev.setycmd = 0X2B;
		}
		else if (lcddev.id == 0X8989)
		{
			lcddev.wramcmd = R34;
			lcddev.setxcmd = 0X4F;
			lcddev.setycmd = 0X4E;
		}
		else
		{
			lcddev.wramcmd = R34;
			lcddev.setxcmd = R33;
			lcddev.setycmd = R32;
		}
	}
	LCD_Scan_Dir(DFT_SCAN_DIR);	//Ĭ��ɨ�跽��
}
//��ʼ��lcd
//�ó�ʼ���������Գ�ʼ������ILI93XXҺ��,�������������ǻ���ILI9320��!!!
//�������ͺŵ�����оƬ��û�в���! 
void LCD_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
	FSMC_NORSRAMTimingInitTypeDef  readWriteTiming;
	FSMC_NORSRAMTimingInitTypeDef  writeTiming;


	/* Enable the FSMC AND GPIO Clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;         //LCD �������
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;      //LCD-RST
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* Set PD.00(D2), PD.01(D3), PD.04(NOE/RD), PD.05(NWE/WR), PD.08(D13), PD.09(D14),
	  PD.10(D15), PD.14(D0), PD.15(D1) as alternate function push pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 |
		GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_14 |
		GPIO_Pin_15;
		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* Set PE.07(D4), PE.08(D5), PE.09(D6), PE.10(D7), PE.11(D8), PE.12(D9), PE.13(D10),
	   PE.14(D11), PE.15(D12) as alternate function push pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
		GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 |
		GPIO_Pin_15;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* CS ΪFSMC_NE1(PD7) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* RS ΪFSMC_A16(PD11)*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_SetBits(GPIOD, GPIO_Pin_7);            //CS=1 
	GPIO_SetBits(GPIOD, GPIO_Pin_11);           //RS=1
	GPIO_SetBits(GPIOD, GPIO_Pin_14 | GPIO_Pin_15 | GPIO_Pin_0 | GPIO_Pin_1);
	GPIO_SetBits(GPIOE, GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10);
	GPIO_SetBits(GPIOE, GPIO_Pin_0);            //LIGHT��
	GPIO_SetBits(GPIOE, GPIO_Pin_1);            //RESET=1
	GPIO_SetBits(GPIOD, GPIO_Pin_4);            //RD=1
	GPIO_SetBits(GPIOD, GPIO_Pin_5);            //WR=1

	readWriteTiming.FSMC_AddressSetupTime = 0x01;	 //��ַ����ʱ�䣨ADDSET��Ϊ2��HCLK 1/36M=27ns
	readWriteTiming.FSMC_AddressHoldTime = 0x00;	 //��ַ����ʱ�䣨ADDHLD��ģʽAδ�õ�	
	readWriteTiming.FSMC_DataSetupTime = 0x0f;		 // ���ݱ���ʱ��Ϊ16��HCLK,��ΪҺ������IC�Ķ����ݵ�ʱ���ٶȲ���̫�죬�����1289���IC��
	readWriteTiming.FSMC_BusTurnAroundDuration = 0x00;
	readWriteTiming.FSMC_CLKDivision = 0x00;
	readWriteTiming.FSMC_DataLatency = 0x00;
	readWriteTiming.FSMC_AccessMode = FSMC_AccessMode_A;	 //ģʽB


	writeTiming.FSMC_AddressSetupTime = 0x00;	 //��ַ����ʱ�䣨ADDSET��Ϊ1��HCLK  
	writeTiming.FSMC_AddressHoldTime = 0x00;	 //��ַ����ʱ�䣨A		
	writeTiming.FSMC_DataSetupTime = 0x03;		 ////���ݱ���ʱ��Ϊ4��HCLK	
	writeTiming.FSMC_BusTurnAroundDuration = 0x00;
	writeTiming.FSMC_CLKDivision = 0x00;
	writeTiming.FSMC_DataLatency = 0x00;
	writeTiming.FSMC_AccessMode = FSMC_AccessMode_A;	 //ģʽA 


	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;//  ��������ʹ��NE1��
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable; // ���������ݵ�ַ
	FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_NOR;// FSMC_MemoryType_NOR;  //NOR  
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;//�洢�����ݿ��Ϊ16bit   
	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;// FSMC_BurstAccessMode_Disable; 
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;	//  �洢��дʹ��
	FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Enable; // ��дʹ�ò�ͬ��ʱ��
	FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &readWriteTiming; //��дʱ��
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &writeTiming;  //дʱ��

	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);  //��ʼ��FSMC����

	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);  // ʹ��BANK1 

	delay_ms(50); // delay 50 ms 
	LCD_WriteReg(0x0000, 0x0001);
	delay_ms(50); // delay 50 ms 
	lcddev.id = LCD_ReadReg(0x0000);
	if (lcddev.id == 0x9328)//ILI9328   OK  
	{
		LCD_WriteReg(0x00EC, 0x108F);// internal timeing      
		LCD_WriteReg(0x00EF, 0x1234);// ADD        
		//LCD_WriteReg(0x00e7,0x0010);      
		//LCD_WriteReg(0x0000,0x0001);//�����ڲ�ʱ��
		LCD_WriteReg(0x0001, 0x0100);
		LCD_WriteReg(0x0002, 0x0700);//��Դ����                    
		//LCD_WriteReg(0x0003,(1<<3)|(1<<4) ); 	//65K  RGB
		//DRIVE TABLE(�Ĵ��� 03H)
		//BIT3=AM BIT4:5=ID0:1
		//AM ID0 ID1   FUNCATION
		// 0  0   0	   R->L D->U
		// 1  0   0	   D->U	R->L
		// 0  1   0	   L->R D->U
		// 1  1   0    D->U	L->R
		// 0  0   1	   R->L U->D
		// 1  0   1    U->D	R->L
		// 0  1   1    L->R U->D �����������.
		// 1  1   1	   U->D	L->R
		LCD_WriteReg(0x0003, (1 << 12) | (3 << 4) | (0 << 3));//65K    
		LCD_WriteReg(0x0004, 0x0000);
		LCD_WriteReg(0x0008, 0x0202);
		LCD_WriteReg(0x0009, 0x0000);
		LCD_WriteReg(0x000a, 0x0000);//display setting         
		LCD_WriteReg(0x000c, 0x0001);//display setting          
		LCD_WriteReg(0x000d, 0x0000);//0f3c          
		LCD_WriteReg(0x000f, 0x0000);
		//��Դ����
		LCD_WriteReg(0x0010, 0x0000);
		LCD_WriteReg(0x0011, 0x0007);
		LCD_WriteReg(0x0012, 0x0000);
		LCD_WriteReg(0x0013, 0x0000);
		LCD_WriteReg(0x0007, 0x0001);
		delay_ms(50);
		LCD_WriteReg(0x0010, 0x1490);
		LCD_WriteReg(0x0011, 0x0227);
		delay_ms(50);
		LCD_WriteReg(0x0012, 0x008A);
		delay_ms(50);
		LCD_WriteReg(0x0013, 0x1a00);
		LCD_WriteReg(0x0029, 0x0006);
		LCD_WriteReg(0x002b, 0x000d);
		delay_ms(50);
		LCD_WriteReg(0x0020, 0x0000);
		LCD_WriteReg(0x0021, 0x0000);
		delay_ms(50);
		//٤��У��
		LCD_WriteReg(0x0030, 0x0000);
		LCD_WriteReg(0x0031, 0x0604);
		LCD_WriteReg(0x0032, 0x0305);
		LCD_WriteReg(0x0035, 0x0000);
		LCD_WriteReg(0x0036, 0x0C09);
		LCD_WriteReg(0x0037, 0x0204);
		LCD_WriteReg(0x0038, 0x0301);
		LCD_WriteReg(0x0039, 0x0707);
		LCD_WriteReg(0x003c, 0x0000);
		LCD_WriteReg(0x003d, 0x0a0a);
		delay_ms(50);
		LCD_WriteReg(0x0050, 0x0000); //ˮƽGRAM��ʼλ�� 
		LCD_WriteReg(0x0051, 0x00ef); //ˮƽGRAM��ֹλ��                    
		LCD_WriteReg(0x0052, 0x0000); //��ֱGRAM��ʼλ��                    
		LCD_WriteReg(0x0053, 0x013f); //��ֱGRAM��ֹλ��  

		LCD_WriteReg(0x0060, 0xa700);
		LCD_WriteReg(0x0061, 0x0001);
		LCD_WriteReg(0x006a, 0x0000);
		LCD_WriteReg(0x0080, 0x0000);
		LCD_WriteReg(0x0081, 0x0000);
		LCD_WriteReg(0x0082, 0x0000);
		LCD_WriteReg(0x0083, 0x0000);
		LCD_WriteReg(0x0084, 0x0000);
		LCD_WriteReg(0x0085, 0x0000);

		LCD_WriteReg(0x0090, 0x0010);
		LCD_WriteReg(0x0092, 0x0600);
		//������ʾ����    
		LCD_WriteReg(0x0007, 0x0133);
	}
	LCD_Display_Dir(1);		 	//Ĭ��Ϊ����
	LCD_LED = 1;					//��������
	LCD_Clear(BLACK);
}
//��������
//color:Ҫ���������ɫ
void LCD_Clear(u16 color)
{
	u32 index = 0;
	u32 totalpoint = lcddev.width;
	totalpoint *= lcddev.height; 	//�õ��ܵ���
	LCD_SetCursor(0x00, 0x0000);	//���ù��λ�� 
	LCD_WriteRAM_Prepare();     //��ʼд��GRAM	 	  
	for (index = 0; index < totalpoint; index++)
	{
		LCD->LCD_RAM = color;
	}
}
//��ָ����������䵥����ɫ
//(sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex-sx+1)*(ey-sy+1)   
//color:Ҫ������ɫ
void LCD_Fill(u16 sx, u16 sy, u16 ex, u16 ey, u16 color)
{
	u16 i, j;
	u16 xlen = 0;
	xlen = ex - sx + 1;
	for (i = sy; i <= ey; i++)
	{
		LCD_SetCursor(sx, i);      				//���ù��λ�� 
		LCD_WriteRAM_Prepare();     			//��ʼд��GRAM	  
		for (j = 0; j < xlen; j++)LCD_WR_DATA(color);	//���ù��λ�� 	    
	}
}
//��ָ�����������ָ����ɫ��			 
//(sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex-sx+1)*(ey-sy+1)   
//color:Ҫ������ɫ
void LCD_Color_Fill(u16 sx, u16 sy, u16 ex, u16 ey, u16 *color)
{
	u16 height, width;
	u16 i, j;
	width = ex - sx + 1; 		//�õ����Ŀ��
	height = ey - sy + 1;		//�߶�
	for (i = 0; i < height; i++)
	{
		LCD_SetCursor(sx, sy + i);   	//���ù��λ�� 
		LCD_WriteRAM_Prepare();     //��ʼд��GRAM
		for (j = 0; j < width; j++)LCD->LCD_RAM = color[i*height + j];//д������ 
	}
}
//����
//x1,y1:�������
//x2,y2:�յ�����  
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2)
{
	u16 t;
	int xerr = 0, yerr = 0, delta_x, delta_y, distance;
	int incx, incy, uRow, uCol;
	delta_x = x2 - x1; //������������ 
	delta_y = y2 - y1;
	uRow = x1;
	uCol = y1;
	if (delta_x > 0)incx = 1; //���õ������� 
	else if (delta_x == 0)incx = 0;//��ֱ�� 
	else { incx = -1; delta_x = -delta_x; }
	if (delta_y > 0)incy = 1;
	else if (delta_y == 0)incy = 0;//ˮƽ�� 
	else { incy = -1; delta_y = -delta_y; }
	if (delta_x > delta_y)distance = delta_x; //ѡȡ�������������� 
	else distance = delta_y;
	for (t = 0; t <= distance + 1; t++)//������� 
	{
		LCD_DrawPoint(uRow, uCol);//���� 
		xerr += delta_x;
		yerr += delta_y;
		if (xerr > distance)
		{
			xerr -= distance;
			uRow += incx;
		}
		if (yerr > distance)
		{
			yerr -= distance;
			uCol += incy;
		}
	}
}
//������	  
//(x1,y1),(x2,y2):���εĶԽ�����
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2)
{
	LCD_DrawLine(x1, y1, x2, y1);
	LCD_DrawLine(x1, y1, x1, y2);
	LCD_DrawLine(x1, y2, x2, y2);
	LCD_DrawLine(x2, y1, x2, y2);
}
//��ָ��λ�û�һ��ָ����С��Բ
//(x,y):���ĵ�
//r    :�뾶
void Draw_Circle(u16 x0, u16 y0, u8 r)
{
	int a, b;
	int di;
	a = 0; b = r;
	di = 3 - (r << 1);             //�ж��¸���λ�õı�־
	while (a <= b)
	{
		LCD_DrawPoint(x0 + a, y0 - b);             //5
		LCD_DrawPoint(x0 + b, y0 - a);             //0           
		LCD_DrawPoint(x0 + b, y0 + a);             //4               
		LCD_DrawPoint(x0 + a, y0 + b);             //6 
		LCD_DrawPoint(x0 - a, y0 + b);             //1       
		LCD_DrawPoint(x0 - b, y0 + a);
		LCD_DrawPoint(x0 - a, y0 - b);             //2             
		LCD_DrawPoint(x0 - b, y0 - a);             //7     	         
		a++;
		//ʹ��Bresenham�㷨��Բ     
		if (di < 0)di += 4 * a + 6;
		else
		{
			di += 10 + 4 * (a - b);
			b--;
		}
	}
}
//��ָ��λ����ʾһ���ַ�
//x,y:��ʼ����
//num:Ҫ��ʾ���ַ�:" "--->"~"
//size:�����С 12/16
//mode:���ӷ�ʽ(1)���Ƿǵ��ӷ�ʽ(0)
void LCD_ShowChar(u16 x, u16 y, u8 num, u8 size, u8 mode)
{
	u8 temp, t1, t;
	u16 y0 = y;
	u16 colortemp = POINT_COLOR;
	//���ô���		   
	num = num - ' ';//�õ�ƫ�ƺ��ֵ
	if (!mode) //�ǵ��ӷ�ʽ
	{
		for (t = 0; t < size; t++)
		{
			if (size == 12)temp = asc2_1206[num][t];  //����1206����
			else temp = asc2_1608[num][t];		 //����1608���� 	                          
			for (t1 = 0; t1 < 8; t1++)
			{
				if (temp & 0x80)POINT_COLOR = colortemp;
				else POINT_COLOR = BACK_COLOR;
				LCD_DrawPoint(x, y);
				temp <<= 1;
				y++;
				if (x >= lcddev.height) { POINT_COLOR = colortemp; return; }//��������
				if ((y - y0) == size)
				{
					y = y0;
					x++;
					if (x >= lcddev.width) { POINT_COLOR = colortemp; return; }//��������
					break;
				}
			}
		}
	}
	else//���ӷ�ʽ
	{
		for (t = 0; t < size; t++)
		{
			if (size == 12)temp = asc2_1206[num][t];  //����1206����
			else temp = asc2_1608[num][t];		 //����1608���� 	                          
			for (t1 = 0; t1 < 8; t1++)
			{
				if (temp & 0x80)LCD_DrawPoint(x, y);
				temp <<= 1;
				y++;
				if (x >= lcddev.height) { POINT_COLOR = colortemp; return; }//��������
				if ((y - y0) == size)
				{
					y = y0;
					x++;
					if (x >= lcddev.width) { POINT_COLOR = colortemp; return; }//��������
					break;
				}
			}
		}
	}
	POINT_COLOR = colortemp;
}
//m^n����
//����ֵ:m^n�η�.
u32 LCD_Pow(u8 m, u8 n)
{
	u32 result = 1;
	while (n--)result *= m;
	return result;
}
//��ʾ����,��λΪ0,����ʾ
//x,y :�������	 
//len :���ֵ�λ��
//size:�����С
//color:��ɫ 
//num:��ֵ(0~4294967295);	 
void LCD_ShowNum(u16 x, u16 y, u32 num, u8 len, u8 size)
{
	u8 t, temp;
	u8 enshow = 0;
	for (t = 0; t < len; t++)
	{
		temp = (num / LCD_Pow(10, len - t - 1)) % 10;
		if (enshow == 0 && t < (len - 1))
		{
			if (temp == 0)
			{
				LCD_ShowChar(x + (size / 2)*t, y, ' ', size, 0);
				continue;
			}
			else enshow = 1;

		}
		LCD_ShowChar(x + (size / 2)*t, y, temp + '0', size, 0);
	}
}
//��ʾ����,��λΪ0,������ʾ
//x,y:�������
//num:��ֵ(0~999999999);	 
//len:����(��Ҫ��ʾ��λ��)
//size:�����С
//mode:
//[7]:0,�����;1,���0.
//[6:1]:����
//[0]:0,�ǵ�����ʾ;1,������ʾ.
void LCD_ShowxNum(u16 x, u16 y, u32 num, u8 len, u8 size, u8 mode)
{
	u8 t, temp;
	u8 enshow = 0;
	for (t = 0; t < len; t++)
	{
		temp = (num / LCD_Pow(10, len - t - 1)) % 10;
		if (enshow == 0 && t < (len - 1))
		{
			if (temp == 0)
			{
				if (mode & 0X80)LCD_ShowChar(x + (size / 2)*t, y, '0', size, mode & 0X01);
				else LCD_ShowChar(x + (size / 2)*t, y, ' ', size, mode & 0X01);
				continue;
			}
			else enshow = 1;

		}
		LCD_ShowChar(x + (size / 2)*t, y, temp + '0', size, mode & 0X01);
	}
}
//��ʾ�ַ���
//x,y:�������
//width,height:�����С  
//size:�����С
//*p:�ַ�����ʼ��ַ		  
void LCD_ShowString(u16 x, u16 y, u16 width, u16 height, u8 size, u8 *p)
{
	u8 x0 = x;
	width += x;
	height += y;
	while ((*p <= '~') && (*p >= ' '))//�ж��ǲ��ǷǷ��ַ�!
	{
		if (x >= width) { x = x0; y += size; }
		if (y >= height)break;//�˳�
		LCD_ShowChar(x, y, *p, size, 0);
		x += size / 2;
		p++;
	}
}































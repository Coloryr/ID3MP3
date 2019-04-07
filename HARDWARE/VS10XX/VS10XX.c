#include "vs10xx.h"
#include "includes.h"
#include "spi.h"

_vs10xx_obj vsset =
	{
		160, //����:160
		10,  //�������� 150Hz
		15,  //�������� 15dB
		10,  //�������� 10Khz
		0,   //�������� 0dB
		0,   //�ռ�Ч��
};

////////////////////////////////////////////////////////////////////////////////
//��ֲʱ��Ľӿ�
//data:Ҫд�������
//����ֵ:����������
u8 VS_SPI_ReadWriteByte(u8 data)
{
	return SPI1_ReadWriteByte(data);
}
//SD����ʼ����ʱ��,��Ҫ����
void VS_SPI_SpeedLow(void)
{
	SPI1_SetSpeed(SPI_BaudRatePrescaler_32); //���õ�����ģʽ
}
//SD������������ʱ��,���Ը�����
void VS_SPI_SpeedHigh(void)
{
	SPI1_SetSpeed(SPI_BaudRatePrescaler_8); //���õ�����ģʽ
}

////////////////////////////////////////////////////////////////////////////////
//��λVS10XX
void VS_Soft_Reset(void)
{
	u8 retry = 0;
	while (VS_DQ == 0)
		;						//�ȴ������λ����
	VS_SPI_ReadWriteByte(0Xff); //��������
	retry = 0;
	while (VS_RD_Reg(SPI_MODE) != 0x0800) // �����λ,��ģʽ
	{
		VS_WR_Cmd(SPI_MODE, 0x0804); // �����λ,��ģʽ
		delay_ms(2);				 //�ȴ�����1.35ms
		if (retry++ > 100)
			break;
	}
	while (VS_DQ == 0)
		; //�ȴ������λ����
	retry = 0;
	while (VS_RD_Reg(SPI_CLOCKF) != 0X9800) //����VS10XX��ʱ��,3��Ƶ ,1.5xADD
	{
		VS_WR_Cmd(SPI_CLOCKF, 0X9800); //����VS10XX��ʱ��,3��Ƶ ,1.5xADD
		if (retry++ > 100)
			break;
	}
	delay_ms(20);
}
//Ӳ��λMP3
//����1:��λʧ��;0:��λ�ɹ�
u8 VS_HD_Reset(void)
{
	u8 retry = 0;
	VS_RST = 0;
	delay_ms(20);
	VS_XDCS = 1; //ȡ�����ݴ���
	VS_XCS = 1;  //ȡ�����ݴ���
	VS_RST = 1;
	while (VS_DQ == 0 && retry < 200) //�ȴ�DREQΪ��
	{
		retry++;
		delay_us(50);
	};
	delay_ms(20);
	if (retry >= 200)
		return 1;
	else
		return 0;
}
//���Ҳ���
void VS_Sine_Test(void)
{
	VS_HD_Reset();
	VS_Soft_Reset();
	VS_Set_Vol(100);
	VS_WR_Cmd(SPI_MODE, 0x0820); //����VS10XX�Ĳ���ģʽ
	while (VS_DQ == 0)
		; //�ȴ�DREQΪ��
	//��VS10XX�������Ҳ������0x53 0xef 0x6e n 0x00 0x00 0x00 0x00
	//����n = 0x24, �趨VS10XX�����������Ҳ���Ƶ��ֵ��������㷽����VS10XX��datasheet
	VS_SPI_SpeedLow(); //����
	VS_XDCS = 0;	   //ѡ�����ݴ���
	VS_SPI_ReadWriteByte(0x53);
	VS_SPI_ReadWriteByte(0xef);
	VS_SPI_ReadWriteByte(0x6e);
	VS_SPI_ReadWriteByte(0x24);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	delay_ms(100);
	VS_XDCS = 1;
	//�˳����Ҳ���
	VS_XDCS = 0; //ѡ�����ݴ���
	VS_SPI_ReadWriteByte(0x45);
	VS_SPI_ReadWriteByte(0x78);
	VS_SPI_ReadWriteByte(0x69);
	VS_SPI_ReadWriteByte(0x74);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	delay_ms(100);
	VS_XDCS = 1;

	//�ٴν������Ҳ��Բ�����nֵΪ0x44���������Ҳ���Ƶ������Ϊ�����ֵ
	VS_XDCS = 0; //ѡ�����ݴ���
	VS_SPI_ReadWriteByte(0x53);
	VS_SPI_ReadWriteByte(0xef);
	VS_SPI_ReadWriteByte(0x6e);
	VS_SPI_ReadWriteByte(0x44);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	delay_ms(100);
	VS_XDCS = 1;
	//�˳����Ҳ���
	VS_XDCS = 0; //ѡ�����ݴ���
	VS_SPI_ReadWriteByte(0x45);
	VS_SPI_ReadWriteByte(0x78);
	VS_SPI_ReadWriteByte(0x69);
	VS_SPI_ReadWriteByte(0x74);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	delay_ms(100);
	VS_XDCS = 1;
}
//ram ����
//����ֵ:RAM���Խ��
// VS1003����õ���ֵΪ0x807F����������;VS1053Ϊ0X83FF.
u16 VS_Ram_Test(void)
{
	VS_HD_Reset();
	VS_WR_Cmd(SPI_MODE, 0x0820); // ����VS10XX�Ĳ���ģʽ
	while (VS_DQ == 0)
		;			   // �ȴ�DREQΪ��
	VS_SPI_SpeedLow(); //����
	VS_XDCS = 0;	   // xDCS = 1��ѡ��VS10XX�����ݽӿ�
	VS_SPI_ReadWriteByte(0x4d);
	VS_SPI_ReadWriteByte(0xea);
	VS_SPI_ReadWriteByte(0x6d);
	VS_SPI_ReadWriteByte(0x54);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	delay_ms(150);
	VS_XDCS = 1;
	return VS_RD_Reg(SPI_HDAT0); // VS1003����õ���ֵΪ0x807F����������;VS1053Ϊ0X83FF.;
}
//��VS10XXд����
//address:�����ַ
//data:��������
void VS_WR_Cmd(u8 address, u16 data)
{
	while (VS_DQ == 0)
		;			   //�ȴ�����
	VS_SPI_SpeedLow(); //����
	VS_XDCS = 1;
	VS_XCS = 0;
	VS_SPI_ReadWriteByte(VS_WRITE_COMMAND); //����VS10XX��д����
	VS_SPI_ReadWriteByte(address);			//��ַ
	VS_SPI_ReadWriteByte(data >> 8);		//���͸߰�λ
	VS_SPI_ReadWriteByte(data);				//�ڰ�λ
	VS_XCS = 1;
	VS_SPI_SpeedHigh(); //����
}
//��VS10XXд����
//data:Ҫд�������
void VS_WR_Data(u8 data)
{
	VS_SPI_SpeedHigh(); //����,��VS1003B,���ֵ���ܳ���36.864/4Mhz����������Ϊ9M
	VS_XDCS = 0;
	VS_SPI_ReadWriteByte(data);
	VS_XDCS = 1;
}
//��VS10XX�ļĴ���
//address���Ĵ�����ַ
//����ֵ��������ֵ
//ע�ⲻҪ�ñ��ٶ�ȡ,�����
u16 VS_RD_Reg(u8 address)
{
	u16 temp = 0;
	while (VS_DQ == 0)
		;			   //�ǵȴ�����״̬
	VS_SPI_SpeedLow(); //����
	VS_XDCS = 1;
	VS_XCS = 0;
	VS_SPI_ReadWriteByte(VS_READ_COMMAND); //����VS10XX�Ķ�����
	VS_SPI_ReadWriteByte(address);		   //��ַ
	temp = VS_SPI_ReadWriteByte(0xff);	 //��ȡ���ֽ�
	temp = temp << 8;
	temp += VS_SPI_ReadWriteByte(0xff); //��ȡ���ֽ�
	VS_XCS = 1;
	VS_SPI_SpeedHigh(); //����
	return temp;
}
//��ȡVS10xx��RAM
//addr��RAM��ַ
//����ֵ��������ֵ
u16 VS_WRAM_Read(u16 addr)
{
	u16 res;
	CPU_SR_ALLOC();
	OS_CRITICAL_ENTER(); //�����ٽ���
	VS_WR_Cmd(SPI_WRAMADDR, addr);
	res = VS_RD_Reg(SPI_WRAM);
	OS_CRITICAL_EXIT(); //�˳��ٽ���
	return res;
}
//���ò����ٶȣ���VS1053��Ч��
//t:0,1,�����ٶ�;2,2���ٶ�;3,3���ٶ�;4,4����;�Դ�����
void VS_Set_Speed(u8 t)
{
	VS_WR_Cmd(SPI_WRAMADDR, 0X1E04); //�ٶȿ��Ƶ�ַ
	while (VS_DQ == 0)
		;					//�ȴ�����
	VS_WR_Cmd(SPI_WRAM, t); //д�벥���ٶ�
}
//FOR WAV HEAD0 :0X7761 HEAD1:0X7665
//FOR MIDI HEAD0 :other info HEAD1:0X4D54
//FOR WMA HEAD0 :data speed HEAD1:0X574D
//FOR MP3 HEAD0 :data speed HEAD1:ID
//������Ԥ��ֵ,�ײ�III
const u16 bitrate[2][16] =
	{
		{0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0},
		{0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0}};
//����Kbps�Ĵ�С
//����ֵ���õ�������
u16 VS_Get_HeadInfo(void)
{
	unsigned int HEAD0;
	unsigned int HEAD1;
	CPU_SR_ALLOC();
	OS_CRITICAL_ENTER(); //�����ٽ���
	HEAD0 = VS_RD_Reg(SPI_HDAT0);
	HEAD1 = VS_RD_Reg(SPI_HDAT1);
	OS_CRITICAL_EXIT(); //�˳��ٽ���
						//printf("(H0,H1):%x,%x\n",HEAD0,HEAD1);
	switch (HEAD1)
	{
	case 0x7665: //WAV��ʽ
	case 0X4D54: //MIDI��ʽ
	case 0X4154: //AAC_ADTS
	case 0X4144: //AAC_ADIF
	case 0X4D34: //AAC_MP4/M4A
	case 0X4F67: //OGG
	case 0X574D: //WMA��ʽ
	case 0X664C: //FLAC��ʽ
	{
		////printf("HEAD0:%d\n",HEAD0);
		HEAD1 = HEAD0 * 2 / 25; //�൱��*8/100
		if ((HEAD1 % 10) > 5)
			return HEAD1 / 10 + 1; //��С�����һλ��������
		else
			return HEAD1 / 10;
	}
	default: //MP3��ʽ,�����˽ײ�III�ı�
	{
		HEAD1 >>= 3;
		HEAD1 = HEAD1 & 0x03;
		if (HEAD1 == 3)
			HEAD1 = 1;
		else
			HEAD1 = 0;
		return bitrate[HEAD1][HEAD0 >> 12];
	}
	}
}
//�õ�ƽ���ֽ���
//����ֵ��ƽ���ֽ����ٶ�
u32 VS_Get_ByteRate(void)
{
	return VS_WRAM_Read(0X1E05); //ƽ��λ��
}
//�õ���Ҫ��������
//����ֵ:��Ҫ��������
u16 VS_Get_EndFillByte(void)
{
	return VS_WRAM_Read(0X1E06); //����ֽ�
}
//����һ����Ƶ����
//�̶�Ϊ32�ֽ�
//����ֵ:0,���ͳɹ�
//		 1,VS10xx��ȱ����,��������δ�ɹ�����
u8 VS_Send_MusicData(u8 *buf)
{
	u8 n;
	if (VS_DQ != 0) //�����ݸ�VS10XX
	{
		VS_XDCS = 0;
		for (n = 0; n < 32; n++)
		{
			VS_SPI_ReadWriteByte(buf[n]);
		}
		VS_XDCS = 1;
	}
	else
		return 1;
	return 0; //�ɹ�������
}
//�и�
//ͨ���˺����и裬��������л���������
void VS_Restart_Play(void)
{
	u16 temp;
	u16 i;
	u8 n;
	u8 vsbuf[32];
	for (n = 0; n < 32; n++)
		vsbuf[n] = 0;			//����
	temp = VS_RD_Reg(SPI_MODE); //��ȡSPI_MODE������
	temp |= 1 << 3;				//����SM_CANCELλ
	temp |= 1 << 2;				//����SM_LAYER12λ,������MP1,MP2
	VS_WR_Cmd(SPI_MODE, temp);  //����ȡ����ǰ����ָ��
	for (i = 0; i < 2048;)		//����2048��0,�ڼ��ȡSM_CANCELλ.���Ϊ0,���ʾ�Ѿ�ȡ���˵�ǰ����
	{
		if (VS_Send_MusicData(vsbuf) == 0) //ÿ����32���ֽں���һ��
		{
			i += 32;					//������32���ֽ�
			temp = VS_RD_Reg(SPI_MODE); //��ȡSPI_MODE������
			if ((temp & (1 << 3)) == 0)
				break; //�ɹ�ȡ����
		}
	}
	if (i < 2048) //SM_CANCEL����
	{
		temp = VS_Get_EndFillByte() & 0xff; //��ȡ����ֽ�
		for (n = 0; n < 32; n++)
			vsbuf[n] = temp; //����ֽڷ�������
		for (i = 0; i < 2052;)
		{
			if (VS_Send_MusicData(vsbuf) == 0)
				i += 32; //���
		}
	}
	else
		VS_Soft_Reset(); //SM_CANCEL���ɹ�,�����,��Ҫ��λ
	temp = VS_RD_Reg(SPI_HDAT0);
	temp += VS_RD_Reg(SPI_HDAT1);
	if (temp) //��λ,����û�гɹ�ȡ��,��ɱ���,Ӳ��λ
	{
		VS_HD_Reset();   //Ӳ��λ
		VS_Soft_Reset(); //��λ
	}
}
//�������ʱ��
void VS_Reset_DecodeTime(void)
{
	VS_WR_Cmd(SPI_DECODE_TIME, 0x0000);
	VS_WR_Cmd(SPI_DECODE_TIME, 0x0000); //��������
}
//�õ�mp3�Ĳ���ʱ��n sec
//����ֵ������ʱ��
u16 VS_Get_DecodeTime(void)
{
	u16 dt = 0;
	CPU_SR_ALLOC();
	OS_CRITICAL_ENTER(); //�����ٽ���
	dt = VS_RD_Reg(SPI_DECODE_TIME);
	OS_CRITICAL_EXIT(); //�˳��ٽ���
	return dt;
}
//vs10xxװ��patch.
//patch��patch�׵�ַ
//len��patch����
void VS_Load_Patch(u16 *patch, u16 len)
{
	u16 i;
	u16 addr, n, val;
	for (i = 0; i < len;)
	{
		addr = patch[i++];
		n = patch[i++];
		if (n & 0x8000U) //RLE run, replicate n samples
		{
			n &= 0x7FFF;
			val = patch[i++];
			while (n--)
				VS_WR_Cmd(addr, val);
		}
		else //copy run, copy n sample
		{
			while (n--)
			{
				val = patch[i++];
				VS_WR_Cmd(addr, val);
			}
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////
//Ƶ����ʾ����
#define SPEC_DATA_BASE 0X1810 //0X1380 for VS1011
//�õ�Ƶ������
//*specbuf:Ƶ�����ݻ�����
//����ֵ:Ƶ����.
u8 VS_Get_Spec(u16 *p)
{
	u8 bands;
	u8 i;
	CPU_SR_ALLOC();
	OS_CRITICAL_ENTER(); //�����ٽ���
	VS_WR_Cmd(SPI_WRAMADDR, SPEC_DATA_BASE + 2);
	bands = VS_RD_Reg(SPI_WRAM); //��ȡƵ����
	VS_WR_Cmd(SPI_WRAMADDR, SPEC_DATA_BASE + 4);
	for (i = 0; i < bands; i++)
	{
		//������Ƶ�����ݷ�Ϊ2����,��ЧλΪ12λ.��Χ����0~31
		//[5:0]:��ǰֵ
		//[11:6]:��ֵ
		*p++ = VS_RD_Reg(SPI_WRAM); //��ȡ��ǰֵ�ͷ�ֵ
	}
	OS_CRITICAL_EXIT(); //�˳��ٽ���
	return bands;
}

//�趨�µ�����Ƶ��
//buf:����Ƶ��ֵ
//bands:buf�Ĵ�С.��VS1053���Ϊ15
void VS_Set_Bands(u16 *buf, u8 bands)
{
	u8 i;
	CPU_SR_ALLOC();
	OS_CRITICAL_ENTER();							//�����ٽ���
	VS_WR_Cmd(SPI_WRAMADDR, SPEC_DATA_BASE + 0X58); //��ַ����1868,��VS1053,SPEC_DATA_BASE��0X1810.���Լ���0X58
	for (i = 0; i < bands; i++)
	{
		VS_WR_Cmd(SPI_WRAM, buf[i]); //����Ƶ������
	}
	if (i < 15)
		VS_WR_Cmd(SPI_WRAM, 25000);				 //��VS1053������ֵΪ15,���25000,��ʾƵ�ױ����
	VS_WR_Cmd(SPI_WRAMADDR, SPEC_DATA_BASE + 1); //��ַSPEC_DATA_BASE+1,ΪSamples Rates����ʼ��ַ
	VS_WR_Cmd(SPI_WRAM, 0);						 //��ʼ��Ƶ��
	OS_CRITICAL_EXIT();							 //�˳��ٽ���
}
///////////////////////////////////////////////////////////////////////////////////////////////////
//�趨VS10XX���ŵ������͸ߵ���
//volx:������С(0~254)
void VS_Set_Vol(u8 volx)
{
	u16 volt = 0; //�ݴ�����ֵ
	CPU_SR_ALLOC();
	volt = 254 - volx; //ȡ��һ��,�õ����ֵ,��ʾ���ı�ʾ
	volt <<= 8;
	volt += 254 - volx;		  //�õ��������ú��С
	OS_CRITICAL_ENTER();	  //�����ٽ���
	VS_WR_Cmd(SPI_VOL, volt); //������
	OS_CRITICAL_EXIT();		  //�˳��ٽ���
}
//�趨�ߵ�������
//bfreq:��Ƶ����Ƶ��	2~15(��λ:10Hz)
//bass:��Ƶ����			0~15(��λ:1dB)
//tfreq:��Ƶ����Ƶ�� 	1~15(��λ:Khz)
//treble:��Ƶ����  	 	0~15(��λ:1.5dB,С��9��ʱ��Ϊ����)
void VS_Set_Bass(u8 bfreq, u8 bass, u8 tfreq, u8 treble)
{
	u16 bass_set = 0; //�ݴ������Ĵ���ֵ
	signed char temp = 0;
	CPU_SR_ALLOC();
	if (treble == 0)
		temp = 0; //�任
	else if (treble > 8)
		temp = treble - 8;
	else
		temp = treble - 9;
	bass_set = temp & 0X0F; //�����趨
	bass_set <<= 4;
	bass_set += tfreq & 0xf; //��������Ƶ��
	bass_set <<= 4;
	bass_set += bass & 0xf; //�����趨
	bass_set <<= 4;
	bass_set += bfreq & 0xf;	   //��������
	OS_CRITICAL_ENTER();		   //�����ٽ���
	VS_WR_Cmd(SPI_BASS, bass_set); //BASS
	OS_CRITICAL_EXIT();			   //�˳��ٽ���
}
//�趨��Ч
//eft:0,�ر�;1,��С;2,�е�;3,���.
void VS_Set_Effect(u8 eft)
{
	u16 temp;
	CPU_SR_ALLOC();
	OS_CRITICAL_ENTER();		//�����ٽ���
	temp = VS_RD_Reg(SPI_MODE); //��ȡSPI_MODE������
	if (eft & 0X01)
		temp |= 1 << 4; //�趨LO
	else
		temp &= ~(1 << 5); //ȡ��LO
	if (eft & 0X02)
		temp |= 1 << 7; //�趨HO
	else
		temp &= ~(1 << 7);	 //ȡ��HO
	VS_WR_Cmd(SPI_MODE, temp); //�趨ģʽ
	OS_CRITICAL_EXIT();		   //�˳��ٽ���
}
///////////////////////////////////////////////////////////////////////////////
//��������,��Ч��.
void VS_Set_All(void)
{
	VS_Set_Vol(vsset.mvol);
	VS_Set_Bass(vsset.bflimit, vsset.bass, vsset.tflimit, vsset.treble);
	VS_Set_Effect(vsset.effect);
}

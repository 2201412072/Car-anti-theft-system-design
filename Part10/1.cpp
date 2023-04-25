#include <iostream>
using namespace std;

int GBPCON;
int CPDCON;

void lcdpininit() 
{ //��ʼ��lcd����
	GBPCON &= ~0x3;//��GBPCON�Ͷ�λ����
	GPBCON |= 0x01;//�����һλ���1
	//��ʼ����������GPB0
	GPCCON = 0xaaaaaaaa;
	GPDCON = 0xaaaaaaaa;
	//��ʼ��LCDר������
	GPGCON |= (3 << 8);
	//����
}

void lcdctrlinit()
{
	int CLKVAL = 100 / VCLK / 2 - 1;
	//����ʱ�ӷ�Ƶֵ
	int bppmod = 0xd;
	int LCDCON1;
	int LCDCON2;
	int LCDCON3;
	int LCDCON4;
	int LCDCON5;
	LCDCON1 = (CLKVAL << 8) | (3 << 5) | (BPPMODE << 1);
	LCDCON2 = (VBPD < 24) | (LINEVAL << 14) | (VFPD << 6) | (VSPW << 0);
	LCDCON3 = (HBPD << 19) | (HOZVAL << 8) | (HFPD << 0);
	LCDCON4 = (HSPW << 0);
	LCDCON5 = (0 << 10) | (0 << 9) | (0 << 8) | (0 << 7) | (0 << 6) | (0 << 5);
	//��ʼ��LCD�������Ĵ�����
	int addr = fb_base & ~(1 << 31);
	//��ʼ����ַ�����ҵ�ַ���λ��Ϊ0
	addr = fb_base + xres * yres * 4;
	addr >>= 1;		 
	addr &= 0x1fffff;  	
	LCDSADDR2 = addr;
}

void lcdconuse(void) {
	GPBDAT |= (1 << 0);
	// pwren��LCD�ṩAVDD
	LCDCON5 |= (1 << 3);
	//LCDCON1'BIT 0 : ����LCD�������Ƿ�����ź�
	LCDCON1 |= (1 << 0);
}

void lcdconstop(void) {
	GPBDAT &= ~(1 << 0);
	// pwren	��LCD�ṩAVDD
	LCDCON5 &= ~(1 << 3);
	// LCDCON1'BIT 0 : ����LCD�������Ƿ�����ź�
	LCDCON1 &= ~(1 << 0);
}

void getc() {

}

extern const unsigned char fontdata_8x16[];
//ASCII�ַ�����
void fbshowchar(int x, int y, char c, unsigned char red, char green, char blue) {
	int i, j;
	//�����ʾ����
	get_lcd_params(&fb_base, &xres, &yres);
	// ���c��ascii��ĵ�������
	unsigned char* dots = &fontdata_8x16[c * 16];
	unsigned char data;   //��һ�е�������
	int bit;
	for (j = y; j < y + 16; j++) { //��ȡÿ������
		data = *dots++;
		bit = 7;
		for (i = x; i < x + 8; i++) {
			//���ݵ����ĳλ�����ص���ɫ����д��FB
			if (data & (1 << bit))				fb_put_pixel(i, j, red, green, blue);
			bit--;
		}
	}
}

void fbshowpixal(int i, int j, unsigned char red, char green, char blue) {
	unsigned int* pc;
	unsigned int pixel_base;
	pixel_base = fb_base + (xres * j + i) * 4;
	pc = (unsigned int*)pixel_base;
	*pc = (0 << 24) | ((red << 16) | (green << 8) | blue;//���ݲ�ͬ��ֵȷ�������ɫ
}

//�����жϳ�ʼ��
void KeyINT_Init(void) {
	rGPFCON = (rGPFCON & 0xF3FF) | (2 << 10);
	rGPECON = (rGPECON & 0xF3FFFFFF) | (0x01 << 26); //����GPE13Ϊ���λ��ģ��ʱ�����
	rEXTINT0 &= 0xff8fffff; //�ⲿ�ж�5�͵�ƽ��Ч
	if (rEINTPEND & 0x20)   //����ⲿ�ж�5���жϹ���λ
		rEINTPEND |= 0x20;
	if ((rINTPND & BIT_EINT4_7)) {
		rSRCPND |= BIT_EINT4_7;
		rINTPND |= BIT_EINT4_7;
	}
	rINTMSK &= ~(BIT_EINT4_7);      //ʹ���ⲿ�ж�4��7
	rEINTMASK &= 0xffffdf;           //�ⲿ�ж�5ʹ��
	pISR_EINT4_7 = (int)Key_ISR;
}

//��д�жϷ����ӳ���
void __irq Key_ISR(void) {
	int j;
	rINTMSK |= BIT_ALLMSK;   // ���ж�
	if (rEINTPEND & 0x20) {
		key_number = read7279(cmd_read);//������ָ�����
		rINTMSK &= ~(BIT_EINT4_7);
		switch (key_number) {
		case 0x04:     key_number = 0x08;   break;
		case 0x05:     key_number = 0x09;   break;
		case 0x06:     key_number = 0x0A;   break;
		case 0x07:     key_number = 0x0B;   break;
		case 0x08:     key_number = 0x04;   break;
		case 0x09:     key_number = 0x05;   break;
		case 0x0A:     key_number = 0x06;   break;
		case 0x0b:     key_number = 0x07;   break;
		default:        break;
		}
		Uart_Printf("key is %x \n", key_number);
	}
	rEINTPEND |= 0x20;
	rSRCPND |= BIT_EINT4_7;
	rINTPND |= BIT_EINT4_7;
}


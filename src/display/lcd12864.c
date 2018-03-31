/*
 * lcd12864.c
 *
 *  Created on: 2016-6-25
 *      Author: redchenjs
 *
 *      ����˵����LCD12864Ӳ����������������LCD��ʼ������ʾ���֣�ͼƬ�ȹ��ܡ�
 *      �������ã�
 *      	M68���ڣ�RS->P6.6, RW->P6.5, EN->P6.4, DATA->P3����c�ļ��ж��壩
 *      	SPI���ڣ�CS->P6.6, SID->P6.5, CLK->P6.4(��spi.h�ж���)
 *      ע���л�ͨ��Э��ֱ���޸�lcd12864.h�е���������ָ�������ȫ����
 */
#include "msp430.h"
#include "lcd12864.h"

const unsigned char CGRAM[4][32]={	0x0F,0xF0,0x10,0x08,0x20,0x04,0x40,0x02,0x9C,0x39,0xBE,0x7D,0x80,0x01,0x80,0x01,
									0x80,0x01,0x88,0x11,0x84,0x21,0x43,0xC2,0x20,0x04,0x10,0x08,0x0F,0xF0,0x00,0x00,

									0x00,0x00,0x10,0x02,0x18,0x04,0x06,0x08,0x00,0x00,0x00,0x00,0xFF,0xCE,0x00,0x00,
									0x00,0x00,0x00,0x20,0x00,0x50,0x00,0x88,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

									0x01,0x80,0x01,0x80,0x01,0x80,0xFF,0xFF,0xFF,0xFF,0x01,0x80,0x01,0x80,0x01,0x80,
									0x01,0x80,0x01,0x80,0x01,0x80,0xFF,0xFF,0xFF,0xFF,0x01,0x80,0x01,0x80,0x01,0x80,

									0xFF,0xFF,0xFF,0xFF,0xC1,0x83,0xC1,0x83,0xC1,0x83,0xC1,0x83,0xC1,0x83,0xFF,0xFF,
									0xFF,0xFF,0xC1,0x83,0xC1,0x83,0xC1,0x83,0xC1,0x83,0xC1,0x83,0xFF,0xFF,0xFF,0xFF};

#define CPU_FREQ ((double)16000000)
#define delay_ns(x) __delay_cycles((long)(CPU_FREQ*(double)x/1000000000.0))
#define delay_us(x) __delay_cycles((long)(CPU_FREQ*(double)x/1000000.0))
#define delay_ms(x) __delay_cycles((long)(CPU_FREQ*(double)x/1000.0))

char tab_SPI1[17]={"0123456789ABCDEF"};
char tab_SPI2[17]={"abcdef0123456789"};
char tab_SPI3[17]={"һ�����������߰�"};
char tab_SPI4[17]={"���������װ�ܽ��"};

/*12864����Ӧ��ָ��*/
#define CLEAR_SCREEN        0x01                    //����ָ�������ACֵΪ00H
#define AC_INIT             0x02                    //��AC����Ϊ00H�����α��Ƶ�ԭ��λ��
#define CURSE_ADD        	0x06                    //�趨�α��Ƶ�����ͼ�������ƶ�����Ĭ���α����ƣ�ͼ�����岻����
#define FUN_MODE        	0x30                    //����ģʽ��8λ����ָ�
#define DISPLAY_ON        	0x0c                    //��ʾ��,��ʾ�α꣬���α�λ�÷���
#define DISPLAY_OFF        	0x08                    //��ʾ��
#define CURSE_DIR        	0x14                    //�α������ƶ�:AC=AC+1
#define SET_CG_AC        	0x40                    //����AC����ΧΪ��00H~3FH
#define SET_DD_AC        	0x80
#define BUSY_FLAG			0x80
/*12864��չӦ��ָ��*/
#define SP_MODE      		0x34					// 8λ���ݣ�����ָ�����
#define GRAPON    			0x36					// ����ָ�ͼ����ʾ��
#define	gdbas     			0x80
//*******************************************************************
#define CGRAM_0				0x00
#define CGRAM_1				0x02
#define CGRAM_2				0x04
#define CGRAM_3				0x06

#ifdef M68_MODE										//M68����ͨ��ģʽ

#define SET_CMD_PORT  	P6DIR |= 0x70; P8DIR |= BIT2; P8OUT |= BIT2
													//��������ڷ���
#define SET_DATA_IN		P3DIR  = 0x00				//�л����ݿڷ�������ģʽ
#define SET_DATA_OUT	P3DIR  = 0xff				//�л����ݿڷ������ģʽ
#define LCD_DATA_IN		P3IN						//ѡ��LCD���ݿ�����Ĵ���
#define LCD_DATA_OUT	P3OUT						//ѡ��LCD���ݿ�����Ĵ���
#define LCD_RS_H		P6OUT |= BIT6				//RS�Ĵ���ѡ��ѡ�����ݼĴ���
#define LCD_RS_L		P6OUT &=~BIT6				//RS�Ĵ���ѡ��ѡ��ָ��Ĵ���
#define LCD_RW_H		P6OUT |= BIT5				//RW��дλ���ã�ѡ�ж�ģʽ
#define LCD_RW_L		P6OUT &=~BIT5				//RW��дλ���ã�ѡ��дģʽ
#define LCD_EN_H		P6OUT |= BIT4				//ENʹ���źţ�ʹ����Ч
#define LCD_EN_L		P6OUT &=~BIT4				//ENʹ���źţ�ʹ����Ч

/*******************************************
�������ƣ�WaitForReady
�������ܣ�����ģʽ��æ
�����������
�����������
�������أ���
********************************************/
void WaitForReady(void)
{
	unsigned char ReadTemp = 0;

	LCD_RS_L;										//ѡ��ָ��Ĵ���
	LCD_RW_H;										//ѡ�ж�ȡģʽ
    SET_DATA_IN;									//�л����ݿڵ�����ģʽ

    do {
        LCD_EN_H;									//ʹ�ܴ�
       	_NOP();										//�ȴ�����
       	ReadTemp = LCD_DATA_IN;						//��ȡ���ݿ�
       	LCD_EN_L;									//ʹ�ܹر�
    }while(ReadTemp & BUSY_FLAG);					//��æ�ȴ�
}
/*******************************************
�������ƣ�SendByte
�������ܣ���LCD����һ���ֽڵ�����
���������data-->�ֽ�����
�����������
�������أ���
********************************************/
void SendByte(unsigned char data)
{
	WaitForReady();									//��æ�ȴ�

	SET_DATA_OUT;									//�л����ݿڵ����ģʽ

    LCD_RW_L;										//ѡ��д��ģʽ
    LCD_DATA_OUT = data;							//�������ݿ�

    LCD_EN_H;										//ʹ�ܴ�
    _NOP();											//�ȴ�����
    LCD_EN_L;										//ʹ�ܹر�
}
/*******************************************
�������ƣ�SendCMD
�������ܣ���LCD����һ֡����
���������data-->�ֽ�����
�����������
�������أ���
********************************************/
void SendCMD(unsigned char data)
{
	WaitForReady();									//��æ�ȴ�

	SET_DATA_OUT;									//�л����ݿڵ����ģʽ

	LCD_RS_L;										//ѡ��ָ��Ĵ���
    LCD_RW_L;										//ѡ��д��ģʽ
    LCD_DATA_OUT = data;							//�������ݿ�

    LCD_EN_H;										//ʹ�ܴ�
    _NOP();											//�ȴ�����
    LCD_EN_L;										//ʹ�ܹر�
}
/*******************************************
�������ƣ�SendData
�������ܣ���LCD����һ֡����
���������data-->�ֽ�����
�����������
�������أ���
********************************************/
void SendData(unsigned char data)
{
	WaitForReady();									//��æ�ȴ�

	SET_DATA_OUT;									//�л����ݿڵ����ģʽ

	LCD_RS_H;										//ѡ�����ݼĴ���
    LCD_RW_L;										//ѡ��д��ģʽ
    LCD_DATA_OUT = data;							//�������ݿ�

    LCD_EN_H;										//ʹ�ܴ�
    _NOP();											//�ȴ�����
    LCD_EN_L;										//ʹ�ܹر�
}
/*******************************************
//        ������
********************************************/
unsigned char ReceiveByte(void)
{
    unsigned char ReadData = 0;

	WaitForReady();									//��æ�ȴ�

    SET_DATA_IN;									//�л����ݿڵ�����ģʽ

    LCD_RS_H;										//ѡ�����ݼĴ���
    LCD_RW_H;										//ѡ�ж�ģʽ

    LCD_EN_H;										//ʹ�ܴ�
   	_NOP();											//�ȴ�����
   	ReadData = LCD_DATA_IN;							//��ȡ���ݿ�
    LCD_EN_L;										//ʹ�ܹر�

    return ReadData;
}
#endif

#ifdef SPI_MODE

#include "spi.h"

#define SET_CMD_PORT  	P6DIR |= 0x70; P8DIR |= BIT2; P8OUT &=~BIT2		//��������ڷ���

unsigned char GDRAM_buff[1024] = {0};									//ȫ��GDRAM����
unsigned char DDRAM_buff[2048] = {0};									//ȫ��DDRAM����

void SendCMD(unsigned char dat)	//д��������
{
	unsigned char conf[3]={0};

	conf[0] = 0xf8;//11111,00,0 RW=0,RS=0 ͬ����־
	conf[1] = dat&0xf0;//����λ
	conf[2] = (dat&0x0f)<<4;//����λ

	SPI_CS_High();
	SPI_TxFrame(conf, 3);
	SPI_CS_Low();
}

void SendData(unsigned char dat)	//д��ʾ���ݻ��ֽ��ַ�
{

	unsigned char conf[3]={0};

	conf[0] = 0xfa;//11111,01,0 RW=0,RS=1
	conf[1] = dat&0xf0;//����λ
	conf[2] = (dat&0x0f)<<4;//����λ

	SPI_CS_High();
	SPI_TxFrame(conf, 3);
	SPI_CS_Low();
}

unsigned char ReceiveByte(void)
{
	unsigned char temp;

	SPI_CS_High();
	SPI_RxFrame(&temp, 1);
	SPI_CS_Low();

	return temp;
}

#endif
/*
 * ���õ�ǰ�Դ��α��ַ
 *
 * x��1-8(ÿ����ַ�����ֽ�)
 * y��1-4(4��)
 */
void SetCoord(unsigned char x, unsigned char y)
{
   switch (y) {
		case 1:	SendCMD(0x7F + x);	break;
		case 2: SendCMD(0x8F + x);	break;
		case 3: SendCMD(0x87 + x);	break;
		case 4: SendCMD(0x97 + x);	break;
		default:					break;
   }
}
/*
 * x:1-16
 * y:1-4
 */
void ReadChar(unsigned char *data, unsigned char x, unsigned char y)
{
	unsigned char DDRAM_hbit, DDRAM_lbit;

	SendCMD(0x30);
	SetCoord((x-1)/2+1, y);
	ReceiveByte();				//Ԥ������
	DDRAM_hbit=ReceiveByte();	//��ȡ��ǰ��ʾ��8 λ����
	DDRAM_lbit=ReceiveByte();	//��ȡ��ǰ��ʾ��8 λ����

	if (x%2)
		*data = DDRAM_lbit;
	else
		*data = DDRAM_hbit;
}

/*
 * x:1-8
 * y:1-4
 */
void ReadWord(unsigned char *data, unsigned char x, unsigned char y)
{
	SendCMD(0x30);
	SetCoord(x, y);
	ReceiveByte();				//Ԥ������
	data[1] = ReceiveByte();	//��ȡ��ǰ��ʾ��8 λ����
	data[0] = ReceiveByte();	//��ȡ��ǰ��ʾ��8 λ����
}

void WriteGDRAM(unsigned char data)
{
	unsigned char i,j,k;
	unsigned char bGDRAMAddrX = 0x80; //GDRAM ˮƽ��ַ
	unsigned char bGDRAMAddrY = 0x80; //GDRAM ��ֱ��ַ
	for (i=0;i<2;i++) {
		for (j=0;j<32;j++) {
			for (k=0;k<8;k++) {
				SendCMD(0x34); 			//����Ϊ8 λMPU �ӿڣ�����ָ�,��ͼģʽ��
				SendCMD(bGDRAMAddrY+j); //��ֱ��ַY
				SendCMD(bGDRAMAddrX+k); //ˮƽ��ַX
				SendData(data);
				SendData(data);
			}
		}
		bGDRAMAddrX = 0x88;
	}
	SendCMD(0x36); //�򿪻�ͼģʽ
	SendCMD(0x30); //�ָ�����ָ����رջ�ͼģʽ
}

/*
 * ��LCD������д���Զ����ַ�
 */
void WriteCGRAM(void)
{
     int i, j;

     for (j=0; j<4; j++) {
    	 SendCMD(0x30);
    	 SendCMD(0x40+j*16);
		 for (i=0;i<16;i++) {
			 SendData(CGRAM[j][i*2]);
			 SendData(CGRAM[j][i*2+1]);
		 }
     }
}

/*
 * ��ʾ�û��Զ����ַ�
 * index��1-4
 * x��1-8(ÿ���ַ�2���ֽڿ��)
 * y��1-4
 */
void LCD_Disp_CGRAM(const unsigned char index, unsigned char x,unsigned char y)
{
	SetCoord(x, y);
	SendData(0x00);
	SendData(index*2);
}

void LCD_Disp_Image(const unsigned char *str, unsigned char x, unsigned char y, unsigned char width, unsigned char height)
{
	unsigned char i=0, j=0;
	unsigned char x_start=0, x_width=0;
	unsigned char y_upper_start=0, y_upper_end=0;
	unsigned char y_lower_start=0, y_lower_end=0;

	x_start = x / 8;
	x_width = width % 8 ? width / 8 + 1 : width / 8;

	y_upper_start = y < 32 ? y : 32;
	y_upper_end	= y + height < 32 ? y + height : 32;
	y_lower_start = y < 32 ? 0 : y - 32;
	y_lower_end = y + height < 32 ? 0 : y + height - 32;

	SendCMD(0x36);	//��ͼ��ʾ��������ָ�extended instruction(DL=8BITS,RE=1,G=1)

	for (i=y_upper_start; i<y_upper_end; i++) {
		SendCMD(0x80 + i);				//SET  ��ֱ��ַ VERTICAL ADD
		SendCMD(0x80 + x_start);		//SET  ˮƽ��ַ HORIZONTAL ADD
		for (j=0; j<x_width; j++) {
			SendData(*str++);
		}
	}

	for (i=y_lower_start; i<y_lower_end; i++) {
		SendCMD(0x80 + i);				//SET ��ֱ��ַ VERTICAL ADD
		SendCMD(0x88 + x_start);		//SET ˮƽ��ַ HORIZONTAL ADD
		for (j=0; j<x_width; j++) {
			SendData(*str++);
		}
	}
}

void LCD_CLR_Line(unsigned char x, unsigned char y, unsigned char length, unsigned char dir)
{
	const unsigned char clr[128]={0};
	if (dir)
		LCD_Disp_Image(clr, x, y, length, 1);
	else
		LCD_Disp_Image(clr, x, y, 1, length);
}
/*
 * x:1-16
 * y:1-4
 */
void LCD_Disp_Char(unsigned char data, unsigned char x, unsigned char y)
{
	unsigned char temp;

	SendCMD(0x30);
	ReadChar(&temp, x, y);
	SetCoord((x-1)/2+1, y);
	if (x%2) {
		SendData(data);
		SendData(temp);
	}
	else {
		SendData(temp);
		SendData(data);
	}
}

void LCD_CLR_Char(unsigned char x, unsigned char y)
{
	SendCMD(0x30);
	SetCoord(x, y);
    SendData(0x20);
}

void LCD_Disp_Word(unsigned char *data, unsigned char x, unsigned char y)
{
	SendCMD(0x30);
	SetCoord(x, y);
    SendData(data[0]);
    SendData(data[1]);
}

void LCD_CLR_Word(unsigned char x, unsigned char y)
{
	SendCMD(0x30);
	SetCoord(x, y);
    SendData(0x20);
    SendData(0x20);
}

void LCD_Del_Word(char num)
{
	unsigned char ReadTemp=0;

	SendCMD(0x30);
	SendCMD(0x07);   //�α�����

	ReceiveByte();
	ReadTemp = ReceiveByte();

	SendCMD(ReadTemp);

	while (num--) {
		SendData(0x20);
		SendData(0x20);
	}
}

void LCD_Disp_String(char *str, unsigned char x, unsigned char y)
{
	unsigned char temp;
	SendCMD(0x30);
	SetCoord(x, y);
	temp = *str;
	while (temp != 0) {
		SendData(temp);
		temp = *++str;
	}
}

void LCD_Disp_Point(unsigned char color, unsigned char x,unsigned char y)
{
	unsigned char x_Dyte,x_byte; //�����е�ַ���ֽ�λ�������ֽ��е���1 λ
	unsigned char y_Dyte,y_byte; //����Ϊ����������(ȡֵΪ0��1)���е�ַ(ȡֵΪ0~31)
	unsigned char GDRAM_hbit,GDRAM_lbit;

	SendCMD(0x36); //��չָ������
	/***X,Y ���껥��������ͨ��X,Y ����***/
	x_Dyte=x/16; //������16 ���ֽ��е���һ��
	x_byte=x&0x0f; //�����ڸ��ֽ��е���һλ
	y_Dyte=y/32; //0 Ϊ�ϰ�����1 Ϊ�°���
	y_byte=y&0x1f; //������0~31 ���е���һ��
	SendCMD(0x80+y_byte); //�趨�е�ַ(y ����),���Ǵ�ֱ��ַ
	SendCMD(0x80+x_Dyte+8*y_Dyte); //�趨�е�ַ(x ����)����ͨ��8*y_Dyte ѡ��������������ˮƽ��ַ
	ReceiveByte(); //Ԥ��ȡ����
	GDRAM_hbit=ReceiveByte(); //��ȡ��ǰ��ʾ��8 λ����
	GDRAM_lbit=ReceiveByte(); //��ȡ��ǰ��ʾ��8 λ����
	delay_ms(1);
	SendCMD(0x80+y_byte); //�趨�е�ַ(y ����)
	SendCMD(0x80+x_Dyte+8*y_Dyte); //�趨�е�ַ(x ����)����ͨ��8*y_Dyte ѡ��������
	delay_ms(1);
	if(x_byte<8) //�ж����ڸ�8 λ�������ڵ�8 λ
	{
		if(color==1)
		{
			SendData(GDRAM_hbit|(0x01<<(7-x_byte))); //��λGDRAM ����8 λ��������Ӧ�ĵ�
		}
		else
			SendData(GDRAM_hbit&(~(0x01<<(7-x_byte)))); //���GDRAM ����8 λ��������Ӧ�ĵ�

			SendData(GDRAM_lbit); //��ʾGDRAM ����8 λ����
		}
		else
		{
			SendData(GDRAM_hbit);         //д��8λ����
		if(color==1)
			SendData(GDRAM_lbit|(0x01<<(15-x_byte))); //��λGDRAM ����8 λ��������Ӧ�ĵ�
		else
			SendData(GDRAM_lbit&(~(0x01<<(15-x_byte))));//���GDRAM����8λ��������Ӧ�ĵ�
	}
	SendCMD(0x30); //�ָ�������ָ�
}

void LCD_CLR_GDRAM(void)
{
	WriteGDRAM(0x00);
}

void LCD_Fill_GDRAM(void)
{
	WriteGDRAM(0xff);
}

void LCD_CLR_DDRAM(void)
{
	SendCMD(0x01);
	SendCMD(0x34);
	SendCMD(0x30);
}

void LCD_Disp_Init(void)
{
	SET_CMD_PORT;  //��ʼ�������

    delay_ms(500);
    SendCMD(0x30);   //����ָ�
    delay_ms(1);
    SendCMD(0x02);   // ��ַ��λ
    delay_ms(1);
	SendCMD(0x0c);   //������ʾ��,�α�ر�
    delay_ms(1);
	SendCMD(0x01);   //�����ʾ
    delay_ms(10);
	SendCMD(0x06);   //�α�����
    delay_ms(1);
	SendCMD(0x80);   //�趨��ʾ����ʼ��ַ

	WriteCGRAM();

	LCD_CLR_GDRAM();
	LCD_CLR_DDRAM();
}

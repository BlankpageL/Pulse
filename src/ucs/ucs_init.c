/*
 * ucs_init.c
 *
 *  Created on: 2016-6-29
 *      Author: redchenjs
 *
 *		Ĭ��ʱ�����ã�
 *			ACLK:XT1CLK(����Ч��Ƶʱ�л���REFO����Ƶ�л���DCO)
 *			SMCLK:DCOCLKDIV
 *			MCLK:DCOCLKDIV
 *			FLLREFCLK:XT1CLK(����Ч�л���REFO)
 *
 *		ʱ���ź�����ڣ�
 *			ACLK:P1.0
 *			SMCLK:P2.2
 *			MCLK:P7.7
 *
 * 		��ǰ�ź�Դ���ã�
 *			XT1CLK:32.768kHz
 *			XT2CLK:4MHz
 *			VLOCLK:10kHz
 *			REFOCLK:32.768kHz
 *			DCOCLK:15.99MHz
 *
 *		��ǰʱ��Դ���ã�
 *			ACLK:XT1CLK(32.768kHz)
 *			SMCLK:XT2CLK(4MHz)
 *			MCLK:DCOCLK(15.99MHz)
 *
 */
#include "msp430.h"

#define SET_ACLK_OUT	P1SEL |= BIT0; P1DIR |= BIT0	//����P1.0ΪACLK���
#define SET_SMCLK_OUT	P2SEL |= BIT2; P2DIR |= BIT2	//����P2.2ΪSMCLK���
#define SET_MCLK_OUT	P7SEL |= BIT7; P7DIR |= BIT7	//����P7.7ΪMCLK���
#define SET_XT1_IN		P5SEL |= BIT4 | BIT5			//����P5.4,P5.5ΪXT1�ⲿ��������
#define SET_XT2_IN		P5SEL |= BIT2 | BIT3			//����P5.2,P5.3ΪXT2�ⲿ��������
#define SET_XT1_ON		UCSCTL6 |= XCAP_3; UCSCTL6 &=~XT1OFF;//����XT1����
#define SET_XT2_ON		UCSCTL6 &=~XT2OFF;				//����XT2����
#define SET_CLK_SOURCE	UCSCTL4=(UCSCTL4&(~(SELA_7|SELS_7|SELM_7)))|SELA_0|SELS__XT2CLK|SELM_3
														//����ʱ��Դ
//void WaitForOscillation(void)
//{
//	while (SFRIFG1 & OFIFG) {
//		UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);		//UCS���ƼĴ������ã����ʱ�ӹ��ϱ�־λ
//		SFRIFG1 &= ~OFIFG;								//���⹦�ܼĴ������ã����ʱ�ӹ��ϱ�־λ
//	}
//}

void SetVcoreUp (unsigned int level)
{
  // Open PMM registers for write
  PMMCTL0_H = PMMPW_H;
  // Set SVS/SVM high side new level
  SVSMHCTL = SVSHE + SVSHRVL0 * level + SVMHE + SVSMHRRL0 * level;
  // Set SVM low side to new level
  SVSMLCTL = SVSLE + SVMLE + SVSMLRRL0 * level;
  // Wait till SVM is settled
  while ((PMMIFG & SVSMLDLYIFG) == 0);
  // Clear already set flags
  PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);
  // Set VCore to new level
  PMMCTL0_L = PMMCOREV0 * level;
  // Wait till new level reached
  if ((PMMIFG & SVMLIFG))
    while ((PMMIFG & SVMLVLRIFG) == 0);
  // Set SVS/SVM low side to new level
  SVSMLCTL = SVSLE + SVSLRVL0 * level + SVMLE + SVSMLRRL0 * level;
  // Lock PMM registers for write access
  PMMCTL0_H = 0x00;
}

void UCS_Init(void)
{
//	SET_MCLK_OUT;				//����MCLK���
//	SET_ACLK_OUT;				//����ACLK���
//	SET_SMCLK_OUT;				//����SMCLK���
//
//	SET_XT1_IN;					//����XT1��������
//	SET_XT1_ON;					//����XT1�ⲿ����
//
//	SET_XT2_IN;					//����XT2��������
//	SET_XT2_ON;					//����XT2�ⲿ����
//
//	__bis_SR_register(SCG0);	//����DCOCLK֮ǰ������SCG0�Ĵ���λ��MCLKֹͣ����
//
//	UCSCTL0 = 0x00;   			//�����㣬FLL����ʱ���üĴ���ϵͳ���Զ����ã����ù�
//	UCSCTL1 = DCORSEL_6;		//���ڷ�ΧԼΪ ~ MHZ������DCO��Ƶ�ʷ�Χ��֮�����õ�DCOʱ��Ҫ�������Χ�ڣ���������
//	UCSCTL2 = FLLD_1 | 243;		//FLLD=1,FLLN=243,��Ƶ��Ϊ2*��243+1��*32.768=15.99MHZ
//								//DCOCLK = D*(N+1)*(REFCLK/n)
//								//DCOCLKDIV = (N+1)*(REFCLK/n)
//
//	__bic_SR_register(SCG0);	//DCOCLK������ϣ����SCG0�Ĵ���λ��MCLK�ָ�����
//	__delay_cycles(782000);		//��ʱ
//
//	WaitForOscillation();		//�ȴ�ʱ������
//
//	SET_CLK_SOURCE;				//����ʱ��Դ

//     Increase Vcore setting to level3 to support fsystem=25MHz
//     NOTE: Change core voltage one level at a time..
    SetVcoreUp (0x01);
    SetVcoreUp (0x02);
    SetVcoreUp (0x03);

    UCSCTL3 = SELREF_2;                       // Set DCO FLL reference = REFO
    UCSCTL4 |= SELA_2;                        // Set ACLK = REFO

    __bis_SR_register(SCG0);                  // Disable the FLL control loop
    UCSCTL0 = 0x0000;                         // Set lowest possible DCOx, MODx
    UCSCTL1 = DCORSEL_7;                      // Select DCO range 50MHz operation
    UCSCTL2 = FLLD_0 + 762;                   // Set DCO Multiplier for 25MHz
                                              // (N + 1) * FLLRef = Fdco
                                              // (762 + 1) * 32768 = 25MHz
                                              // Set FLL Div = fDCOCLK/2
    __bic_SR_register(SCG0);                  // Enable the FLL control loop

    // Worst-case settling time for the DCO when the DCO range bits have been
    // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
    // UG for optimization.
    // 32 x 32 x 25 MHz / 32,768 Hz ~ 780k MCLK cycles for DCO to settle
    __delay_cycles(782000);

    // Loop until XT1,XT2 & DCO stabilizes - In this case only DCO has to stabilize
    do
    {
      UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
                                              // Clear XT2,XT1,DCO fault flags
      SFRIFG1 &= ~OFIFG;                      // Clear fault flags
    }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag

}


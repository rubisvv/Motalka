/*
 * Timers.c
 *
 * Created: 20.01.2020 23:22:03
 *  Author: Vl
 */ 

//#define Max_Numbers_of_Timer  7;
//uint16_t SoftTimer[Max_Numbers_of_Timer];
#include <avr/io.h>
#include <avr/interrupt.h>
#include "timers.h"



void InitTimers()
{
	TCNT0 = 0;
	// ������ 0 ������������ 32
	TCCR0 = 0b00000011;
	TCNT0 = 26; // �� ������������ 230 ���. ��� Fq = 14745600, ������������ 32, ���������� ����������� � �������� 2003,478 ��

	//������ 1 correct pWM 8 ���,
	//��������� �� ���������� ��� ����� �����. ��������������� �� ���������� ��� ����� ���� (�� ��������������� ���)
	// ������� ��� f TC1 /510
	TCCR1A = (1<<COM1A1)|(1<<COM1B1)|(1<<WGM10);
	TCCR1B = (1<<CS11)|(1<<CS10);  //������ 1 ������������ 64
	
	OCR1AH = 0;
	OCR1AL = 00;
	OCR1BH = 0;
	OCR1BL = 00;
		
	TCNT1H = 0;
}


ISR(TIMER0_OVF_vect)
{
	TCNT0 = 26;
	if (KeyScanTimer > 0){
		KeyScanTimer--;
	}

	if (KeyEventTimer>0){
		KeyEventTimer--;
	}
	

	if (LEDFlashTimer>0){
		LEDFlashTimer--;
	}

	if (StepEngTimer > 0){
		StepEngTimer--;
	}

	if (WindEngTimer > 0){
		WindEngTimer--;
	}
	
	if (BlinkTimer > 0){
		BlinkTimer--;
	}
	
	if (StopTimer > 0){
		StopTimer--;
	}
	

	//uint8_t i;
	//for(i=0;i!=Max_Numbers_of_Timer;i++)
	//{
	//
	//if(SoftTimer[i] !=0)		 // ���� ������ �� ��������, �� ������� ��� ���.
	//{
	//SoftTimer[i] --;	 // ��������� ����� � ������ ���� �� �����.
	//}
	//
	//}

}

ISR(TIMER2_OVF_vect)
{

}

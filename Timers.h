/*
 * Timers.h
 *
 * Created: 20.01.2020 23:36:32
 *  Author: Vl
 */ 


#ifndef TIMERS_H_
#define TIMERS_H_

uint16_t KeyEventTimer;		// �������� ����������� ������� �������
uint8_t KeyScanTimer;		// ��������������� �������� ���������� ������������
uint8_t LEDFlashTimer;		// ����������������� �������� ������
volatile uint16_t StepEngTimer;
uint16_t WindEngTimer;
uint16_t BlinkTimer;
uint16_t StopTimer;



void InitTimers();
ISR(TIMER0_OVF_vect);
ISR(TIMER2_OVF_vect);

#endif /* TIMERS_H_ */
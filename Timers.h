/*
 * Timers.h
 *
 * Created: 20.01.2020 23:36:32
 *  Author: Vl
 */ 


#ifndef TIMERS_H_
#define TIMERS_H_

uint16_t KeyEventTimer;		// Задержка автоповтора нажатия клавиши
uint8_t KeyScanTimer;		// Антидребезговая задержка повторного сканирования
uint8_t LEDFlashTimer;		// продолжительность свечения диодов
volatile uint16_t StepEngTimer;
uint16_t WindEngTimer;
uint16_t BlinkTimer;
uint16_t StopTimer;



void InitTimers();
ISR(TIMER0_OVF_vect);
ISR(TIMER2_OVF_vect);

#endif /* TIMERS_H_ */
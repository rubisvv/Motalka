/*
 * globals.h
 *
 *	“ут описываютс€ все инклюды, общие дл€ всего проекта. „тобы их каждый раз не определ€ть в каждом файле.
 *
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

#define F_CPU 14700000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <stdio.h>

//KeyEventTimer   2b	
//KeyScanTimer 	1b 
//LEDFlashTimer	1b /

uint8_t KeyCode;			// код нажатой клавиши
uint8_t EventCode;

uint8_t	    StateStepEng;
uint8_t		phaseStepEng;
uint8_t		prevStateStepEng;
uint8_t		PotentValue;

#endif /* GLOBALS_H_ */

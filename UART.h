/*
 * IncFile4.h
 *
 * Created: 28.12.2020 14:20:27
 *  Author: Vl
 */ 


#ifndef INCFILE4_H_
#define INCFILE4_H_





#endif /* INCFILE4_H_ */

//#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)


void UART_init(long USART_BAUDRATE);

unsigned char UART_RxChar();

void UART_TxChar(char ch);

void UART_SendString(char *str);

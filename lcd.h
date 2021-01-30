#ifndef _LCH_H_
#include <avr/io.h>
#define _LCD_H_ 1

#define LCD_PORT	PORTA
#define LCD_DDR		DDRA
#define LCD_PIN		PINA
#define LCD_RS		PA0
#define LCD_RW		PA1
#define LCD_E		PA2
#define LCD_DB4		PA4
#define LCD_DB5		PA5
#define LCD_DB6		PA6
#define LCD_DB7		PA7
  
void lcd_write(unsigned char codeA)
{
	cli();
	LCD_DDR |= ( _BV(LCD_DB7) | _BV(LCD_DB6) | _BV(LCD_DB5) | _BV(LCD_DB4) );  // A7-A4  на выход
	//LCD_DDR |= ( _BV(LCD_DB7) | _BV(LCD_DB6) | _BV(LCD_DB5) | _BV(LCD_DB4) | _BV(PA2) | _BV(PA1) | _BV(PA0));
	LCD_PORT &= ~( _BV(LCD_RS) | _BV(LCD_RW) );  //  RS(A0) = 0, RW(A1) = 0  обнуляем 
	_delay_us(10);
	LCD_PORT |= _BV(LCD_E); // строб
	LCD_PORT |= codeA; 
	_delay_us(10);
	LCD_PORT &= ~_BV(LCD_E); // ~строб
	_delay_us(10);
	LCD_PORT ^= codeA; 
	_delay_us(10);
	sei();
}

unsigned char lcd_readStatus()
{
	char BS;

	LCD_PORT |= ( _BV(LCD_RW) );
	_delay_us(10);
	LCD_DDR &= ~( _BV(LCD_DB7) | _BV(LCD_DB6) | _BV(LCD_DB5) | _BV(LCD_DB4) );
	LCD_PORT |= ( _BV(LCD_E) ); // строб
	_delay_us(10);
	LCD_PORT &= ~_BV(LCD_E); // ~строб
	BS = (LCD_PORT & _BV(LCD_DB7)) >> LCD_DB7;
	_delay_us(10);
	LCD_PORT |= _BV(LCD_E); // строб
	_delay_us(10);
	LCD_PORT &= ~_BV(LCD_E); // ~строб
	_delay_us(10);

	LCD_DDR |= ( _BV(LCD_DB7) | _BV(LCD_DB6) | _BV(LCD_DB5) | _BV(LCD_DB4) );
	LCD_PORT &= ~( _BV(LCD_DB7) | _BV(LCD_DB6) | _BV(LCD_DB5) | _BV(LCD_DB4) | _BV(LCD_RW));
	_delay_us(2);

	return BS;
}

void lcd_putc(unsigned char ch)
{
	while (lcd_readStatus()) {}
	//_delay_us(10);
	
	lcd_write(_BV(LCD_RS) 
		| ((ch & _BV(7)) >> 7)	<< LCD_DB7
		| ((ch & _BV(6)) >> 6)	<< LCD_DB6
		| ((ch & _BV(5)) >> 5)	<< LCD_DB5
		| ((ch & _BV(4)) >> 4)	<< LCD_DB4
	);
	lcd_write(_BV(LCD_RS) 
		| ((ch & _BV(3)) >> 3)	<< LCD_DB7
		| ((ch & _BV(2)) >> 2)	<< LCD_DB6
		| ((ch & _BV(1)) >> 1)	<< LCD_DB5
		| (ch & 1) 		<< LCD_DB4
	);
	_delay_us(2);
}

void lcd_put_hex(unsigned char ch)
{
	unsigned char s;
	s = (ch & 0xf0) >> 4;
	if (s<10)
		lcd_putc('0'+s);
	else
		lcd_putc('A'+s-10);

	s = (ch & 0x0f);
	if (s<10)
		lcd_putc('0'+s);
	else
		lcd_putc('A'+s-10);
}

void lcd_init(void)
{
	LCD_DDR = 0xff;
	LCD_PORT = 0;
	_delay_ms(50);

	lcd_write( _BV(LCD_DB5) | _BV(LCD_DB4) );
	_delay_ms(5);
	lcd_write( _BV(LCD_DB5) | _BV(LCD_DB4) );
	_delay_ms(1);
	lcd_write( _BV(LCD_DB5) | _BV(LCD_DB4) );
	_delay_ms(2);

	lcd_write( _BV(LCD_DB5) );
	_delay_ms(2);

	lcd_write( _BV(LCD_DB5) );
	_delay_ms(2);
	lcd_write( _BV(LCD_DB7) | _BV(LCD_DB6) );
	_delay_ms(2);

	lcd_write( 0 );
	lcd_write( _BV(LCD_DB7) );
	_delay_ms(2);

	lcd_write( 0 );
	lcd_write( _BV(LCD_DB4) );
	_delay_ms(2);

	lcd_write( 0 );
	lcd_write( _BV(LCD_DB6) | _BV(LCD_DB5) );
	_delay_ms(2);

	lcd_write( 0 );
	lcd_write( _BV(LCD_DB7) | _BV(LCD_DB6) | _BV(LCD_DB5) );
	_delay_ms(2);
}

// mode:
//1 - курсор не виден
//2 - курсор виден в виде подчёркивания
//3 - мигающий курсор
void lcd_cursor(unsigned char mode) 
{
	switch (mode)
	{
		
		case 1:
		lcd_write( 0 );
		lcd_write(_BV(LCD_DB7) | _BV(LCD_DB6));  // курсор не виден
		_delay_ms(2);
		break;
		case 2:
		lcd_write( 0 );
		lcd_write(_BV(LCD_DB7) | _BV(LCD_DB6) |_BV(LCD_DB5)); // курсор виден в виде подчёркивания
		_delay_ms(2);
		break;
		case 3:
		lcd_write( 0 );
		lcd_write(_BV(LCD_DB7) | _BV(LCD_DB6) |_BV(LCD_DB5) |_BV(LCD_DB4));// мигающий курсор
		_delay_ms(2);
		break;
	}
}

void lcd_puts(char *str)
{
    register char c;

    while ( (c = *str++) ) {
        lcd_putc(c);
    }
}

void lcd_puts_p(char *str)
{
    register char c;
	//c = pgm_read_byte(str++);
    while ( (c = pgm_read_byte(str++)) ) 
	//while (c != 0) 
	{
        lcd_putc(c);
		//c = pgm_read_byte(str++);
    }
}

void lcd_gotoxy(unsigned char x, unsigned char y)
{
     if (y)
        lcd_write( _BV(LCD_DB7) | _BV(LCD_DB6) );
     else
        lcd_write( _BV(LCD_DB7) );

        lcd_write( 
		  ((x & _BV(3))>>3) <<	LCD_DB7
		| ((x & _BV(2))>>2) <<	LCD_DB6
		| ((x & _BV(1))>>1) <<	LCD_DB5
		| (x & 1) <<		LCD_DB4
	);
}

// Чистим строки на дисплее
// data - номер строки от 1
// data = 0 - чистим все строки
void lcd_clrscr(unsigned char data)
{
     unsigned char i = 0;
     
     if (data == 0)
     {
	     /*Clear display*/
	     lcd_write( 0 );
	     lcd_write( (1<<LCD_DB4) );
	     //_delay_us(120);         //Delay after clearing display
     }
     else
     {
	     
	     lcd_gotoxy(0, data-1);
	     for (i=0; i<16; i++)
	     {
		     lcd_putc(' ');
	     }
	     lcd_gotoxy(0, data-1);
     }
     
     
}

#endif

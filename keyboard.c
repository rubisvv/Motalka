
#include "globals.h"
#include "keyboard.h"
#include "timers.h"
//#include "messages.h"

#define KBD_PIN_LEFT		PB7
#define KBD_PIN_RIGHT		PB6
#define KBD_PIN_UP			PB5
#define KBD_PIN_DOWN		PB4
#define KBD_PIN_OK		PB3
#define KBD_PIN_GND		PB2
#define KBD_MASK (_BV(KBD_PIN_UP)|_BV(KBD_PIN_DOWN)|_BV(KBD_PIN_LEFT)|_BV(KBD_PIN_RIGHT)|_BV(KBD_PIN_OK))

#define KBD_PIN			PINB
#define KBD_PORT		PORTB
#define KBD_DDR			DDRB

uint8_t lastKey,prevKey;
uint8_t kf1,kf2,kf3;

// --- new
#define KEY_PORT		PORTB	// LCD Control Port
#define KEY_PIN			PINB
#define KEY_DDR			DDRB
uint8_t indKeysCol;
uint8_t temp;

unsigned char KeyScanStage;
unsigned char PrevKeyCode;
unsigned char LastKeyCode;

//unsigned char getKeyCode(uint8_t ah) {
//if (!(ah & _BV(KBD_PIN_OK))) {
//return KEY_OK;
//}
//if (!(ah & _BV(KBD_PIN_UP))) {
//return KEY_UP;
//}
//if (!(ah & _BV(KBD_PIN_DOWN))) {
//return KEY_DOWN;
//}
//if (!(ah & _BV(KBD_PIN_LEFT))) {
//return KEY_LEFT;
//}
//if (!(ah & _BV(KBD_PIN_RIGHT))) {
//return KEY_RIGHT;
//}
//return 0;
//}
//
//unsigned char readKey(msg_par par) {
//kf3=kf2;
//kf2=kf1;
//kf1=getKeyCode(KBD_PIN & KBD_MASK);
//
//if ((kf2==kf1) && (kf3==kf2)) {
//prevKey = lastKey;
//lastKey = kf1;
//
//if (prevKey != lastKey) {
//sendMessage(MSG_KEY_PRESS, lastKey);
//killTimer(MSG_KEY_REPEAT);
//if (lastKey) {
////setTimer(MSG_KEY_REPEAT,40,50);
//}
//}
//}
//return(0);
//}
//
//unsigned char repeatKey(msg_par par) {
//if (prevKey == lastKey) {
//sendMessage(MSG_KEY_PRESS, lastKey);
//if (par>5)
//setTimer(MSG_KEY_REPEAT,par-1,par);
//else
//setTimer(MSG_KEY_REPEAT,5,5);
//}
//return(0);
//}
//
//
//
//void  KBD_init() {
//KBD_PORT |= KBD_MASK;
//KBD_DDR &= ~ KBD_MASK;
//
//lastKey = 0;
//prevKey = 0;
//
//setHandler(MSG_KEY_SCAN, &readKey);
//setHandler(MSG_KEY_REPEAT, &repeatKey);
//
//setTimer(MSG_KEY_SCAN, 1, 1);
//}








unsigned char  KeysReq()
{
	unsigned char InStr;
	unsigned char InCol;
	unsigned char scanKeyCode;
	//; --- проверим строки
	KEY_PORT = 0b11110000;   // старшие биты на землю, младшие к подт€гивающим резюкам
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	KEY_DDR =  0b00001111;    // переводит старшие 4 бита порта ввод на , младшие на вывод
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	InStr = KEY_PIN;
	
asm("nop");
asm("nop");
asm("nop");
asm("nop");
	asm("nop");
	InStr = InStr | 0b00001111;//return InStr;
	InStr = ~InStr;

	KEY_PORT = 0b00001111;   // старшие биты на землю, младшие к подт€гивающим резюкам
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	KEY_DDR =  0b11110000;    // переводит старшие 4 бита порта B на вывод, младшие на ввод
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	InCol = KEY_PIN;
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	InCol = InCol | 0b11110000;
	InCol = ~InCol;
	
	scanKeyCode = InStr | InCol;
	return scanKeyCode;
}



// ====================================================
// GetKey процедура опроса клавиатуры с антидребезговой задержкой
// сохран€ет результат в KeyCode
// ====================================================
unsigned char GetKey(void)
{
	uint8_t CurKeyCode;
	uint8_t KeyCode;
	uint8_t scanKeyCode;

	KeyCode = 0;
	
	if (KeyScanTimer > 0) // если таймер отрабатывает антидребезговую задержку, то ничего не делаем
	{
		return 0;
	}
	scanKeyCode = KeysReq();	//иначе читаем клавиатуру

	CurKeyCode = scanKeyCode;  //
	if (CurKeyCode != PrevKeyCode)   // новый код нажатой клавиши
	{
		KeyScanStage = 1;
		KeyScanTimer = 10;					// после нового нажати€ длинна€ антидребезгова€ пауза перед новым сканированием
	}
	else
	{
		if (KeyScanStage == 1)          // если нажата та же клавиша, и предыдуща€ антидреб.пауза была длинной, то сделаем ещЄ одну паузу, короткую
		{
			KeyScanStage = 2;
			KeyScanTimer = 5;
		}
		else
		{
			KeyScanStage = 0;				// и только если в третий раз та же клавиша, то возвращаем код клавиши
			KeyCode = CurKeyCode;
			return KeyCode;
		}
	}

	PrevKeyCode = CurKeyCode;
	return 0;
}


// ====================================================
// KeyEvent ѕроцедура анализа опроса клавиатуры (реализаци€ автоповтора)
// сохран€ет EventCode = 1 если произошло нажание, 0 - если не произошло
// код клавиши остаЄтс€ в KeyCode
// ====================================================
unsigned char KeyEvent(uint8_t KeyCode)
{
	if (KeyCode == 0)
	{
		KeyEventTimer = 0;
		EventCode = 0;
	}
	else
	{
		if (KeyCode == LastKeyCode)
		{
			if (KeyEventTimer != 0)
			{
				EventCode = 0;
			}
			else
			{
				KeyEventTimer = 120; // задаЄм таймер автоповтора 200мс
				EventCode = 1;
			}
		}
		else
		{
			EventCode = 1;
			KeyEventTimer = 500;
		};
	}
	LastKeyCode = KeyCode;
	return EventCode;
}






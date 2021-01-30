/*
* Motalko_C_My.c
*
* Created: 28.02.2017 21:10:03
* Author : VL
*/
#define F_CPU 8000000L
#include <avr/io.h>
#include <avr/eeprom.h>
#include "BCD_DHALT.h"
#include "globals.h"
#include "keyboard.h"
#include "lcd.h"
#include "timers.h"
#include "Adc.h"


#define MAX_MENU_ITEM 18
#define ENG_PORT PORTC
#define ENG_DDR DDRC
#define WENG_DDR DDRD
#define WENG_PORT PORTD
#define WIND_MOTOR_PIN0 4
#define WIND_MOTOR_PIN1 5


//#include "messages.h"
//#include "menu.h"
//const char strNULL[] PROGMEM = "12345";

typedef struct PROGMEM {
	const uint8_t     Nom;
	const uint8_t     Next;
	const uint8_t     Previous;
	const uint8_t     Parent;
	const uint8_t     Child;
	const uint8_t     HandlerNum;
	const char Text[16];
} menuItem;

typedef struct PROGMEM {
	const uint8_t     Nom;
	const uint8_t     Next;
	const uint8_t     Previous;
	//const uint8_t     HandlerNum;
	const char Text[16];
	const char Short[5];
} ListItem;

typedef struct PROGMEM {
	const uint8_t     Ind;		// номер поля
	const uint8_t     Col;		// колонка
	const uint8_t     Row;		// ряд
	const uint8_t	  ComaPos;	// позиция запятой	
	const uint8_t	  NumLen;   // длина числа (знаков без запятой)
	const uint8_t	  FieldType;// 0 - число, 1- список
	const uint8_t	  QBytes;   // длина поля в байтах
} DialogFieldStuct;

typedef struct {
	uint16_t	Num;
	uint8_t	CommaPos;
	} Num16;
	
typedef struct {
	uint8_t	Num;
	uint8_t	CommaPos;
} Num8;


enum HandEnum  {
	HE_NOP = 0,
	HE_INP_QUANT = 1,
	HE_INP_SAVE_MEM = 2,
	HE_EDIT_NUM = 3,
	HE_SET_STEP = 4,
	HE_SET_DIRECT = 5,
	HE_SET_QUANT = 6,
	HE_SET_SCREW_STEP = 7,
	HE_SET_QSTEP_ENG = 8,
	HE_SET_QPULS_VAL = 9,
	HE_SET_WIND_DIR = 10,
	HE_INP_QUANT_IN_LAY = 11,
	HE_SET_WIND_MODE = 12,
	HE_SET_STEP_ENG_PULSE_LEN = 13,
	HE_SET_TYPE_ENGINE = 14,
	HE_WIND = 15,
	HE_SET_STEP_ENG_DIR_CORR = 16,
	HE_SET_WIRE_BREACK_SENSOR_ENABLE = 17,
	HE_SET_WIRE_BREACK_SENSOR_NORMAL_VAL = 18,
	HE_SET_MAX_WIND_MOTOR_SPEED = 19,
	HE_SET_WIND_MOTOR_FINISH_SPEED = 20,
	HE_SET_WIND_MOTOR_STOP_TIME = 21
};


enum WindDirEnum  {
	WDIR_RIGHT = 0,
	WDIR_LEFT = 1
};

//enum WindNModeEnum  {
	//WMODE_BY_LAYS = 0,
	//WMODE_ALL = 1
//};

#define WMODE_BY_LAYS 0
#define WMODE_ALL 1

#define OFF 0
#define ON 1

#define ST_ENG_START_STEP 1
#define ST_ENG_FINISH_STEP 0


uint8_t HandlerNum;

//-------------------
uint16_t NumForEdit;
char NumForEditText[11];
uint8_t NumEditPos;
uint8_t NumTypePos;
uint8_t NumCommaPos;
uint8_t NumLen;
uint8_t ColLCD;
uint8_t RowLCD;
uint8_t FieldType;
uint8_t FieldBytes;
int16_t* AddrEditField;
ListItem* EditList;
char WindingShowSimbol;

		//char* NumForEditText;
		char* AddrNumForEdit;


// -- Параметры аппаратуры  -------

uint16_t	ScrewStep;		// Шаг подаюего винта
uint16_t	QStepEng;		// Количество шагов двигателя укладчика за один оборот
uint16_t	QPulsVal;		// количество импульсов датчика вращения за один оборот подающего винта
uint16_t	StepEngPulseLen;// длина мепульса шагового двинателя
uint8_t		EngType;		// Тип шагового двигателя (0 - униполярный, 1 - биполятрный, 2 - внешний драйвер с интерфейсом "шаг, направление".

uint16_t	StepEng;		// перемещение укладчика за один шаг двигател = ScrewStep / QStepEng
uint16_t	StepVal;		// шаг укладки за один импульс валкодера = Step / QPulsVal

// -- Параметры намотки  ------------

uint16_t	Step;			//  шаг укладки (диаметр провода + допуск)
uint8_t	    WindMode;		//	режим намотки
uint16_t	qLays;			//	количество слоёв
uint16_t	QTurns;			//	количество витков
uint16_t	QTurnsInLay;	//	количество витков в слое
int16_t		CurQTurns;		//	текущее количество витков (уже намотано)
int16_t		CurQTurnsInLay;	//	текущее количество витков (уже намотано) в текущем слое
uint16_t	CurLay;
uint8_t		WindDir;		//	направление намотки (укладки)
uint8_t		StepperDirCorrection; // коррекция направления вращения мотора укладчика
uint8_t		WireBreakSensorEnable; // коррекция направления вращения мотора укладчика
uint8_t		WireBreakSensorNormVal; // коррекция направления вращения мотора укладчика

uint8_t		TurnDirEnkoder; //  направление вращения вала намотчика
int16_t		CurSum;		// Текущая разность позиуции укладчика и позиции намотки
int16_t		DWind;		// перемещение укладчика за один импульс енкодера
int16_t		DStEng;		// перемещение укладчика за один шаг двигателя
int16_t		PulseCount; // счетчик импульсов енкодера

uint16_t    StopTime;

uint8_t flMoveWindMotor=0;
uint8_t WinMotorSpeed = 0; 
uint8_t MaxWindMotorSpeed = 255;
uint8_t CurMaxWindMotorSpeed;
uint8_t WindMotorFinishSpeed;



char temp;


uint8_t ProgState;
uint8_t MenuState;

#define maxQFields 8   // количество полей в диалоге редаактирования кустановок
uint8_t indField = 0;  // номер поля, на котором установлен фокус (курсор)

uint16_t EEMEM	EEScrewStep;		// Шаг подаюего винта
uint16_t EEMEM	EEQStepEng;			// Количество шагов двигателя укладчика за один оборот
uint16_t EEMEM	EEQPulsVal;			// количество импульсов датчика вращения за один оборот подающего винта
uint16_t EEMEM	EEStepEngPulseLen;	// длина импульса шагового двинателя
uint16_t EEMEM	EEEngType;			// Тип шагового двигателя (0 - униполярный, 1 - биполятрный, 2 - внешний драйвер с интерфейсом "шаг, направление".

uint16_t EEMEM	EEStepperDirCorrection; // Коррекция направления шагового двигателя
uint16_t EEMEM	EEWireBreakSensorEnable; // Датчик обрыва провода включен
uint16_t EEMEM	EEWireBreakSensorNormVal;// нормальное значение датчика обрыва провода
uint16_t EEMEM	EEMaxWindMotorSpeed; //
uint16_t EEMEM	EEWindMotorFinishSpeed;
uint16_t EEMEM	EEStopTime;


//const menuItem  MenuItems[] PROGMEM =
//{  //  Nom,  Next, Previous, Parent, Child, HandlerNum, Text
	//{ 1, 2, 5, 0, 6, HE_NOP, "Settings\0"},
	//{ 2, 3, 1, 0,11, HE_NOP, "Hard_Setings\0"},
	//{ 3, 4, 2, 0, 0, HE_NOP, "Save mem\0"},
	//{ 4, 5, 3, 0, 0, HE_NOP, "Load mem\0"},
	//{ 5, 1, 4, 0, 0, HE_WIND, "Start\0"},
	//{ 6, 7,10, 1, 0, HE_SET_STEP, "Step\0"},
	//{ 7, 8, 6, 1, 0, HE_SET_QUANT, "Quant\0"},
	//{ 8, 9, 7, 1, 0, HE_INP_QUANT_IN_LAY,"Quant in lay\0"},
	//{ 9,10, 8, 1, 0, HE_SET_WIND_DIR, "Wind dir\0"},
	//{10, 6, 9, 1, 0, HE_SET_WIND_MODE, "Wind mode\0"},
	//{11,12,15, 2, 0, HE_SET_SCREW_STEP,"Screw step\0"},
	//{12,13,11, 2, 0, HE_SET_QSTEP_ENG,"steps Engine\0"},
	//{13,14,12, 2, 0, HE_SET_QPULS_VAL,"Steps valcoder\0"},
	//{14,15,13, 2, 0, HE_SET_TYPE_ENGINE,"Type engine\0"},
	//{15,16,14, 2, 0, HE_SET_STEP_ENG_PULSE_LEN,"Pulse Len\0"},
	//{16,17,15, 2, 0, HE_SET_STEP_ENG_DIR_CORR,"Stepper dir cor.\0"},
	//{17,18,16, 2, 0, HE_SET_WIRE_BREACK_SENSOR_ENABLE,"Br.sens.enable\0"},
	//{18,11,17, 2, 0, HE_SET_WIRE_BREACK_SENSOR_NORMAL_VAL,"Br.sens.norm\0"},
//} ;


const menuItem  MenuItems[] PROGMEM =
{  //  Nom,  Next, Previous, Parent, Child, HandlerNum, Text

	{ 1, 2, 3, 0, 4, HE_NOP, "Hard_Setings\0"},
	{ 2, 3, 1, 0, 0, HE_NOP, "Save mem\0"},
	{ 3, 1, 2, 0, 0, HE_NOP, "Load mem\0"},	
		
	{ 4, 5, 18, 1, 6,HE_NOP, "Stacker\0"},
	{ 5, 15, 4, 1,12,HE_NOP, "Winding motor\0"},
	
	
	{ 6, 7,11, 4, 0, HE_SET_SCREW_STEP,"Screw step\0"},
	{ 7, 8, 6, 4, 0, HE_SET_QPULS_VAL,"Steps valcoder\0"},
	{ 8, 9, 7, 4, 0, HE_SET_QSTEP_ENG,"Steps Engine\0"},
	{ 9,10, 8, 4, 0, HE_SET_TYPE_ENGINE,"Type engine\0"},
	{10,11, 9, 4, 0, HE_SET_STEP_ENG_PULSE_LEN,"Pulse Len\0"},
	{11, 6,10, 4, 0, HE_SET_STEP_ENG_DIR_CORR,"Stepper dir cor.\0"},
	
	{12,13,14, 5, 0, HE_SET_MAX_WIND_MOTOR_SPEED,"Max speed\0"},
	{13,14,12, 5, 0, HE_SET_WIND_MOTOR_FINISH_SPEED,"Finish speed\0"},
	{14,12,13, 5, 0, HE_SET_WIND_MOTOR_STOP_TIME,"Stop time\0"},
	

	{15,16, 5, 1, 0, HE_SET_WIRE_BREACK_SENSOR_ENABLE,"Br.sens.enable\0"},
	{16, 4,15, 1, 0, HE_SET_WIRE_BREACK_SENSOR_NORMAL_VAL,"Br.sens.norm\0"}
} ;


const DialogFieldStuct DialogFields[] PROGMEM =
{	//	Ind	Col	Row	ComaPos	NumLen	FieldType QBytes;
	{	0,	0,	0,	0,	4, 0, 2}, //QTurns	
	{	1,	5,	0,	0,	4, 0, 2}, //QTurnsInLay 
	{	2, 10,	0,	1,	3, 0, 2}, //Step  
	{	3,	0,	1,	0,	4, 0, 2}, //CurQTurns 
	{	4,	5,	1,	0,	4, 0, 2}, //CurQTurnsInLay 
	{	5, 10,	1,	0,	2, 0, 2}, //CurLay
	{	6, 13,	1,	0,	0, 1, 2}, //WinDir
	{	7, 15,	1,	0,	0, 1, 2}  //WinDir	
		  	
} ;

const ListItem WindDirItems[] PROGMEM =
{	//Num			Next		Prev
	{WDIR_RIGHT,	WDIR_LEFT,	WDIR_LEFT,	"-->\0", ">>\0"},
	{WDIR_LEFT,		WDIR_RIGHT, WDIR_RIGHT,	"<--\0", "<<\0"}
};

const ListItem WindModeItems[] PROGMEM =
{	//Num				Next			Prev
	{ WMODE_BY_LAYS,	WMODE_ALL,		WMODE_ALL,		"S pauzoy\0", "P\0"},
	{ WMODE_ALL,		WMODE_BY_LAYS,	WMODE_BY_LAYS,	"Bez pauz\0", "N\0"}
};

const ListItem EngineTypeItems[] PROGMEM =
{	//Num				Next			Prev
	{ 0,	1,	2,		"Unipolar\0", "\0"},
	{ 1,	2,	0,		"Bipolar\0", "\0"},
	{ 2,	0,	1,		"Ext. driver\0", "\0"}
};

const ListItem OffOnItems[] PROGMEM =
{	//Num				Next			Prev
	{ OFF,	ON,		ON,		"Off\0",	"Off\0"},
	{ ON,	OFF,	OFF,	"On\0",		"On\0"}
};

enum ProgStateEnum  {
	PS_MENU = 1,
	PS_WIND = 2,
	PS_EDIT_NUM = 3,
	PS_SET_STEP = 4,
	PS_SET_QUANT = 5,
	PS_SET_DIRECT = 6,
	PS_SET_SCREW_STEP = 7,
	PS_SET_QSTEP_ENG = 8,
	PS_SET_QPULS_VAL = 9,
	PS_SET_WIND_DIR = 10,
	PS_INP_QUANT_IN_LAY = 11,
	PS_SET_WIND_MODE = 12,
	PS_SET_STEP_ENG_PULSE_LEN = 13,
	PS_SET_TYPE_ENGINE = 14
};

const uint8_t LCDType = 2; 

//=========================================================
uint16_t atou_1(const char *buffer)
{
	char ind;
	uint16_t val;
	char Ch;

	ind = 0;
	val = 0;
	//(char)buffer--;
	while ((ind < 4) && (*buffer != 0))
	{
		
		Ch = *buffer;
		val = ((val + (val << 2)) << 1);
		val = val + Ch - 48;
		ind++;
		(char)buffer++;
	}

	return val;
}

//=========================================================
// Выводит текущий пункт меню
void ShowMenu(uint8_t MenuState, uint8_t showmode)
{
	uint8_t ind;
	uint8_t Parent;
	//char *Buffer;
	//char *CurrCodeStr;
	uint8_t indStr;

	ind = MenuState;
	ind--;
	if (ind> MAX_MENU_ITEM)
	{
		return;
	}
	
	for (indStr == 1; indStr<=LCDType; indStr++)
	{
		lcd_clrscr(indStr);
	}
	
	Parent  = pgm_read_byte(&(MenuItems[ind].Parent));
	if ((Parent == 0) | (showmode == 1))
	{
		lcd_gotoxy(0, 0);
		lcd_puts_p(&(MenuItems[ind].Text));
	}
	else
	{
		lcd_gotoxy(0, 0);
		lcd_puts_p(&(MenuItems[Parent-1].Text));
		lcd_gotoxy(3, 1);
		lcd_puts_p(&(MenuItems[ind].Text));
	}
	//lcd_putc("33");
}


//uint8_t SymbToDig()

//===================================================
void PrintNumComma(char* NumForEdit, uint8_t NumCommaPos, uint8_t NumLen)
{
	uint8_t i;
	char c;
	i=1;
	while  (c = *(NumForEdit++))  {
		if (i== NumCommaPos)
		{
			lcd_putc(',');
		}
		lcd_putc(c);
		i++;
	}
}

// ---------------------------------------------------------
char*  FormatNum(char* NumForEditText, uint8_t NumLen)
{
	//uint8_t i;
	//char c;
	//char* a[4];
	//
	//(char*)a = NumForEditText;
	//
	//i = 1;
	//i--;
	//i = 0;
	//while  (c = *a)  {
	//(char*)a++;
	//i++;
	//}
	//i = NumLen - i;
	//while (i>0)	{
	//*NumForEditText = "0" +  *NumForEditText ;
	//i--;
	//}
	//return NumForEditText;
}

//================================================
// Ввоод числа
// параметры из глобальных переменных :
//
// NumForEdit	- редактируемое число
// NumCommaPos  - позиция запятой в строчном представлении числа
// NumLen		- количество разрядов числа не учитывая запятую
// NumEditPos	- редактируемый разряд числа
// NumTypePos	- редактируемый разряд числа с учётом запятой
void EditNum(uint8_t NColLCD, uint8_t NStrLCD)
{
	//char NumForEditText[11];
	//char* AddrNumForEdit;
	char CurSymb;
	char InpDig;
	uint8_t flDigit;

	KeyCode = 0;
	EventCode = 0;
	NumEditPos = 1;	

	AddrNumForEdit = utoa_fast_div_len(NumForEdit, NumForEditText, NumLen);

	// Дополним нулями
	//AddrNumForEdit = FormatNum((char*) &NumForEditText, NumLen);
	//utoa(NumForEdit, &NumForEditText,10);
	//AddrNumForEdit = &NumForEditText;
	lcd_gotoxy(NColLCD, NStrLCD);
	//lcd_puts(AddrNumForEdit);
	PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);
	NumTypePos = NumEditPos;
	if ((NumCommaPos !=0) & (NumCommaPos <= NumEditPos))
	{
		NumTypePos++;
	}
	lcd_gotoxy(NColLCD + NumTypePos-1, NStrLCD);
	
	while (1)
	{
		flDigit = 0;
		// опрос клавиатуры
		KeyCode = GetKey();
		EventCode = KeyEvent(KeyCode);
		
		//---debug----------
		//KeyCode = KEY_OK;
		//EventCode = 1;
		//-------------------
		if (EventCode == 0)
		{
			continue;
		}
		EventCode = 0;
		
		switch (KeyCode)
		{
			case KEY_0: flDigit = 1; InpDig = 48; break;
			case KEY_1: flDigit = 1; InpDig = 49; break;
			case KEY_2: flDigit = 1; InpDig = 50; break;
			case KEY_3: flDigit = 1; InpDig = 51; break;
			case KEY_4: flDigit = 1; InpDig = 52; break;
			case KEY_5: flDigit = 1; InpDig = 53; break;
			case KEY_6: flDigit = 1; InpDig = 54; break;
			case KEY_7: flDigit = 1; InpDig = 55; break;
			case KEY_8: flDigit = 1; InpDig = 56; break;
			case KEY_9: flDigit = 1; InpDig = 57; break;
			case KEY_RIGHT	:
				lvRight:
				if (NumEditPos >= NumLen)
				{
					NumEditPos = NumLen;
				}
				else
				{
					NumEditPos++;
				}
				
				NumTypePos = NumEditPos;
				if ((NumCommaPos !=0) & (NumCommaPos <= NumEditPos))
				{
					NumTypePos++;
				}
				lcd_gotoxy(NColLCD + NumTypePos-1, NStrLCD);
				break;
			
			
			
			case KEY_LEFT	:
			
				if (NumEditPos > 1)
				{
					NumEditPos--;
				}
				else
				{
					//NumForEdit = BCDToHex(AddrNumForEdit);
					NumForEdit = atou_1((char *)AddrNumForEdit);
					return;
				}
				
				NumTypePos = NumEditPos;
				if ((NumCommaPos !=0) & (NumCommaPos <= NumEditPos))
				{
					NumTypePos++;
				}
				lcd_gotoxy(NColLCD + NumTypePos-1, NStrLCD);
				break;
			case KEY_RET	:
			
				return;
				break;
			case KEY_OK	:
				NumForEdit = atou_1((char *)AddrNumForEdit);
				return;
				break;
			
			
		}
		if (flDigit != 0)
		{
			flDigit = 0;
			(*(AddrNumForEdit+NumEditPos-1)) = InpDig;
			lcd_gotoxy(NColLCD + NumTypePos-1, NStrLCD);
			lcd_putc(*(AddrNumForEdit+NumEditPos-1));
			goto lvRight;
		}
	}
}



//================================================
// Ввоод числа
// параметры из глобальных переменных :
//
// NumForEdit	- редактируемое число
// NumCommaPos  - позиция запятой в строчном представлении числа
// NumLen		- количество разрядов числа не учитывая запятую
// NumEditPos	- редактируемый разряд числа
// NumTypePos	- редактируемый разряд числа с учётом запятой
void EditNumStruct(uint8_t NColLCD, uint8_t NStrLCD)
{
	//char NumForEditText[11];
	//char* AddrNumForEdit;
	char CurSymb;
	char InpDig;
	uint8_t flDigit;

	KeyCode = 0;
	EventCode = 0;

	AddrNumForEdit = utoa_fast_div(NumForEdit, NumForEditText);

	// Дополним нулями
	//AddrNumForEdit = FormatNum((char*) &NumForEditText, NumLen);
	//utoa(NumForEdit, &NumForEditText,10);
	//AddrNumForEdit = &NumForEditText;
	lcd_gotoxy(NColLCD, NStrLCD);
	//lcd_puts(AddrNumForEdit);
	PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);
	NumTypePos = NumEditPos;
	if ((NumCommaPos !=0) & (NumCommaPos <= NumEditPos))
	{
		NumTypePos++;
	}
	lcd_gotoxy(NColLCD + NumTypePos-1, NStrLCD);
	
	while (1)
	{
		flDigit = 0;
		// опрос клавиатуры
		KeyCode = GetKey();
		EventCode = KeyEvent(KeyCode);

		if (EventCode == 0)
		{
			continue;
		}
		EventCode = 0;
		switch (KeyCode)
		{
			case KEY_0: flDigit = 1; InpDig = 48; break;
			case KEY_1: flDigit = 1; InpDig = 49; break;
			case KEY_2: flDigit = 1; InpDig = 50; break;
			case KEY_3: flDigit = 1; InpDig = 51; break;
			case KEY_4: flDigit = 1; InpDig = 52; break;
			case KEY_5: flDigit = 1; InpDig = 53; break;
			case KEY_6: flDigit = 1; InpDig = 54; break;
			case KEY_7: flDigit = 1; InpDig = 55; break;
			case KEY_8: flDigit = 1; InpDig = 56; break;
			case KEY_9: flDigit = 1; InpDig = 57; break;
			case KEY_RIGHT	:
			lvRight:
			if (NumEditPos >= NumLen)
			{
				NumEditPos = NumLen;
			}
			else
			{
				NumEditPos++;
			}
			
			NumTypePos = NumEditPos;
			if ((NumCommaPos !=0) & (NumCommaPos <= NumEditPos))
			{
				NumTypePos++;
			}
			lcd_gotoxy(NColLCD + NumTypePos-1, NStrLCD);
			break;
			
			
			
			case KEY_LEFT	:
			
			if (NumEditPos > 1)
			{
				NumEditPos--;
			}
			else
			{
				//NumForEdit = BCDToHex(AddrNumForEdit);
				NumForEdit = atou_1((char *)AddrNumForEdit);
				return;
			}
			
			NumTypePos = NumEditPos;
			if ((NumCommaPos !=0) & (NumCommaPos <= NumEditPos))
			{
				NumTypePos++;
			}
			lcd_gotoxy(NColLCD + NumTypePos-1, NStrLCD);
			break;
			
		}
		if (flDigit != 0)
		{
			flDigit = 0;
			(*(AddrNumForEdit+NumEditPos-1)) = InpDig;
			lcd_gotoxy(NColLCD + NumTypePos-1, NStrLCD);
			lcd_putc(*(AddrNumForEdit+NumEditPos-1));
			goto lvRight;
		}
	}
}


// ========================================================
// Чтение настроек аппаратуры из EEPROM
//------------------------------------------------------
void readHardSetings()
{
	ScrewStep = eeprom_read_word(&EEScrewStep);		// Шаг подаюего винта
	if (ScrewStep > 9999) ScrewStep = 0;
	
	QStepEng = eeprom_read_word(&EEQStepEng);		// Количество шагов двигателя укладчика за один оборот
	if (QStepEng > 999) QStepEng = 0;
	
	QPulsVal = eeprom_read_word(&EEQPulsVal);		// Количество импульсов датчика вращения за один оборот подающего винта
	if (QPulsVal > 999) QPulsVal = 0;
	
	StepEngPulseLen = eeprom_read_word(&EEStepEngPulseLen);	// Длина импульса шагового двинателя
	if (StepEngPulseLen > 999) StepEngPulseLen = 0;
	
	EngType = eeprom_read_word(&EEEngType);			// Тип двигателя укоадчика
	if (EngType > 3) EngType = 1;
	
	StepperDirCorrection = eeprom_read_word(&EEStepperDirCorrection);			// 
	if (StepperDirCorrection > 1) StepperDirCorrection = 0;
	
	WireBreakSensorEnable = eeprom_read_word(&EEWireBreakSensorEnable);			//
	if (WireBreakSensorEnable > 1) WireBreakSensorEnable = 0;
	
	WireBreakSensorNormVal = eeprom_read_word(&EEWireBreakSensorNormVal);			//
	if (WireBreakSensorNormVal > 1)  WireBreakSensorNormVal = 0;
	
	MaxWindMotorSpeed = eeprom_read_word(&EEMaxWindMotorSpeed);			//
	if (MaxWindMotorSpeed > 255)  MaxWindMotorSpeed = 255;
	
	WindMotorFinishSpeed = eeprom_read_word(&EEWindMotorFinishSpeed);			//
	if (WindMotorFinishSpeed > 255)  WindMotorFinishSpeed = 255;
	
	StopTime = eeprom_read_word(&EEStopTime);			//
	if (WindMotorFinishSpeed > 9999 ) StopTime = 9999;
	
	
}


// ========================================================
// Запись настроек аппаратуры в EEPROM
//------------------------------------------------------
void writeHardSetings()
{
	eeprom_write_word(&EEScrewStep, ScrewStep);		// Шаг подаюего винта
	eeprom_write_word(&EEQStepEng, QStepEng);		// Количество шагов двигателя укладчика за один оборот
	eeprom_write_word(&EEQPulsVal, QPulsVal);		// количество импульсов датчика вращения за один оборот подающего винта
	eeprom_write_word(&EEStepEngPulseLen, StepEngPulseLen);		// длина импульса шагового двинателя
	eeprom_write_word(&EEStepperDirCorrection, StepperDirCorrection);		// длина импульса шагового двинателя
	eeprom_write_word(&EEWireBreakSensorEnable, WireBreakSensorEnable);		// длина импульса шагового двинателя
	eeprom_write_word(&EEWireBreakSensorNormVal, WireBreakSensorNormVal);		// длина импульса шагового двинателя
	eeprom_write_word(&EEMaxWindMotorSpeed, MaxWindMotorSpeed);
	eeprom_write_word(&EEWindMotorFinishSpeed, WindMotorFinishSpeed);
	eeprom_write_word(&EEStopTime, StopTime);
}



// --------------------------------------------------------
void SetStep()
{
	lcd_clrscr(1);
	lcd_clrscr(2);

	NumForEdit = Step;
	//NumForEditText =
	
	NumCommaPos = 2;	// второй символ - запятая  0,...
	NumLen = 4;			// число - 3 знака (без запятой) 0,00
	ShowMenu(MenuState, 1);
	EditNum(0, 1);
	Step = NumForEdit;
}

// --------------------------------------------------------
void SetQuant()
{
	lcd_clrscr(1);
	lcd_clrscr(2);

	NumForEdit = QTurns;
	//NumForEditText =
	
	NumCommaPos = 0;	// второй символ - запятая  0,...
	NumLen = 4;			// число - 3 знака (без запятой) 0,00
	ShowMenu(MenuState, 1);
	EditNum(0, 1);
	QTurns = NumForEdit;
	//PORTC = 17;
}

// --------------------------------------------------------
void SetQuantInLay()
{
	lcd_clrscr(1);
	lcd_clrscr(2);

	NumForEdit = QTurnsInLay;
	//NumForEditText =
	
	NumCommaPos = 0;	// второй символ - запятая  0,...
	NumLen = 4;			// число - 3 знака (без запятой) 0,00
	ShowMenu(MenuState, 1);
	EditNum(0, 1);
	QTurnsInLay = NumForEdit;
}

// --------------------------------------------------------
void SetScrewStep()
{
	lcd_clrscr(1);
	lcd_clrscr(2);

	NumForEdit = ScrewStep ;
		
	NumCommaPos = 2;//ScrewStep.CommaPos;	// второй символ - запятая  0,...
	NumLen = 4;			// число - 4 знака (без запятой) 0,00
	ShowMenu(MenuState, 1);
	EditNum(0, 1);
	ScrewStep = NumForEdit;
	eeprom_write_word(&EEScrewStep, ScrewStep); // Записываем значение в ЕЕПРОМ
}



// --------------------------------------------------------
void SetQStepEng()
{
	lcd_clrscr(1);
	lcd_clrscr(2);

	NumForEdit = QStepEng;
	
	NumCommaPos = 0;	// второй символ - запятая  0,...
	NumLen = 4;			// число - 4 знака (без запятой) 0,00
	ShowMenu(MenuState, 1);
	EditNum(0, 1);
	QStepEng = NumForEdit;
	eeprom_write_word(&EEQStepEng, QStepEng); // Записываем значение в ЕЕПРОМ

}

// --------------------------------------------------------
void SetQPulsVal()
{
	lcd_clrscr(1);
	lcd_clrscr(2);

	NumForEdit = QPulsVal;
	
	NumCommaPos = 0;	// второй символ - запятая  0,...
	NumLen = 4;			// число - 3 знака (без запятой) 0,00
	ShowMenu(MenuState, 1);
	EditNum(0, 1);
	QPulsVal = NumForEdit;
	eeprom_write_word(&EEQPulsVal, QPulsVal); // Записываем значение в ЕЕПРОМ
}

void SetWinMotorStopTime()
{
	lcd_clrscr(1);
	lcd_clrscr(2);

	NumForEdit = StopTime;
	
	NumCommaPos = 0;	// второй символ - запятая  0,...
	NumLen = 4;			// число - 3 знака (без запятой) 0,00
	ShowMenu(MenuState, 1);
	EditNum(0, 1);
	StopTime = NumForEdit;
	eeprom_write_word(&EEStopTime, StopTime); // Записываем значение в ЕЕПРОМ
}


// --------------------------------------------------------
uint8_t ChoiceListItem(ListItem* ListItems,  uint8_t  NumItem, uint8_t NColLCD, uint8_t NStrLCD, uint8_t flShort)
{
	uint8_t KeyCode;
	uint8_t EventCode;
	uint8_t NewNom;
	uint8_t CurNumItem;

	CurNumItem = NumItem;
	//CurNumItem;

	
	lcd_gotoxy(NColLCD, NStrLCD);
	if (flShort == 0)
	{
		lcd_puts_p(&(*ListItems[CurNumItem].Text));
	}
	else
	{
		lcd_puts_p(&(*ListItems[CurNumItem].Short));
	}
	lcd_gotoxy(NColLCD, NStrLCD);

	while (1)
	{
		// опрос клавиатуры
		KeyCode = GetKey();
		EventCode = KeyEvent(KeyCode);

		if (EventCode == 0) continue;


		switch (KeyCode)
		{
			//case KEY_OK		:
			case KEY_UP	:
			NewNom = pgm_read_byte(&(ListItems[CurNumItem].Previous));
			CurNumItem = NewNom;
			//lcd_clrscr(NStrLCD + 1);
			lcd_gotoxy(NColLCD, NStrLCD);
			if (flShort == 0)
			{
			lcd_puts("               ");
			lcd_gotoxy(NColLCD, NStrLCD);
			lcd_puts_p(&(*ListItems[CurNumItem].Text));	
			}
			else
			{
			lcd_puts("  ");
			lcd_gotoxy(NColLCD, NStrLCD);
			lcd_puts_p(&(*ListItems[CurNumItem].Short));	
			}
			lcd_gotoxy(NColLCD, NStrLCD);
			
			break;

			case KEY_DOWN	:
			NewNom = pgm_read_byte(&(ListItems[CurNumItem].Next));
			CurNumItem = NewNom;
			//lcd_clrscr(NStrLCD + 1);
			lcd_gotoxy(NColLCD, NStrLCD);
			if (flShort == 0)
			{
				lcd_puts("               ");
				lcd_gotoxy(NColLCD, NStrLCD);
				lcd_puts_p(&(*ListItems[CurNumItem].Text));
			}
			else
			{
				lcd_puts("  ");
				lcd_gotoxy(NColLCD, NStrLCD);
				lcd_puts_p(&(*ListItems[CurNumItem].Short));
			}
			lcd_gotoxy(NColLCD, NStrLCD);
			break;

			case KEY_LEFT	:
			case KEY_OK	:
			//if (flShort == 0)
			//{
			//lcd_clrscr(NStrLCD + 1);
			//}
			return CurNumItem;
			break;
			case KEY_RET:
			if (flShort == 0)
			{
				
				//lcd_gotoxy(NColLCD, NStrLCD);
				lcd_puts_p(&(*ListItems[NumItem].Text));
			}
			else
			{
				//lcd_gotoxy(NColLCD, NStrLCD);
				lcd_puts_p(&(*ListItems[NumItem].Short));
			}
			lcd_gotoxy(NColLCD, NStrLCD);
			return NumItem;
			
		}
	}
}


// --------------------------------------------------------
void SetWindDir()
{
	lcd_clrscr(1);
	lcd_clrscr(2);
	ShowMenu(MenuState, 1);

	WindDir = ChoiceListItem((ListItem *) &WindDirItems,  WindDir, 1, 1, 0);
	
	lcd_clrscr(1);
	lcd_clrscr(2);
	ShowMenu(MenuState, 0);
}

// --------------------------------------------------------
void SetWindMode()
{
	lcd_clrscr(1);
	lcd_clrscr(2);
	ShowMenu(MenuState, 1);

	WindMode = ChoiceListItem((ListItem *) &WindModeItems,  WindMode, 1, 1, 0);

	lcd_clrscr(1);
	lcd_clrscr(2);
	ShowMenu(MenuState, 0);
}


//EngineTyes
// --------------------------------------------------------
void SetEngType()
{
	lcd_clrscr(1);
	lcd_clrscr(2);
	ShowMenu(MenuState, 1);

	EngType = ChoiceListItem((ListItem *) &EngineTypeItems,  EngType, 1, 1, 0);

	lcd_clrscr(1);
	lcd_clrscr(2);
	ShowMenu(MenuState, 0);
	eeprom_write_word(&EEEngType, EngType); // Записываем значение в ЕЕПРОМ
}

// --------------------------------------------------------
void SetStepperDirCorrection()
{
	lcd_clrscr(1);
	lcd_clrscr(2);
	ShowMenu(MenuState, 1);

	StepperDirCorrection = ChoiceListItem((ListItem *) &OffOnItems,  StepperDirCorrection, 1, 1, 0);
	eeprom_write_word(&EEStepperDirCorrection, StepperDirCorrection); // Записываем значение в ЕЕПРОМ
	
	lcd_clrscr(1);
	lcd_clrscr(2);
	
	ShowMenu(MenuState, 0);
}

// --------------------------------------------------------
void SetWireBreakSensorEnable()
{
	lcd_clrscr(1);
	lcd_clrscr(2);
	ShowMenu(MenuState, 1);

	WireBreakSensorEnable = ChoiceListItem((ListItem *) &OffOnItems,  WireBreakSensorEnable, 1, 1, 0);
	eeprom_write_word(&EEWireBreakSensorEnable, WireBreakSensorEnable); // Записываем значение в ЕЕПРОМ
	
	lcd_clrscr(1);
	lcd_clrscr(2);
	ShowMenu(MenuState, 0);
}

// --------------------------------------------------------
void SetWireBreakSensorNormVal ()
{
	lcd_clrscr(1);
	lcd_clrscr(2);
	ShowMenu(MenuState, 1);

	WireBreakSensorNormVal = ChoiceListItem((ListItem *) &OffOnItems,  WireBreakSensorNormVal, 1, 1, 0);
	eeprom_write_word(&EEWireBreakSensorNormVal, WireBreakSensorNormVal); // Записываем значение в ЕЕПРОМ

	
	lcd_clrscr(1);
	lcd_clrscr(2);
	ShowMenu(MenuState, 0);
}

// --------------------------------------------------------
void SetStepIngPulseLen()
{
	lcd_clrscr(1);
	lcd_clrscr(2);

	NumForEdit = StepEngPulseLen;
	
	NumCommaPos = 0;	// второй символ - запятая  0,...
	NumLen = 4;			// число - 3 знака (без запятой) 0,00
	ShowMenu(MenuState, 1);
	EditNum(0, 1);
	StepEngPulseLen = NumForEdit;
	eeprom_write_word(&EEStepEngPulseLen, StepEngPulseLen); // Записываем значение в ЕЕПРОМ
}


void ShowByte(uint8_t Num)
{
    NumLen = 3;
    AddrNumForEdit = utoa_fast_div_len(Num, NumForEditText, NumLen);
	
	lcd_gotoxy(1, 1);
	NumCommaPos = 0;	// второй символ - запятая  0,...
	NumLen = 3;			// число - 4 знака
	PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);
}

// --------------------------------------------------------
void SetStepIngPulseLen2()
{
	lcd_clrscr(1);
	lcd_clrscr(2);
	ShowMenu(MenuState, 1);
	ShowByte(StepEngPulseLen);

	NumForEdit = StepEngPulseLen;
	
	NumCommaPos = 0;	// второй символ - запятая  0,...
	NumLen = 4;			// число - 3 знака (без запятой) 0,00
	ShowMenu(MenuState, 1);
	ShowByte(StepEngPulseLen);
	StepEngTimer = 0;
	while (1)
	{
		if (StepEngTimer == 0)
		{
			FinishStep();
		}

		// опрос клавиатуры
		KeyCode = GetKey();
		
		if ((KeyCode == KEY_FW) || (KeyCode == KEY_REW))
		{
			if (StepEngTimer == 0)
			{

				
				if (KeyCode == KEY_FW)
				{
					
					StartStepEng(0);
					//prevStateStepEng = ST_ENG_START_STEP;
				}
				else
				{
					StartStepEng(1);
					//prevStateStepEng = ST_ENG_START_STEP;
				}
			}
		}


		EventCode = KeyEvent(KeyCode);

		if (EventCode == 0)
		{
			continue;
		}
		switch (KeyCode)
		{
			case KEY_UP	:
			if (StepEngPulseLen < 255)
			{
			StepEngPulseLen++;
			ShowByte(StepEngPulseLen);

			}
			break;

			case KEY_DOWN	:
			{
				if (StepEngPulseLen > 1)
				{
					StepEngPulseLen--;
					ShowByte(StepEngPulseLen);
				}
			}
			break;
			
			case KEY_OK:
			eeprom_write_word(&EEStepEngPulseLen, StepEngPulseLen); // Записываем значение в ЕЕПРОМ
			return;
			break;

			case KEY_LEFT:
			return;
			break;
			default :

			break;
		}
	}
}



void SetMaxWinMotorSpeed(char fl_Finish)
{
	uint8_t CurrSpeed;
	
	lcd_clrscr(1);
	lcd_clrscr(2);
	ShowMenu(MenuState, 1);
	

	if (fl_Finish != 0)
	{
		CurrSpeed = WindMotorFinishSpeed;
		NumForEdit = WindMotorFinishSpeed;
		//ShowByte(WindMotorFinishSpeed);
	}
	else
	{
		CurrSpeed = MaxWindMotorSpeed;
		NumForEdit = MaxWindMotorSpeed;
		//ShowByte(MaxWindMotorSpeed);
	}
	
	
	NumCommaPos = 0;	// второй символ - запятая  0,...
	NumLen = 4;			// число - 3 знака (без запятой) 0,00
	ShowMenu(MenuState, 1);
	
	//if (fl_Finish != 0)
	//{
		//ShowByte(WindMotorFinishSpeed);
	//}
	//else
	//{
		//ShowByte(MaxWindMotorSpeed);
	//}
	ShowByte(CurrSpeed);
	
	
	//StepEngTimer = 0;
	while (1)
	{
		// опрос клавиатуры
		KeyCode = GetKey();
		
		
			if (KeyCode == KEY_ROTATE_F)
			{
				//if (fl_Finish != 0)
				//{
					//CurMaxWindMotorSpeed = WindMotorFinishSpeed;
				//}
				//else
				//{
					//CurMaxWindMotorSpeed = MaxWindMotorSpeed;
				//}
				CurMaxWindMotorSpeed = CurrSpeed;
				
				flMoveWindMotor = 1;
				MoveWindMotor();
			}
			else
			{
				flMoveWindMotor = 0;
				StopWindMotor();
			}
		


		EventCode = KeyEvent(KeyCode);

		if (EventCode == 0)
		{
			continue;
		}
		switch (KeyCode)
		{
			case KEY_UP	:
			if (CurrSpeed < 255)
			{
				CurrSpeed++;
				ShowByte(CurrSpeed);
			}
			break;

			case KEY_DOWN	:
			{
				if (CurrSpeed > 1)
				{
					CurrSpeed--;
					ShowByte(CurrSpeed);
				}
			}
			break;
			
			case KEY_OK:
			if (fl_Finish != 0)
			{
				WindMotorFinishSpeed = CurrSpeed;
				eeprom_write_word(&EEWindMotorFinishSpeed, WindMotorFinishSpeed); // Записываем значение в ЕЕПРОМ
			}
			else
			{
				MaxWindMotorSpeed = CurrSpeed;
				eeprom_write_word(&EEMaxWindMotorSpeed, MaxWindMotorSpeed); // Записываем значение в ЕЕПРОМ
			}
			return;
			break;

			case KEY_RET:
			return;
			break;
			default :

			break;
		}
	}
}


// =======================================================================
void StartStepEng(uint8_t StepDir)
{
	
	uint8_t CurStepDir;
	
	// корректируем направление движения шаговика
	if (StepperDirCorrection == 0)
	{
		CurStepDir = StepDir;
	}
	else
	{
		CurStepDir = StepperDirCorrection - StepDir;
	}

	switch (EngType){
		case 0:// Униполярный шаговик
		{
			
			if (phaseStepEng == 0)
			{
				phaseStepEng = 1;
			}

			if (CurStepDir ==0) // Если крутим туда
			{
				switch (phaseStepEng){
					case 0b0001:
					phaseStepEng = 0b0100;
					break;
					case 0b0100:
					phaseStepEng = 0b0010;
					break;
					case 0b0010:
					phaseStepEng = 0b1000;
					break;
					case 0b1000:
					phaseStepEng = 0b0001;
					break;
				}

			}
			else         // Если крутим сюда
			{
				switch (phaseStepEng){
					case 0b0001:
					phaseStepEng = 0b1000;
					break;
					case 0b1000:
					phaseStepEng = 0b0010;
					break;
					case 0b0010:
					phaseStepEng = 0b0100;
					break;
					case 0b0100:
					phaseStepEng = 0b0001;
					break;
				}
			}
			//ENG_PORT = phaseStepEng | ENG_PORT & 0xF0;
			ENG_PORT = (phaseStepEng<<4) | ENG_PORT & 0x0F;
		}
		break;

		case 1:// Биполярный шаговик
		{
			
			if (CurStepDir ==0) // Если крутим туда
			{
				if ((phaseStepEng >= 8) || (phaseStepEng==0))
				{
					phaseStepEng = 1;
				}
				else
				{
					phaseStepEng = phaseStepEng << 1;
				}
			}
			else         // Если крутим сюда
			{
				if ((phaseStepEng == 1) || (phaseStepEng==0) || (phaseStepEng>8))
				{
					phaseStepEng = 8;
				}
				else
				{
					phaseStepEng = phaseStepEng >> 1;
				}
			}
			//ENG_PORT = phaseStepEng | ENG_PORT & 0xF0;
			ENG_PORT = (phaseStepEng<<4) | ENG_PORT & 0x0F;
			//---------------------------------
			
		}
		break;

		case 2:  // внешний драйвер. 
		if (CurStepDir ==0) // Если крутим туда
		{
			ENG_PORT = 0b00100000 | (ENG_PORT & 0b11001111);	// на первый бит подаём сигнал шаг, на второй направление "туда"
		}
		else
		{
			ENG_PORT = 0b00110000 | (ENG_PORT & 0b11001111);	// на первый бит подаём сигнал шаг, на второй направление "сюда"
		}
		break;
	}
	StateStepEng = ST_ENG_START_STEP;
	StepEngTimer = StepEngPulseLen;
	
}

// ============================================================================
void FinishStep()
{
	ENG_PORT = ENG_PORT & 0x0F;
	StateStepEng = ST_ENG_FINISH_STEP;

}

// ============================================================================
void StartWindMotor()
{
	flMoveWindMotor = 1;	
	
 //WENG_PORT = WENG_PORT | (1<<WIND_MOTOR_PIN0);
}

// ============================================================================
void MoveWindMotor()
{
uint8_t Temp = 0;
 
	if (flMoveWindMotor == 1)
	{
		if (WindEngTimer ==0)
		{
			if (WinMotorSpeed < CurMaxWindMotorSpeed)
			{
				WindEngTimer = 125;
				Temp = WinMotorSpeed>>3;
				if (Temp == 0)
				{
					Temp++;
				}
				if ((WinMotorSpeed +  Temp)> CurMaxWindMotorSpeed)
				{
					WinMotorSpeed = CurMaxWindMotorSpeed;
				}
				else
				{
					WinMotorSpeed = WinMotorSpeed +  Temp;
				}
				if (WinMotorSpeed<10)
				{
				WinMotorSpeed = 10;
				}
				OCR1BH = 0;
				OCR1BL = WinMotorSpeed;
			}
		}
		if (WinMotorSpeed > CurMaxWindMotorSpeed)
		{
			WinMotorSpeed = CurMaxWindMotorSpeed;
		
		OCR1BH = 0;
		OCR1BL = WinMotorSpeed;
		}
	}
	//OCR1BH = 0;
	//OCR1BL = 125;
	//WENG_PORT = WENG_PORT | (1<<WIND_MOTOR_PIN0);
}
// ============================================================================
void StopWindMotor()
{
	//WENG_PORT = ~(1<<WIND_MOTOR_PIN0) & WENG_PORT;
	OCR1BH = 0;
	OCR1BL = 0;
	flMoveWindMotor = 0;
	WinMotorSpeed = 0;
}


void ShowField()
{
	
	GetFieldInfo(indField);
	
	
	lcd_gotoxy(ColLCD, RowLCD);
	if (FieldType==0)
	{
		if (FieldBytes==2){
			NumForEdit = *(uint16_t*)AddrEditField;
			AddrNumForEdit = utoa_fast_div_len(NumForEdit, NumForEditText, NumLen);
		}
		else{
			NumForEdit = (uint8_t)*AddrEditField;
			AddrNumForEdit = utoa_fast_div_len(NumForEdit, NumForEditText, NumLen);
		}
		
		PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);
		lcd_gotoxy(ColLCD, RowLCD);
	}
	else
	{
		lcd_gotoxy(ColLCD, RowLCD);
		uint8_t ListFieldFoEdit = *AddrEditField;
		//WindDir = ChoiceListItem(EditList,  ListFieldFoEdit, 1, 1, 1);
		lcd_puts_p(&(*EditList[ListFieldFoEdit].Short));
		lcd_gotoxy(ColLCD, RowLCD);
	}
}

// ============================================================================
 void ShowWinding()
{
//char NumForEditText[11];
//char* AddrNumForEdit;


//lcd_clrscr(1);
//lcd_clrscr(0);
//AddrNumForEdit = utoa_fast_div(DWind , NumForEditText);
//lcd_gotoxy(0, 0);
////lcd_puts(AddrNumForEdit);
//NumCommaPos = 0;	// второй символ - запятая  0,...
//NumLen = 4;			// число - 4 знака
//PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);


//AddrNumForEdit = utoa_fast_div(QTurns, NumForEditText);
//lcd_gotoxy(1, 0);
////utoa(QTurns, c, 10);
////lcd_puts(NumForEditText);
//
//NumCommaPos = 0;	// второй символ - запятая  0,...
//NumLen = 4;			// число - 4 знака
//PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);
//
//AddrNumForEdit = utoa_fast_div(DStEng, NumForEditText);
//lcd_gotoxy(0, 1);
//////lcd_puts(AddrNumForEdit);
//NumCommaPos = 0;	// второй символ - запятая  0,...
//NumLen = 4;			// число - 4 знака
//PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);
//
//AddrNumForEdit = utoa_fast_div(CurSum, NumForEditText);
//lcd_gotoxy(5, 1);
//////lcd_puts(AddrNumForEdit);
//NumCommaPos = 0;	// второй символ - запятая  0,...
//NumLen = 5;			// число - 4 знака
//PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);
//
//AddrNumForEdit = utoa_fast_div(QTurnsInLay, NumForEditText);
//lcd_gotoxy(6, 0);
////lcd_puts(AddrNumForEdit);
//NumCommaPos = 0;	// второй символ - запятая  0,...
//NumLen = 4;			// число - 4 знака
//PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);
//
//AddrNumForEdit = utoa_fast_div(CurQTurnsInLay, NumForEditText);
//lcd_gotoxy(6, 1);
//NumCommaPos = 0;	// второй символ - запятая  0,...
//NumLen = 4;			// число - 4 знака
//PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);
//
//AddrNumForEdit = utoa_fast_div(Step, NumForEditText);
//lcd_gotoxy(11, 0);
//NumCommaPos = 2;	// второй символ - запятая  0,...
//NumLen = 4;			// число - 4 знака
//PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);
//
//AddrNumForEdit = utoa_fast_div(CurLay, NumForEditText);
//lcd_gotoxy(11, 1);
//NumCommaPos = 0;	// второй символ - запятая  0,...
//NumLen = 4;			// число - 4 знака
//PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);

//ShowMainScreen();
indField = 3;
ShowField();
indField = 4;
ShowField();
indField = 5;
ShowField();
indField = 6;
ShowField();

lcd_gotoxy(15, 0);
lcd_putc(WindingShowSimbol);
}


void GetFieldInfo(uint8_t NumField)
{
		

			
		ColLCD  = pgm_read_byte(&(DialogFields[NumField].Col));
		RowLCD  = pgm_read_byte(&(DialogFields[NumField].Row));
		NumLen  = pgm_read_byte(&(DialogFields[NumField].NumLen));
		NumCommaPos  = pgm_read_byte(&(DialogFields[NumField].ComaPos));
		FieldType = pgm_read_byte(&(DialogFields[NumField].FieldType));
		FieldBytes = pgm_read_byte(&(DialogFields[NumField].QBytes));
		
		
		//QTurns
		//QTurnsInLay
		//Step
		//CurQTurns
		//CurQTurnsInLay
		//CurLay
		switch (indField)
		{
			case 0:
			//AddrNumForEdit = utoa_fast_div(QTurns, NumForEditText);
			AddrEditField = &QTurns;
			break;
			case 1:
			//AddrNumForEdit = utoa_fast_div(QTurnsInLay, NumForEditText);
			AddrEditField = &QTurnsInLay;
			break;
			case 2:
			//AddrNumForEdit = utoa_fast_div(Step, NumForEditText);
			AddrEditField = &Step;
			break;
			case 3:
			//AddrNumForEdit = utoa_fast_div(CurQTurns, NumForEditText);
			AddrEditField = &CurQTurns;
			break;
			case 4:
			//AddrNumForEdit = utoa_fast_div(CurQTurnsInLay, NumForEditText);
			AddrEditField = &CurQTurnsInLay;
			break;
			case 5:
			//AddrNumForEdit = utoa_fast_div(CurLay, NumForEditText);
			AddrEditField = &CurLay;
			break;
			case 6:
			//AddrNumForEdit = utoa_fast_div(CurLay, NumForEditText);
			AddrEditField = &WindDir;
			EditList = (ListItem *) &WindDirItems;
			break;
			case 7:
			//AddrNumForEdit = utoa_fast_div(CurLay, NumForEditText);
			AddrEditField = &WindMode;
			EditList = (ListItem *) &WindModeItems;
			break;
		}
}




// ============================================================================
void TabField(uint8_t Right)
{

	//uint8_t FieldType;
	
	if (Right!=0){
		indField++;
		if (indField >= maxQFields)
		{
			indField = 0;
		}
	}
	else
	{
		if (indField==0 )
		{
			indField = maxQFields-1;
		}
		else
		{
			indField--;
		}
	}
	
	ShowField();
}

// ============================================================================
void SetToField(uint8_t NumField)
{

	//uint8_t FieldType;
	
	if (NumField < maxQFields)
		{
			indField = NumField;
			ShowField();
		}
}

//=============================================================================
// редактирование поля
void EditField(uint8_t NumField)
{
	GetFieldInfo(NumField);
	if (FieldType==0)
	{
		lcd_cursor(3); 
		AddrNumForEdit = utoa_fast_div_len(*AddrEditField, NumForEditText, NumLen);
		lcd_gotoxy(ColLCD, RowLCD);
		
		if (FieldBytes == 2){
			NumForEdit = *AddrEditField;
			EditNum(ColLCD, RowLCD);
			*AddrEditField = NumForEdit;
			
			GetFieldInfo(NumField);
			AddrNumForEdit = utoa_fast_div_len(*AddrEditField, NumForEditText, NumLen);
			lcd_gotoxy(ColLCD, RowLCD);
			PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);
			lcd_gotoxy(ColLCD, RowLCD);
		}
		else if(FieldBytes==1){
			NumForEdit = (uint8_t)*AddrEditField;
			EditNum(ColLCD, RowLCD);
			*AddrEditField = (uint8_t)NumForEdit;
			
			GetFieldInfo(NumField);
			AddrNumForEdit = utoa_fast_div_len(*AddrEditField, NumForEditText, NumLen);
			lcd_gotoxy(ColLCD, RowLCD);
			PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);
			lcd_gotoxy(ColLCD, RowLCD);
		}
				
		lcd_cursor(2);
				
	}
	else
	{
		lcd_cursor(3);
		lcd_gotoxy(ColLCD, RowLCD);
		uint8_t ListFieldFoEdit = *AddrEditField;
		*AddrEditField = ChoiceListItem(EditList,  ListFieldFoEdit, ColLCD, RowLCD, 1);
		lcd_cursor(2);
	}
	
}



// ============================================================================
void EditSetingsDialog()
{
	lcd_gotoxy(15, 0);
	lcd_putc('E');

	//indField = 1;
	//TabField(0);
	SetToField(0);
	lcd_cursor(2);
	
	while (1)
	{
		KeyCode = 0;
		EventCode = 0;
		// опрос клавиатуры
		KeyCode = GetKey();
		EventCode = KeyEvent(KeyCode);
		
		////---debug
		//indField = 2;
		//KeyCode = KEY_OK;
		//EventCode = 1;
		////---------
		
		if (EventCode == 0)
		{
			continue;
		}
		

		EventCode = 0;	// перехватываем событие
		switch (KeyCode)
		{
		case KEY_RIGHT:  
			TabField(1); // переход на следующее поле (вправо, по часовой)
			break;
		case KEY_LEFT: 
			TabField(0); // переход на предыдущее поле (влево, против часовой)
			break;
		case KEY_UP:
			{
				switch (indField)
				{
				case  3:
					SetToField(0);
					break;
				case  4:
					SetToField(1);
					break;
				case  5:
				case  6:
				case  7:
					SetToField(2);
					break;
				}
			}
			break;
		case KEY_DOWN:
		{
			switch (indField)
			{
				case  0:
				SetToField(3);
				break;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
				case  1:
				SetToField(4);
				break;
				case  2:
				SetToField(5);
				break;
			}
		}
		break;
		case KEY_OK: 
			EditField(indField); // начинаем редактировать 
			lcd_cursor(2);
			break;
		case KEY_RET: 	 // заканчиваем шариться по диалогу, выходим из режима редактирования установок	
			return;
			break;
		}
			
		
	}
	
}


// ============================================================================
// Намотка
//
char Winding()
{
	uint16_t PrevCurSum;
	char CurSymb;
	char flStop = 0;
	uint16_t PrevQTurns;

	ProgState = PS_WIND;
	TurnDirEnkoder = 0;

	DStEng = ScrewStep *10 / QStepEng;  // перемещение укладчик аза один шаг двигателя
	DWind =  Step*10 / QPulsVal;	   // изменение ширины намотки за один импульс валкодера
	PrevCurSum = 9999;
	CurSum = 0;
	StateStepEng = 0;
	PrevQTurns = 0;
	StateStepEng = ST_ENG_FINISH_STEP;
	prevStateStepEng = ST_ENG_FINISH_STEP;
	phaseStepEng = 0;
	
	
	PrevQTurns = CurQTurns; 

	ShowWinding();
	
	while (1)
	{
		
		KeyCode = 0;
		EventCode = 0;
		// Индикация режим "Намотка" - мигает буква "W"
		if (WindingShowSimbolTimer == 0)
		{
			WindingShowSimbolTimer = 1000;
			if (WindingShowSimbol == 'W')
			{
				WindingShowSimbol = ' ';
			}
			else{
				WindingShowSimbol = 'W';
			}
			lcd_gotoxy(15, 0);
			lcd_putc(WindingShowSimbol);
		}
		
		// опрос клавиатуры
		KeyCode = GetKey();
		
		
		if (KeyCode == KEY_STOP)
		{
			if (flStop==0)
			{
				StopTimer = StopTime; // некоторое время, необходимое для остановки намотчик после команды остановки, продолжаем счиать витки и двигать укладчик
			}
			flStop = 1;
		}
		
		if (flStop != 0)
		{
			flMoveWindMotor = 0;
			StopWindMotor();
			if (StopTimer == 0)
			{
				FinishStep();
				ShowWinding();
				return 1;
			}
		}
		
		else
		{
			if ((CurQTurnsInLay > QTurnsInLay-10) && (WindMode == WMODE_BY_LAYS) ||(CurQTurns > QTurns-10))
			{
				if (PotentValue < 10)
				{
					CurMaxWindMotorSpeed =  PotentValue;
				}
				else
				{
					CurMaxWindMotorSpeed = 10;
				}
			}
			else
			{
				if (PotentValue < MaxWindMotorSpeed)
				{
					CurMaxWindMotorSpeed =  PotentValue;
				}
				
				else
				{
					CurMaxWindMotorSpeed = MaxWindMotorSpeed;
				}
			}
			
			flMoveWindMotor = 1;
			MoveWindMotor();
		}
		
		
		
		
		
		// считаем, отображаем что мы ту намотали
		if (CurQTurns < QTurns)
		{
			//-------------------------------
			
			if (CurQTurns != PrevQTurns)
			{
				//ShowWinding();
				
				//continue;
				// --------------------------------------
				
				if (CurQTurnsInLay<0)
				{
					CurLay--;
					CurQTurnsInLay = CurQTurnsInLay + QTurnsInLay;
				}
				else
				{
					if ((CurQTurnsInLay > QTurnsInLay-10) && (WindMode == WMODE_BY_LAYS) ||(CurQTurns > QTurns-10))
					{
						if (PotentValue < 10)
						CurMaxWindMotorSpeed =  PotentValue;
						else CurMaxWindMotorSpeed = WindMotorFinishSpeed;
					}
					else
					{
						if (PotentValue < MaxWindMotorSpeed)
						CurMaxWindMotorSpeed =  PotentValue;
						else CurMaxWindMotorSpeed = MaxWindMotorSpeed;
					}
					
					if (CurQTurnsInLay >= QTurnsInLay) // Если витков больше или  равно  максимального количества в слое
					{
						if ((WindMode == WMODE_BY_LAYS))// если режим с ожиданием между слоями
						{
							if (flStop==0)
							{
								StopTimer = StopTime;
							}
							flStop = 1;								// останавливаем двигатель намоточный
						}
						
						CurLay++;	// счетчик слоёв увеличиваем
						CurQTurnsInLay = CurQTurnsInLay - QTurnsInLay; // счетчик количества в слое уменьшаем на макс. количество в слое
						
						// тут нужно  поменять направление укладчика
						if(WindDir == WDIR_RIGHT)
						{
							WindDir = WDIR_LEFT;
						}
						else
						{
							WindDir = WDIR_RIGHT;
						}
					}
					//else     // если счетчик витков в слое не переполнен
					//{
						//if (CurQTurnsInLay == QTurnsInLay) //последний виток слоя намотан
						//{
							//if ((WindMode == WMODE_BY_LAYS))// если режим с ожиданием между слоями
							//{
								//if (flStop==0)
								//{
									//StopTimer = StopTime;
								//}
								//flStop = 1;								// останавливаем двигатель намоточный
							//}
						//}
					//}
				}
				PrevQTurns = CurQTurns;
				ShowWinding();
			}

		}
		else
		{
			if (flStop==0)
			{
				StopTimer = StopTime;
			}
			flStop = 1;
			ShowWinding();
		}
		


		// смотрим что намотали, как двигать укладчик
		// ================================================================
		if (StepEngTimer == 0)
		{
			FinishStep();
			if (TurnDirEnkoder==0) // если вал намотчика вращается в правильном направлении
			{
				//if (CurSum != PrevCurSum) // jотладка
				//{
				//ShowWinding();
				//}
				PrevCurSum = CurSum;
				
				if (CurSum < DWind)
				{

					StartStepEng(WindDir);
					//PrevCurSum = CurSum;
					CurSum = CurSum + DStEng;
					prevStateStepEng = ST_ENG_START_STEP;
				}

				//}
			}
			else  // если вал намотчика вращается в обратном направлении
			{

				PrevCurSum = CurSum;
				
				if (CurSum > DStEng)
				{
					StartStepEng(1-WindDir); // крутим в обратную сторону
					PrevCurSum = CurSum;
					CurSum = CurSum - DStEng;
					prevStateStepEng = ST_ENG_START_STEP;
				}

				
			}
		}

		
		//======================================================================
		//}
		
	}
	
	//while (StepEngTimer>0)
	//{
	//}
	FinishStep();
	return 1;
}





// ====================================================
// Обработка события выбора пункта меню
void MenuKeyEventsHandling(uint8_t HandlerNum)
{
	char res;

	switch (HandlerNum)
	{
		case HE_SET_STEP : // Установка шага намотки
		ProgState = PS_SET_STEP;
		ShowMenu(MenuState, 1);
		NumEditPos = 1;
		NumTypePos = 1;
		SetStep();
		ProgState = PS_MENU;
		ShowMenu(MenuState, 0);
		break;

		case HE_SET_QUANT :// Установка общего количества витков
		ProgState = PS_SET_QUANT;
		ShowMenu(MenuState, 1);
		NumEditPos = 1;
		NumTypePos = 1;
		SetQuant();
		ProgState = PS_MENU;
		ShowMenu(MenuState, 0);
		//lcd_puts("5");
		//lcd_puts(ProgState+48);
		break;

		case HE_SET_SCREW_STEP: // Установка шага подаюего винта
		ProgState = PS_SET_SCREW_STEP;
		ShowMenu(MenuState, 1);
		NumEditPos = 1;
		NumTypePos = 1;
		SetScrewStep();
		ProgState = PS_MENU;
		ShowMenu(MenuState, 0);
		break;

		case HE_SET_QSTEP_ENG:	// Установка количества шагов двигателя укладчика за один оборот
		ProgState = PS_SET_QSTEP_ENG;
		ShowMenu(MenuState, 1);
		NumEditPos = 1;
		NumTypePos = 0;
		SetQStepEng();
		ProgState = PS_MENU;
		ShowMenu(MenuState, 0);
		break;

		case HE_SET_QPULS_VAL:	// Установка количества импульсов датчика вращения за один оборот подающего винта
		ProgState = PS_SET_QPULS_VAL;
		ShowMenu(MenuState, 1);
		NumEditPos = 1;
		NumTypePos = 1;
		SetQPulsVal();
		ProgState = PS_MENU;
		ShowMenu(MenuState, 0);
		break;

		case HE_SET_WIND_DIR:	// Установка начального направления намотки
		ProgState = PS_SET_WIND_DIR;
		ShowMenu(MenuState, 1);
		SetWindDir();
		ProgState = PS_MENU;
		ShowMenu(MenuState, 0);
		break;

		case HE_INP_QUANT_IN_LAY :// Установка количества витков в слое
		ProgState = PS_INP_QUANT_IN_LAY;
		ShowMenu(MenuState, 1);
		NumEditPos = 1;
		NumTypePos = 1;
		SetQuantInLay();
		ProgState = PS_MENU;
		ShowMenu(MenuState, 0);
		break;

		case HE_SET_WIND_MODE :// Установка режима намотки
		ProgState = PS_SET_WIND_MODE;
		ShowMenu(MenuState, 1);
		NumEditPos = 1;
		NumTypePos = 1;
		SetWindMode();
		ProgState = PS_MENU;
		ShowMenu(MenuState, 0);
		break;

		case HE_SET_STEP_ENG_PULSE_LEN :// Установка длины импульса
		ProgState = PS_SET_STEP_ENG_PULSE_LEN;
		ShowMenu(MenuState, 1);
		NumEditPos = 1;
		NumTypePos = 1;
		SetStepIngPulseLen2();
		ProgState = PS_MENU;
		ShowMenu(MenuState, 0);
		break;

		case HE_SET_TYPE_ENGINE:	// Установка начального направления намотки
		ProgState = PS_SET_TYPE_ENGINE;
		ShowMenu(MenuState, 1);
		SetEngType();
		ProgState = PS_MENU;
		ShowMenu(MenuState, 0);
		break;

		case HE_WIND:
		res = Winding();
		ProgState = PS_MENU;
		ShowMenu(MenuState, 0);
		break;
		
		case HE_SET_STEP_ENG_DIR_CORR :	// Установка начального направления намотки
		//ProgState = PS_SET_TYPE_ENGINE;
		ShowMenu(MenuState, 1);
		SetStepperDirCorrection();
		ProgState = PS_MENU;
		ShowMenu(MenuState, 0);
		break;
		
		case HE_SET_WIRE_BREACK_SENSOR_ENABLE :	// Установка начального направления намотки
		//ProgState = PS_SET_TYPE_ENGINE;
		ShowMenu(MenuState, 1);
		SetWireBreakSensorEnable();
		ProgState = PS_MENU;
		ShowMenu(MenuState, 0);
		break;
		
		case HE_SET_WIRE_BREACK_SENSOR_NORMAL_VAL :	// Установка начального направления намотки
		//ProgState = PS_SET_TYPE_ENGINE;
		ShowMenu(MenuState, 1);
		SetWireBreakSensorNormVal ();
		ProgState = PS_MENU;
		ShowMenu(MenuState, 0);
		break;
		
		case HE_SET_MAX_WIND_MOTOR_SPEED:
		ShowMenu(MenuState, 1);
		NumEditPos = 1;
		NumTypePos = 1;
		SetMaxWinMotorSpeed(0);
		ProgState = PS_MENU;
		ShowMenu(MenuState, 0);
		break;
		
		case HE_SET_WIND_MOTOR_FINISH_SPEED:
		ShowMenu(MenuState, 1);
		NumEditPos = 1;
		NumTypePos = 1;
		SetMaxWinMotorSpeed(1);
		ProgState = PS_MENU;
		ShowMenu(MenuState, 0);
		break;
		
		case HE_SET_WIND_MOTOR_STOP_TIME:
		ProgState = PS_SET_QPULS_VAL;
		ShowMenu(MenuState, 1);
		NumEditPos = 1;
		NumTypePos = 1;
		SetWinMotorStopTime();
		ProgState = PS_MENU;
		ShowMenu(MenuState, 0);
		break;
		
	}//lcd_putc("3");
}

// ============================================================================
// Находит новый номер пункта меню по текущему номеру и коду нажатой клавиши
uint8_t JmpMenu(uint8_t Nom, uint8_t Key)
{
	uint8_t NewNom;
	uint8_t ind;

	ind = Nom-1;
	if (ind > MAX_MENU_ITEM)
	{
		return Nom;
	}
	//lcd_putc("8");
	HandlerNum = HE_NOP;

	switch (Key)
	{
		case KEY_OK		:
		case KEY_RIGHT	:
		NewNom = pgm_read_byte(&(MenuItems[ind].Child));
		if (NewNom == 0)
		{
			HandlerNum = pgm_read_byte(&(MenuItems[ind].HandlerNum));
		}
		break;
		case KEY_RET	:
		case KEY_LEFT	:
		NewNom = pgm_read_byte(&(MenuItems[ind].Parent));
		break;
		case KEY_UP		:
		NewNom = pgm_read_byte(&(MenuItems[ind].Previous));
		break;
		case KEY_DOWN	:
		NewNom = pgm_read_byte(&(MenuItems[ind].Next));
		break;
		default :
		NewNom = Nom;
		break;
	}
	if (NewNom==0) 	return Nom;
	else return NewNom;
}

void Menu()
{
	EventCode = 0;
	ShowMenu(MenuState, 0);
	//char* AddrNumForEdit;
	while (1)
	{
		// опрос клавиатуры
		KeyCode = GetKey();
		EventCode = KeyEvent(KeyCode);
		
		//EventCode = 0;
		if (EventCode == 0)
		{
			continue;
		}
		EventCode = 0;
		
		if (KeyCode==KEY_RET)
		{
			lcd_clrscr(3);
			//lcd_clrscr(0);
			return;
		}
		
		MenuState = JmpMenu(MenuState, KeyCode);
		ShowMenu(MenuState, 0);
		MenuKeyEventsHandling(HandlerNum);
	}
}


void ShowMainScreen()
{
	indField = maxQFields;
	for(uint16_t LoopInd=0;LoopInd<maxQFields;LoopInd++)
	{
		TabField(1);
	} 
	indField = maxQFields;
	TabField(1);
	
}



//===================================================
int main(void)

{
	uint8_t KeyCode;
	uint8_t Led;
	uint16_t counttime;
	uint32_t currCodeToPrint;
	uint16_t Step2;
	//char *Buffer;
	//char *CurrCodeStr;
	//char *AddrNumForEdit;
	
	//QTurns	= 3000;
	//indField = 0;
	 //EditField(indField);
	 

	readHardSetings();

	//Step = 80;
	QTurns = 100;
	QTurnsInLay = 12;
	WindMode = WMODE_BY_LAYS;
	WindDir = 1;
	
	

	//DDRD =  0b00000000;
	//PORTD = 0b00000000;

	cli();
	
	//MCUCR = //(1<<ISC11)||(1<<ISC10); //0b11000000; // прерывание сформируется по нарастающему фронту ISC11=1, ISC10=1
	//GICR = //1<<INT1;  //0b10000000; 
	//настраиваем вывод на вход
	//DDRD = 0;//&= ~ 0b00100100;//((1<<PIND2)||(1<<PIND5));
	//включаем подтягивающий резистор
	//PORTD = 0xFF;//|= 0b00100100;//((1<<PIND2)||(1<<PIND5));
	//разрешаем внешнее прерывание на int0
	GICR |= (1<<INT0)|(1<<INT1);
	//настраиваем условие прерывания
	MCUCR |= (1<<ISC01)|(0<<ISC00);

	DDRD = DDRD & 0b10110011;
	PORTD = PORTD | 0b01001100;
	  //DDRD &= ~(1<<PIN_INT0);
	  ////включаем подтягивающий резистор
	  //PORTD |= (1<<PIN_INT0);
	  ////разрешаем внешнее прерывание на int0
	  //GICR |= (1<<INT0);
	  //настраиваем условие прерывания
	  MCUCR |= (1<<ISC01)|(0<<ISC00)|(1<<ISC11)|(0<<ISC10);
	InitTimers();
	AdcInit();
		
	
	KeyScanTimer = 0;

	//TCCR1B = 0b00000001;
	WENG_DDR = WENG_DDR | (1<<WIND_MOTOR_PIN0)|(1<<WIND_MOTOR_PIN1);
	

	TCNT2 = 0;
	TCCR2 = 0b00000011;
	
	TIMSK=(1<<TOIE0)|(1<<TOIE2); // Запускаем таймер
	asm("sei");
	DDRA = 0b11110111;
	
	lcd_init();
	lcd_clrscr(1);
	lcd_clrscr(2);
	
	DDRA = DDRA & 0b11110111;


	/* Replace with your application code */
	//KEY_PORT = 0b11110000;   // старшие биты на землю, младшие к подтягивающим резюкам
	DDRC =  0b11111111;    // переводит на вывод

	Led = 0b11111111;
	//PORTC = Led;
	counttime = 0;
	KeyScanTimer = 0;
	//PrevKeyCode = 0;
	
	lcd_gotoxy(0, 0);
	//lcd_puts((char *)mm);
	//lcd_puts_p(PSTR("MENU:"));
	ProgState = PS_MENU;
	MenuState = 1;
	phaseStepEng = 1;
sei();
	//------------------------------
	//Step = 6135;

	//NumForEdit = Step;
	//NumLen = 3;
	//AddrNumForEdit = utoa_fast_div(NumForEdit, (char*) &NumForEditText);
	//Step2 = atou_1((char *)AddrNumForEdit);
	//lcd_puts(AddrNumForEdit);
	//Step2++;
	//Step2 = atou_1((char *)AddrNumForEdit);
	//lcd_puts(AddrNumForEdit);
	//Step2++;
	//AddrNumForEdit = utoa_fast_div(Step2, (char*) &NumForEditText);
	//lcd_puts(AddrNumForEdit);
	//// Дополним нулями
	//AddrNumForEdit = FormatNum((char*) AddrNumForEdit, NumLen);
	//lcd_gotoxy(1, 0);
	//lcd_puts(AddrNumForEdit);
	//NumCommaPos = 2;
	////PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);
	//while (1)
	//{}
	//------------------------------
	//lcd_puts("bebebeb");
	//while(1)
//{}
	
		//char* NumForEditText;
		//char* AddrNumForEdit;
	QTurns	= 500;
	CurQTurns = 0;
	QTurnsInLay = 50;
	CurQTurnsInLay = 0;
	Step  = 280;
	CurLay = 1;
	CurSum = 0;	
	
	MaxWindMotorSpeed = 25;
	StopTime = 4000;
	//ShowWinding();	
	
	ShowMainScreen();
	//-----debug
	//LEDFlashTimer = 255;
	//while (1)
	//{
		//KeyCode = GetKey();
		//EventCode = KeyEvent(KeyCode);
		//
		////if (EventCode == 0)
		////{
			////continue;
		////}
		//if (LEDFlashTimer ==0)
		////if (KeyCode == KEY_ROTATE_F)
		//{
			//AddrNumForEdit = utoa_fast_div(PotentValue, NumForEditText);
			//lcd_gotoxy(0, 0);
			//lcd_puts("      \0");
			//lcd_gotoxy(0, 0);
			//lcd_puts(AddrNumForEdit);
			//LEDFlashTimer = 255;
		//}
		//
	//}
	//-----
	
	//EditSetingsDialog();
	//return;	
	//continue;	
		
	//ShowMenu(MenuState, 0);
	//char* AddrNumForEdit;
	uint16_t mm = 0;
	
	lcd_cursor(1);
	while (1)
	{
		// опрос клавиатуры
		KeyCode = GetKey();
		EventCode = KeyEvent(KeyCode);
				
		if (KeyCode == KEY_ROTATE_F)
		{
			CurMaxWindMotorSpeed = MaxWindMotorSpeed;
			flMoveWindMotor = 1;
			MoveWindMotor();
		}
		else
		{
			flMoveWindMotor = 0;
			StopWindMotor();
		}
		
		switch (KeyCode)
		{
			case KEY_REW:
			if (StepEngTimer == 0)
			{
				StartStepEng(1);
			}
			break;
			
			case KEY_FW:
			
			if (StepEngTimer == 0)
			{
				StartStepEng(0);
			}
			break;
			default:
			FinishStep();
		}
		
		if (EventCode == 0)
		{
			continue;
		}
		
		//EventCode = 0;
		
		switch (KeyCode)
		{
		case KEY_MENU:
		    // показываем меню
			Menu(); 
			lcd_clrscr(1);
			lcd_clrscr(2);
			ShowMainScreen();
			break;
			
		case KEY_EDIT:
			// редактируем установки в основном окне
			EventCode = 0;
			//indField = 0;
			//for(uint16_t LoopInd=0;LoopInd<maxQFields;LoopInd++)
			//{
				//TabField(1);
			//}
			EventCode = KeyEvent(KeyCode);
			ShowMainScreen();
			lcd_gotoxy(15, 0);
			lcd_putc('E');
			EditSetingsDialog();
			lcd_gotoxy(15, 0);
			lcd_putc(' ');
			lcd_cursor(1);
			
			//ShowMainScreen();
			break;
		
		case KEY_PLAY:
		    //EventCode = KeyEvent(KeyCode);
			lcd_clrscr(1);
			lcd_clrscr(2);
			ShowMainScreen();
			Winding();
			
			lcd_clrscr(1);
			lcd_clrscr(2);
			ShowMainScreen();
			break;
		}
		
		
		
		
		

		EventCode = 0;
	}
}


// ===========================================================================
ISR(INT1_vect)
{   
    char m;
	//temp = ~temp;

	//lcd_putc("w");
	
	//if (ProgState != PS_WIND) return;
	m = PIND;
	//PORTC = temp & m;
	asm("nop");
	asm("nop");
	asm("nop");

	if((m | 0b10111111) == 0b11111111) // проверим бит направления вращения
	{ //  PORTC = temp & 0b00001100;
	//asm("nop");
	//asm("nop");
	//asm("nop");
		
		//StartStepEng(0);
		//return;
		 
		TurnDirEnkoder = 0;
		CurSum = CurSum - DWind;
		
		PulseCount++;
		if (PulseCount >= QPulsVal)
		{
			PulseCount = PulseCount - (QPulsVal);
			CurQTurns++;
			CurQTurnsInLay ++;
		}
		else
		{
			
		}
	}
	else
	{
		//StartStepEng(1);
		//return;
		//PORTC = temp & 0b00000011;
		//asm("nop");
		//asm("nop");
		//asm("nop");
		TurnDirEnkoder = 1;
		CurSum = CurSum + DWind;
		
		PulseCount--;
		if (PulseCount < 0)
		{
			PulseCount = PulseCount + QPulsVal;
			CurQTurns--;
			CurQTurnsInLay --;
		}
		else
		{
			
		}

	}
	
}


//tempCode = pgm_read_byte(&(MenuItems[6].Next));
//CurrCodeStr = utoa_fast_div(tempCode, Buffer);
////lcd_gotoxy(0, 0);
//lcd_puts(CurrCodeStr);


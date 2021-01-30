#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

//#define KEY_UP				0b00100010
//#define KEY_DOWN			0b00100001
//#define KEY_RIGHT			0b01000001
//#define KEY_LEFT			0b00010001
//#define KEY_OK				0b10000010
//#define KEY_RET				0b00010010
//#define KEY_PLAY			0b01000010

//#define KEY_UP				0b00100100 // 2/3  3/2
//#define KEY_DOWN			0b00010100 // 2/4  4/2
//#define KEY_RIGHT			0b00010010 // 3/4  4/3
//#define KEY_LEFT			0b00011000 // 1/4  4/1
//#define KEY_OK				0b00100001 // 4/3  3/4
//#define KEY_RET				0b00101000 // 1/3  3/1
//#define KEY_PLAY			0b01000001 // 4/2  2/4
//#define KEY_REW 			0b10000010 // 3/1  1/3
//#define KEY_FW 			    0b10000001 // 4/1  1/4

//0001 0001   0010 0001   0100 0001   1000 0001
//0001 0010   0010 0010   0100 0010   1000 0010
//0001 0100   0010 0100   0100 0100   1000 0100
//0001 1000   0010 1000   0100 1000   1000 1000
//										2 в степени колонка, ряд
#define KEY_UP				0b00100100 // x24  
#define KEY_DOWN			0b00101000 // x28  
#define KEY_RIGHT			0b01001000 // x48  
#define KEY_LEFT			0b00011000 // x18
#define KEY_OK				0b10000100 // x84
#define KEY_RET				0b10001000 // х88    //0b10000001 // x14
#define KEY_PLAY			0b10000010 // x82
#define KEY_REW 			0b01000001 // x41
#define KEY_FW 			    0b10000001 // x81

#define KEY_0				0b00101000 // х28
#define KEY_1				0b00010100 // х14
#define KEY_2				0b00100100 // х24
#define KEY_3				0b01000100 // х44
#define KEY_4				0b00010010 // х12
#define KEY_5				0b00100010 // х22
#define KEY_6				0b01000010 // х42 
#define KEY_7				0b00010001 // х11
#define KEY_8				0b00100001 // х21
#define KEY_9				0b01000001 // х41

#define KEY_EDIT			0b10000100 // х84
#define KEY_MENU			0b00010100 // х14  0b10001000
#define KEY_STOP			0b10001000 // х88
#define KEY_ROTATE_F		0b01000010 // х42 
#define KEY_ROTATE_R		0b01000100 // х44

#define KEY_POS_STAKER_A	0b00010010 // х12
#define KEY_POS_STAKER_B	0b00100010 // х22
#define KEY_MOVE_TO_POS_STAKER_A	0b00010001 // х11
#define KEY_MOVE_TO_POS_STAKER_B	0b00100001 // х21





//void  KBD_init();
unsigned char GetKey(void);
unsigned char  KeysReq();
unsigned char KeyEvent(uint8_t KeyCode);

#endif

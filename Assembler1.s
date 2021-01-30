
/*
 * Assembler1.s
 *
 * Created: 28.02.2017 23:59:35
 *  Author: VL
 */ 
 ; ====================================================
; KeysReq  Подпрограмма единоразового опроса матричной клавиатуры
; сохраняет результат опроса в scanKeyCode
; ====================================================
KeysReq:
push	temp
push	temp1
push	indKeysStr
push	indKeysCol
;       проверим столбцы
clr		indKeysCol
clr		indKeysStr

ldi		temp, 0b00001111    
out     KEY_PORT, temp				; старшие биты на землю, младшие к подтягивающим резюкам
nop
ldi		temp, 0b11110000;
out		KEY_DDR, temp				; переводит старшие 4 бита порта B на вывод, младшие на ввод 
nop

ldi		indKeysCol, 0b00001000	; взвели бит 3 в индексе строк

LoopKeysCol:
in		temp, KEY_PIN
nop
and		temp, indKeysCol
breq	ResKeysCol				; если ноль, то кнопка нажата, колонка айдена, выходим из цикла

lsr		indKeysCol				; сдвигаем проверочный бит вправо
brne	LoopKeysCol				; если не 0 тогда позврат

ResKeysCol:


cpi	indKeysCol, 0			; если клавиша не нажата, то валим в лес
breq	ResKeysStr

; --- проверим строки
ldi		temp, 0b11110000    
out     KEY_PORT, temp				; младшие биты (столбцы) на землю, старшие (строки) к подтягивающим резюкам
nop
ldi		temp, 0b00001111		;
out		KEY_DDR, temp				; переводит младшие 4 бита (столбцы) порта D на вывод, старшие (строки) на ввод 
nop
ldi		indKeysStr, 0b00010000 	; взвели бит 3 в индексе строк

LoopKeysStr:
in		temp, KEY_PIN
nop

and		temp, indKeysStr
breq	ResKeysStr				; если ноль, то кнопка нажата, колонка найдена
				
lsl		indKeysStr				; сдвигаем проверочный бит влево
brne	LoopKeysStr				; если проверочный бит ещё не перенёсся в "С"-перенос, то листаем дальше

ResKeysStr:
clr		temp1

or		temp1, indKeysCol
or		temp1, indKeysStr

sts		scanKeyCode,	temp1
;out		PortA, 	temp1
nop
nop	


pop		indKeysCol
pop		indKeysStr
pop		temp1
pop		temp

ret     ; выйти из подпрограммы KeysReq
;================================================

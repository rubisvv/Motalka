
/*
 * Assembler1.s
 *
 * Created: 28.02.2017 23:59:35
 *  Author: VL
 */ 
 ; ====================================================
; KeysReq  ������������ ������������� ������ ��������� ����������
; ��������� ��������� ������ � scanKeyCode
; ====================================================
KeysReq:
push	temp
push	temp1
push	indKeysStr
push	indKeysCol
;       �������� �������
clr		indKeysCol
clr		indKeysStr

ldi		temp, 0b00001111    
out     KEY_PORT, temp				; ������� ���� �� �����, ������� � ������������� �������
nop
ldi		temp, 0b11110000;
out		KEY_DDR, temp				; ��������� ������� 4 ���� ����� B �� �����, ������� �� ���� 
nop

ldi		indKeysCol, 0b00001000	; ������ ��� 3 � ������� �����

LoopKeysCol:
in		temp, KEY_PIN
nop
and		temp, indKeysCol
breq	ResKeysCol				; ���� ����, �� ������ ������, ������� ������, ������� �� �����

lsr		indKeysCol				; �������� ����������� ��� ������
brne	LoopKeysCol				; ���� �� 0 ����� �������

ResKeysCol:


cpi	indKeysCol, 0			; ���� ������� �� ������, �� ����� � ���
breq	ResKeysStr

; --- �������� ������
ldi		temp, 0b11110000    
out     KEY_PORT, temp				; ������� ���� (�������) �� �����, ������� (������) � ������������� �������
nop
ldi		temp, 0b00001111		;
out		KEY_DDR, temp				; ��������� ������� 4 ���� (�������) ����� D �� �����, ������� (������) �� ���� 
nop
ldi		indKeysStr, 0b00010000 	; ������ ��� 3 � ������� �����

LoopKeysStr:
in		temp, KEY_PIN
nop

and		temp, indKeysStr
breq	ResKeysStr				; ���� ����, �� ������ ������, ������� �������
				
lsl		indKeysStr				; �������� ����������� ��� �����
brne	LoopKeysStr				; ���� ����������� ��� ��� �� �������� � "�"-�������, �� ������� ������

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

ret     ; ����� �� ������������ KeysReq
;================================================

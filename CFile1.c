/*
 * CFile1.c
 *
 * Created: 20.01.2020 23:20:21
 *  Author: Vl
 */ 

ISR(TIMER0_OVF_vect)
{
	TCNT0 = 26;
	if (KeyScanTimer > 0){
		KeyScanTimer--;
	}

	if (KeyEventTimer>0){
		KeyEventTimer--;
	}
	

	if (LEDFlashTimer>0){
		LEDFlashTimer--;
	}

	if (StepEngTimer > 0){
		StepEngTimer--;
	}

	if (WindEngTimer > 0){
		WindEngTimer--;
	}

	//uint8_t i;
	//for(i=0;i!=Max_Numbers_of_Timer;i++)
	//{
	//
	//if(SoftTimer[i] !=0)		 // ���� ������ �� ��������, �� ������� ��� ���.
	//{
	//SoftTimer[i] --;	 // ��������� ����� � ������ ���� �� �����.
	//}
	//
	//}

}

ISR(TIMER2_OVF_vect)
{
	//TCNT1 = 26;
	//if (TikTimer2 > 0)
	//{
	//TikTimer2--;
	//}

	

}

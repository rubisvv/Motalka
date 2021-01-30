#include "BCD_DHALT.h"



//=============DIHALT===============================



//----------------------------------------------------------
char* utoa_fast_div(uint32_t value, char *buffer)
{
	uint8_t Ind;
	Ind = 0;
	buffer += 11;
	*--buffer = 0;
	do
	{
		divmod10_t res = divmodu10(value);
		*--buffer = res.rem + '0';
		value = res.quot;
		Ind++;

	}
	while (value != 0);
	while (Ind<4) 
	{
	*--buffer = 0 + '0';
	Ind++;
	}
	return buffer;
}


//----------------------------------------------------------
char* utoa_fast_div_len(uint32_t value, char *buffer, uint8_t NumLen)
{
	uint8_t Ind;
	Ind = 0;
	
	buffer += 11;
	*--buffer = 0;
	do
	{
		divmod10_t res = divmodu10(value);
		*--buffer = res.rem + '0';
		value = res.quot;
		Ind++;

	}
	while (value != 0 && Ind<NumLen+1);
	while (Ind<NumLen)
	{
		*--buffer = 0 + '0';
		Ind++;
	}
	return buffer;
}



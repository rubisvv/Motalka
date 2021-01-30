#include <avr/io.h>

typedef struct
{
	uint32_t quot;
	uint8_t rem;
}divmod10_t;

//inline static divmod10_t divmodu10(uint32_t n);
inline static divmod10_t divmodu10(uint32_t n)
//divmod10_t divmodu10(uint32_t n)
{
	divmod10_t res;
	// умножаем на 0.8
	res.quot = n >> 1;
	res.quot += res.quot >> 1;
	res.quot += res.quot >> 4;
	res.quot += res.quot >> 8;
	res.quot += res.quot >> 16;
	uint32_t qq = res.quot;
	// делим на 8
	res.quot >>= 3;
	// вычисляем остаток
	res.rem = (uint8_t)(n - ((res.quot << 1) + (qq & ~7ul)));
	// корректируем остаток и частное
	if(res.rem > 9)
	{
		res.rem -= 10;
		res.quot++;
	}
	return res;
}

char* utoa_fast_div(uint32_t value, char *buffer);
char* utoa_fast_div_len(uint32_t value, char *buffer, uint8_t NumLen);

//uint32_t atou(const char *buffer);
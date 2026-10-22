#include "hall.h"
#include "stdio.h"
	uint8_t u1;
	uint8_t v1;
	uint8_t w1;

uint8_t GetHall_Value(void)
{
	uint8_t u;
	uint8_t v;
	uint8_t w;

	
	u = HALLU_READ();
	v = HALLV_READ();
	w = HALLW_READ();
	if((u1!=u)||(v1!=v)||(w1!=w))
	{
		u1=u;
		v1=v;
		w1=w;
		printf("HallState:w:%d,v:%d,u:%d\r\n",w,v,u);
	}
}






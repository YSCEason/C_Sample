#include "stdio.h"

void main()
{
	short Val = -25579;
	unsigned short result = 0;

	result = (unsigned short)Val;

	printf("Val = %x\n", Val);
	printf("Val = %x\n", result);

}
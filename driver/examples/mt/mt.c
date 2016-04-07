#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "ltmk.c"
int _card_number = 0;
TTmkEventData tmkEvD;
int mtLastBase = 0;
uint16_t mtMaxBase = 0;
uint32_t events_count=0;
int main(int argc,char **argv)
{
	_hVTMK4VxD= open("/dev/tmk1553b", 0);
	if (!_hVTMK4VxD)
	{
		printf ("Could not open /dev/tmk1553b\n");
		exit(0);
	}
	printf ("version %08X\n", ioctl(_hVTMK4VxD, TMK_IOCGetVersion, 0));
	int result_init = 0;
	result_init = tmkconfig(_card_number);
	if (result_init)
	{
		printf("Could not configure device\n");
		exit(0);
	}

	result_init =   tmkselect(_card_number);
	if (result_init)
	{
		printf("Could not select device \n");
		exit(0);
	}

	result_init = mtreset();

	result_init|=mtdefirqmode(RT_GENER1_BL|RT_GENER2_BL);
	if (result_init)
	{
		printf("could not execute mtreset on device \n");
	}
	mtMaxBase = mtgetmaxbase();
	ushort i=0;
	for (i = 0; i < mtMaxBase; ++i)
	{
		mtdefbase(i);
		mtdeflink((ushort)(i + 1), (ushort)(CX_CONT | CX_NOINT | CX_SIG));
	}
	mtdefbase(i);
	mtdeflink(0, (ushort)(CX_CONT | CX_NOINT | CX_SIG));




	////////////////////
	int events;
	int result=0;
	int32_t waitingtime = 100;
	uint16_t buffer[64];
	uint16_t sw;
	mtstop();
	mtstartx(0, (ushort)(CX_CONT | CX_NOINT | CX_NOSIG));
	while (1)
	{
		events = tmkwaitevents(1 << _card_number, waitingtime);
		if (events == (1 << _card_number))
		{
			memset(buffer,0,64*2);
			tmkselect(_card_number);
			tmkgetevd(&tmkEvD);
			if (tmkEvD.nInt == 3)
			{

			}
			else if (tmkEvD.nInt == 4)
			{

				while (mtLastBase != tmkEvD.mt.wBase)
				{
					events_count++;
					if ((result = mtdefbase(mtLastBase)) != 0)
					{
						printf("error mtdefbase\n");
					}
					sw = mtgetsw();
					ushort statusword = tmkEvD.mt.wResultX;
					mtgetblk(0, &buffer[0], 64);
					printf("received data %04X\n",buffer[0]);
					buffer[63] = statusword;
					buffer[58] = sw;

					++mtLastBase;
					if (mtLastBase > mtMaxBase)
						mtLastBase = 0;

				}
			}

			else
			{
				printf("Other interrupt type in ListenLoopMT(): ");
			}


		}
	}
	///////////////////


}

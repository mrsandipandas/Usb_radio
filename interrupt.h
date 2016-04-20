#ifndef _interrupt_INCLUDED_
#define _interrupt_INCLUDED_

#define NGPRSBUF        4         //Number of GPRS buffers

extern volatile char                GPRSbuf[NGPRSBUF][65];           //5 GPRS buffer on receive 65 byte each. The 65[64] byte indicates buffer in use. 0=free 0xFF=in use

void start0(char time, char flag);
unsigned long int int_random(void);

#endif

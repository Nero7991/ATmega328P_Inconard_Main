/*
 * ProjectDef.h
 *
 * Created: 3/18/2019 3:56:50 PM
 *  Author: orencollaco
 */ 


#ifndef PROJECTDEF_H_
#define PROJECTDEF_H_

//#define F_CPU 8000000L
#include <inttypes.h>
#define NO_MILLIS
//#define  DEBUG_SWITCH
#define STATS
#define INDICATE_LED	GET_BITFIELD(PORTD).bit7

#define LINK_LED		GET_BITFIELD(PORTD).bit7
#define LINK_LED_D		GET_BITFIELD(DDRD).bit7

#define CSN				GET_BITFIELD(PORTC).bit0
#define CSN_D			GET_BITFIELD(DDRC).bit0
#define CE				GET_BITFIELD(PORTC).bit1
#define CE_D			GET_BITFIELD(DDRC).bit1
#define BEEP			GET_BITFIELD(PORTC).bit3

#define DEBUG_PIN		GET_BITFIELD(DDRB).bit6
#define DEBUG_PIN_D		GET_BITFIELD(DDRB).bit6

typedef void (*Fptr)(uint8_t);

typedef struct bitstruct
{
	unsigned char bit0 : 1;
	unsigned char bit1 : 1;
	unsigned char bit2 : 1;
	unsigned char bit3 : 1;
	unsigned char bit4 : 1;
	unsigned char bit5 : 1;
	unsigned char bit6 : 1;
	unsigned char bit7 : 1;
}bit_field;
#define GET_BITFIELD(addr)( *((volatile bit_field*)&(addr))  )

//USART Baud rate
#define F_CPU 16000000UL
#define BAUD 500000
#define MYUBRR F_CPU/16/BAUD-1


#endif /* PROJECTDEF_H_ */
/*
 * ATmega328P_Inconard_Main.cpp
 *
 * Created: 3/18/2019 3:50:23 PM
 * Author : orencollaco
 */ 

#include "ProjectDef.h"
#include <avr/io.h>
#include "AVR.h"
#include "Essential.h"
#include "NRF24L01.h"
#include "Timer.h"
#include "Switch.h"

void printSomething(uint8_t);
void runSetup();
void pullSlaveSelectLow(uint8_t SwitchID);
void pullSlaveSelectHigh(uint8_t SwitchID);
void portStateChange(uint8_t PortNo);
volatile uint8_t SPIdata;
volatile bool newSPIData, Timer1_Flag;
uint8_t Socket, State, Payload, StateAll;
int main(void)
{
    runSetup();
	_delay_ms(500);
	Notify(PSTR("Powering on..."));
	NRF24L01 Radio(1,1,1);
	Radio.powerON(true);
	initSPISlave();
	enableSPIInterrupt(true);
	sei();
	TimerClass Timer1;
	SwitchClass S;
	S.begin();
	S.callOnPinStateChange(portStateChange);
	Timer1.begin();
	Timer1.initializeTimer();
	Timer1.setCallBackTime(100, 0, printSomething);
	Notify(PSTR("Done"));
    while (1) 
    {
		if(newSPIData)	{
			newSPIData = false;
			if(SPIdata != 0xAA){
				Socket = SPIdata & 0xF0;
				Socket = Socket >> 4;
				State = SPIdata & 0x0F;
				setPinState(PORT_D, Socket + 1, State);
				printStringCRNL("Command received: ");
				printHexNumber(SPIdata, 1);
			}
			else{
				enableSPIInterrupt(false);
				enableSPI(false);
				SPI_MasterInit();
				if(!Radio.isRXEmpty()){
					Radio.readFIFO(&Payload);
					printStringCRNL("Data received: ");
					printHexNumber(Payload, 1);
				}
				//printStringCRNL("Tick");
				enableSPI(false);
				initSPISlave();
				enableSPIInterrupt(true);
			}
			SPDR = PIND >> 2;
		}
		if(Timer1_Flag){
			Timer1_Flag = false;
			printStringCRNL("Timer done.");
		}
    }
}

void printSomething(uint8_t Timer_ID){
	Timer1_Flag = true;
}

void portStateChange(uint8_t PortNo){
	printStringCRNL("Pin state changed");
	if(PortNo == PORT_C){
		setPinState(PORT_C, 2, getPinState(PORT_C, 0));
	}
}

void runSetup(){
	
	USART_Init(MYUBRR);
	Init_CTC_T1(2,2000);
	setPinDirection(PORT_C, 2, OUTPUT);
	setPinDirection(PORT_D, 2, OUTPUT);
	setPinDirection(PORT_D, 3, OUTPUT);
	setPinDirection(PORT_D, 4, INPUT);
	setPinDirection(PORT_D, 5, OUTPUT);
	setPinState(PORT_D, 2, LOW);
	setPinState(PORT_D, 3, LOW);
	setPinState(PORT_D, 4, LOW);
	setPinState(PORT_D, 5, LOW);
	setPinState(PORT_C, 2, LOW);
	setPinDirection(PORT_C, 0, INPUT);
	SPI_MasterInit();
	//enableSPIInterrupt(true);
	//sei();
}

ISR(SPI_STC_vect)
{															//Grab the data byte from the SPI Data Register (SPDR)
	SPIdata = SPDR;                                         //Put the byte into a temporary variable for processin                                       //process the data byte and put it back into the SPDR for the Master to read it
	newSPIData = true;                                       //Set the Flag as TRUE
}
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
#define BEEP_TIME 30
#define SOCKET1	0x03
#define SOCKET2 0x02
#define SOCKET3 0x05
#define SOCKET4	0x04
void timerDone(uint8_t Timer_ID);
void switchPressed(uint8_t Switch_ID);
void setAllSocketState(bool set);
void setSocketState(uint8_t SocketNo, bool set);
bool getSocketState(uint8_t SocketNo);
void runSetup();
void pullSlaveSelectLow(uint8_t SwitchID);
void pullSlaveSelectHigh(uint8_t SwitchID);
void portStateChange(uint8_t PortNo);
volatile uint8_t SPIdata, SwitchID;
volatile bool newSPIData, Timer1_Flag, SwitchFlag = false;
uint8_t Socket, State, Payload, StateAll, CurrentState;
TimerClass Timer1;
SwitchClass S1, S2, S3, S4;
int main(void)
{
    runSetup();
	_delay_ms(500);
	Notify(PSTR("Powering on..."));
	NRF24L01 Radio(1,1,1);
	Radio.enableReceiveAddress(2, true);
	Radio.enableReceiveAddress(3, true);
	Radio.enableReceiveAddress(4, true);
	Radio.enableReceiveAddress(5, true);
	Radio.enableDPLForRXPipe(0, true);
	Radio.enableDPLForRXPipe(1, true);
	Radio.enableDPLForRXPipe(2, true);
	Radio.enableDPLForRXPipe(3, true);
	Radio.enableDPLForRXPipe(4, true);
	Radio.enableDPLForRXPipe(5, true);
	Radio.setReceiveAddress(0xEABABABAC1, 0);
	Radio.setReceiveAddress(0xEABABABAC2, 1);
	Radio.powerON(true);
	initSPISlave();
	enableSPIInterrupt(true);
	sei();
	Timer1.begin();
	Timer1.initializeTimer();
	Timer1.setCallBackTime(100, 0, timerDone);
	S1.begin();
	S1.initializeSwitch(PORT_D, 6, &S1);
	S2.initializeSwitch(PORT_D, 7, &S2);
	S3.initializeSwitch(PORT_B, 0, &S3);
	S4.initializeSwitch(PORT_B, 1, &S4);
	S1.shortPress(switchPressed);
	S1.enableSamePtrMode(true);
	Notify(PSTR("Done"));
    while (1) 
    {
		if(newSPIData)	{
			newSPIData = false;
			if(SPIdata != 0xAA){
				Socket = SPIdata & 0xF0;
				Socket = Socket >> 4;
				State = SPIdata & 0x0F;
				if((SPIdata & 0xD0) == 0xD0){
					printStringCRNL("Changing all");
					setAllSocketState(State);
				}
				else{
				setSocketState(Socket, State);
				}
				BEEP = 1;
				Timer1.setCallBackTime(BEEP_TIME, 0, timerDone);
				printStringCRNL("Command received: ");
				printHexNumber(SPIdata, 1);
			}
			else{
				enableSPIInterrupt(false);
				enableSPI(false);
				SPI_MasterInit();
				_delay_us(50);
				if(Radio.isDataReady()){
					Radio.readFIFO(&Payload);
					Radio.clearRX_DR();
					Radio.flushRX();
					if(Payload & 0xF0){
						Socket = Payload & 0xF0;
						Socket = Socket >> 4;
						State = Payload & 0x0F;
						if((Payload & 0xD0) == 0xD0){
							setAllSocketState(State);
						}
						else{
							setSocketState(Socket, State);
						}
					}
					else{
						setSocketState(Payload, !getSocketState(Payload));
					}
					BEEP = 1;
					Timer1.setCallBackTime(BEEP_TIME, 0, timerDone);
					printStringCRNL("Data received: ");
					printHexNumber(Payload, 1);
				}
				//printStringCRNL("Tick");
				enableSPI(false);
				initSPISlave();
				enableSPIInterrupt(true);
			}
			CurrentState = (getSocketState(1) << 0) | (getSocketState(2) << 1) | (getSocketState(3) << 2) | (getSocketState(4) << 3);
			SPDR = CurrentState;
		}
		if(SwitchFlag){
			SwitchFlag = false;
			setSocketState(SwitchID + 1, !getSocketState(SwitchID + 1));
			BEEP = 1;
			Timer1.setCallBackTime(BEEP_TIME, 0, timerDone);
		}
		if(Timer1_Flag){
			Timer1_Flag = false;
			//printStringCRNL("Timer done.");
		}
    }
}

void setAllSocketState(bool set){
	setPinState(PORT_D, SOCKET1, set);
	setPinState(PORT_D, SOCKET2, set);
	setPinState(PORT_D, SOCKET3, set);
	setPinState(PORT_D, SOCKET4, set);
}

void setSocketState(uint8_t SocketNo, bool set){
	switch(SocketNo){
		case 1:
		setPinState(PORT_D, SOCKET1, set);
		break;
		case 2:
		setPinState(PORT_D, SOCKET2, set);
		break;
		case 3:
		setPinState(PORT_D, SOCKET3, set);
		break;
		case 4:
		setPinState(PORT_D, SOCKET4, set);
		break;
	}
}

bool getSocketState(uint8_t SocketNo){
	switch(SocketNo){
		case 1:
		return getPinState(PORT_D, SOCKET1);
		break;
		case 2:
		return getPinState(PORT_D, SOCKET2);
		break;
		case 3:
		return getPinState(PORT_D, SOCKET3);
		break;
		case 4:
		return getPinState(PORT_D, SOCKET4);
		break;
	}
	return 0;
}

void switchPressed(uint8_t Switch_ID){
	SwitchFlag = true;
	SwitchID = Switch_ID;
}

void timerDone(uint8_t Timer_ID){
	Timer1_Flag = true;
	if(BEEP)
	BEEP = 0;
}

void portStateChange(uint8_t PortNo){
	printStringCRNL("Pin state changed");
	if(PortNo == PORT_C){
		setPinState(PORT_C, 2, getPinState(PORT_C, 0));
	}
}

void runSetup(){
	
	USART_Init(MYUBRR);
	//Init_CTC_T1(2,2000);
	setPinDirection(PORT_C, 2, OUTPUT);
	setPinDirection(PORT_C, 3, OUTPUT);
	setPinDirection(PORT_D, 2, OUTPUT);
	setPinDirection(PORT_D, 3, OUTPUT);
	setPinDirection(PORT_D, 4, OUTPUT);
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
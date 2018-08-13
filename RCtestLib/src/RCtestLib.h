/*
 Name:		RCtestLib.h
 Created:	3/28/2017 10:45:32 AM
 Author:	John
 Editor:	http://www.visualmicro.com
*/


/**********************************************************************************************
Bescor Pinout looking at pins rotating clockwise from lower left
Lower left - Speed (unused)
             LEFT
             DOWN
Top - 5V
             UP
			 Right
Lower Right  GND
***********************************************************************************************/
#ifndef _RCtestLib_h
#define _RCtestLib_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#define ZOOM_INTERRUPT 0 // INTERRUPT 0 = DIGITAL PIN 2 - use the interrupt number in attachInterrupt
#define ZOOM_SIGNAL_PIN 2 // INTERRUPT 0 = DIGITAL PIN 2 - use the PIN number in digitalRead

#define XAXIS_INTERRUPT 1 //Interrupt 1 = Digital pin 3
#define XAXIS_SIGNAL_PIN 3 //Pin used to read data about Xaxis --- Mega
#define XAXIS_INTERRUPT_UNO 1 //Interrupt 1 = Digital pin 3
#define XAXIS_SIGNAL_PIN_UNO 3 //Pin used to read data about Xaxis --- Mega


#define YAXIS_INTERRUPT 3 //Interrupt 3 for Yaxis on Pin 18
#define YAXIS_SIGNAL_PIN 20 //Pin to collect data from ---Mega
#define YAXIS_INTERRUPT_UNO 0 //Interrupt 3 for Yaxis on Pin 18
#define YAXIS_SIGNAL_PIN_UNO 5 //Pin to collect data from ---Mega


#define RECORD_INTERRUPT 5 //Interrupt 3 for Yaxis on Pin 18
#define RECORD_SIGNAL_PIN 18 //Pin to collect data from 

#define NEUTRAL_STICK 1500 // this is the duration in microseconds of neutral throttle on an electric RC Car
// These are the arduino pins that map to the individual u/d/l/r pins omn the bescor
#define pinToMoveRight  9
#define pinToMoveLeft  8
#define pinToMoveUp 10
#define pinToMoveDown 22
#define pinToMoveRight_UNO  9 // All PWM pins on UNO are used
#define pinToMoveLeft_UNO   10
#define pinToMoveUp_UNO  11
#define pinToMoveDown_UNO  6




//LANC Data
#define cmdPin 7  //original	
#define lancPin 11 //original

#define recButton 6
#define zoomOutButton 5
#define zoomInButton 4
#define focusNearButton 3
#define focusFarButton 2

enum device_type {
	generic,
	Bescor,
	LANC
};

#endif


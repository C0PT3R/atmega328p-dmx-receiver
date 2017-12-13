/*
	AVRage.h - main header
	Copyright © Elie Gendron 2016
	
	AVRage - My personal AVR library
*/

#ifndef AVRage_h
#define AVRage_h


#define PIN_D0 0
#define PIN_D1 1
#define PIN_D2 2
#define PIN_D3 3
#define PIN_D4 4
#define PIN_D5 5
#define PIN_D6 6
#define PIN_D7 7

#define PIN_B0 8
#define PIN_B1 9
#define PIN_B2 10
#define PIN_B3 11
#define PIN_B4 12
#define PIN_B5 13

#define PIN_C0 14
#define PIN_C1 15
#define PIN_C2 16
#define PIN_C3 17
#define PIN_C4 18
#define PIN_C5 19

#define LOW 0
#define HIGH 1

#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2


#include <avr/io.h>
#include "DigitalPin.h"
#include "Button.h"

#endif
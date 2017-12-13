/*
 *  DigitalPin.h - digital pin management
 *  Copyright © Elie Gendron 2016-2017
 *
 *  AVRage - My personal AVR library
 */

#ifndef DigitalPin_h
#define DigitalPin_h


#include "AVRage.h"

class DigitalPin {
	public:
		DigitalPin(uint8_t pinNumber);
		DigitalPin(uint8_t pinNumber, uint8_t pmode);
		DigitalPin(uint8_t pinNumber, uint8_t pmode, uint8_t state);

		void    set_mode (uint8_t pmode);
		uint8_t read     ();
		void    write    (uint8_t value);
		
	private:
		void _get_pin_data(uint8_t pin);
		
		volatile uint8_t *_direction;
		volatile uint8_t *_output;
		volatile uint8_t *_input;
		uint8_t           _mask;
};


#endif
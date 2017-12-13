/*
 *  DigitalPin.cpp - digital pin management
 *  Copyright © Elie Gendron 2016-2017
 *
 *  AVRage - My personal AVR library
 */

#include "AVRage.h"


DigitalPin::DigitalPin(uint8_t pin) {
	_get_pin_data(pin);
}

DigitalPin::DigitalPin(uint8_t pin, uint8_t pmode) {
	_get_pin_data(pin);
	set_mode(pmode);
}

DigitalPin::DigitalPin(uint8_t pin, uint8_t pmode, uint8_t state) {
	_get_pin_data(pin);
	set_mode(pmode);
	write(state);
}

void DigitalPin::_get_pin_data(uint8_t pin) {
	if (pin <= 7) {
		_direction = &DDRD;
		_output    = &PORTD;
		_input     = &PIND;
		_mask = (1<<pin);
	}
	else if (pin <= 13) {
		_direction = &DDRB;
		_output    = &PORTB;
		_input     = &PINB;
		_mask = (1<<(pin - 8));
	}
	else if (pin <= 19) {
		_direction = &DDRC;
		_output    = &PORTC;
		_input     = &PINC;
		_mask = (1<<(pin - 14));
	}
	else {
		//error "Not an existing pin..."
	}
}

void DigitalPin::set_mode(uint8_t pmode) {
	switch (pmode) {
		case INPUT_PULLUP:
			write(HIGH);
		case INPUT:
			*_direction &= ~_mask;
			break;
		case OUTPUT:
			*_direction |= _mask;
	}
}

uint8_t DigitalPin::read() {
	return *_input & _mask ? HIGH : LOW;
}

void DigitalPin::write(uint8_t value) {
	if (value) *_output |= _mask;
	else       *_output &= ~_mask;
}

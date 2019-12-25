/*
 *  Button.cpp - frame-based button management
 *  Copyright © Elie Gendron 2016-2017
 *
 *  AVRage - My personal AVR library
 */

#include "Button.h"


Button::Button(volatile uint8_t *ddr, volatile uint8_t *port, volatile uint8_t *pin, uint8_t n, void (*action)()) {
	_ddr = ddr;
	_port = port;
	_pin = pin;
	_mask = (1<<n);
	_action = action;
	_frameCount = 0;
	_waitFrames = 60;
	_repeatFrames = 2;
	_pressed = 1;
	_prevState = 1;
	
	// Set pin as input-pullup
	*_ddr  &= ~_mask;
	*_port |= _mask;
}

void Button::set_action(void (*action)()) {
	_action = action;
}

void Button::set_wait_frames(uint8_t value) {
	_waitFrames = value;
}

void Button::set_repeat_frames(uint8_t value) {
	_repeatFrames = value;
}

void Button::update() {
	_prevState = _pressed;
	_pressed = !(*_pin & _mask);
	
	if (_pressed) {
		if (!_prevState) {
			_action();
		}
		else {
			if (_frameCount > _waitFrames && !(_frameCount % _repeatFrames)) {
				_action();
			}
			_frameCount++;
		}
	}
	else {
		_frameCount = 0;
	}
}
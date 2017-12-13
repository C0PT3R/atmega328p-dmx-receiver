/*
 *  Button.cpp - frame-based button management
 *  Copyright © Elie Gendron 2016-2017
 *
 *  AVRage - My personal AVR library
 */

#include "AVRage.h"

Button::Button(uint8_t pin, void (*action)()) : _pin(pin, INPUT_PULLUP) {
	_action = action;
	_frameCount = 0;
	_waitFrames = 60;
	_repeatFrames = 2;
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
	_pressed = !_pin.read();
	
	if (_pressed) {
		if (!_prevState) {
			_action();
		}
		else {
			if (_frameCount > _waitFrames && _frameCount % _repeatFrames == 0) {
				_action();
			}
			_frameCount++;
		}
	}
	else {
		_frameCount = 0;
	}
}
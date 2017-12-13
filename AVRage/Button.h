/*
 *  Button.h - frame-based button management
 *  Copyright © Elie Gendron 2016-2017
 *
 *  AVRage - My personal AVR library
 */

#ifndef Button_h
#define Button_h


#include "AVRage.h"

class Button {
	public:
		Button(uint8_t pin, void (*action)());

		void set_action(void (*action)());
		void set_wait_frames(uint8_t value);
		void set_repeat_frames(uint8_t value);
		void update();
		
	private:
		DigitalPin _pin;
		bool       _pressed;
		bool       _prevState;
		uint8_t    _waitFrames;
		uint8_t    _repeatFrames;
		uint16_t   _frameCount;

		void (*_action)();
};


#endif
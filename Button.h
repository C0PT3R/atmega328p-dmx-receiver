/*
 *  Button.h - frame-based button management
 *  Copyright © Elie Gendron 2016-2017
 *
 *  AVRage - My personal AVR library
 */

#ifndef Button_h
#define Button_h

#include <avr/io.h>


class Button {
	public:
		Button(volatile uint8_t *ddr, volatile uint8_t *port, volatile uint8_t *pin, uint8_t n, void (*action)());

		void set_action(void (*action)());
		void set_wait_frames(uint8_t value);
		void set_repeat_frames(uint8_t value);
		void update();
		
	private:
		volatile uint8_t *_ddr;
		volatile uint8_t *_port;
		volatile uint8_t *_pin;
		uint8_t          _mask;
		uint8_t          _pressed;
		uint8_t          _prevState;
		uint8_t          _waitFrames;
		uint8_t          _repeatFrames;
		uint16_t         _frameCount;

		void (*_action)();
};


#endif
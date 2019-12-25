#ifndef DMX_RECEIVER_H
#define DMX_RECEIVER_H


#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>

extern "C" { void USART_RX_vect(void) __attribute__((__always_inline__)); };

class DMXReceiver {
	friend void USART_RX_vect(void);
	
	public:
		DMXReceiver();
		
		void init            (uint8_t *buffer, uint8_t numChannels);
		void start           ();
		void stop            ();
		void set_rx_callback (void (*callback)()) { _isrCallback = callback; }
		void set_channel     (uint16_t ch)        { _channel = ch; }
		
	private:
		enum {IDLE, BREAK, START, RECORDING};

		uint8_t  *_buffer;
		uint8_t  *_rxPnt;
		uint8_t   _dmxState;
		uint16_t  _rxCount;
		uint16_t  _numChannels;
		uint16_t  _channel;
		
		inline void _process_ISR(); // Can it be private??? To be tested
		void (*_isrCallback)();
};

extern DMXReceiver dmxReceiver;


#endif
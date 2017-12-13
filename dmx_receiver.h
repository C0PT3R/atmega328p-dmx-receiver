#ifndef DMX_RECEIVER_H
#define DMX_RECEIVER_H


#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>

#define DMX512  ((F_CPU / ( 250000 * 8)) - 1)  // DMX-512  (250 kbaud  - 512 channels) Standard USITT DMX-512
#define DMX1024 ((F_CPU / ( 500000 * 8)) - 1)  // DMX-1024 (500 kbaud  - 1024 channels)
#define DMX2048 ((F_CPU / (1000000 * 8)) - 1)  // DMX-2048 (1000 kbaud - 2048 channels)

extern "C" { void USART_RX_vect(void) __attribute__((__always_inline__)); };

class DMXReceiver {
	friend void USART_RX_vect(void);
	
	public:
		DMXReceiver();
		
		void        init         (uint8_t *buffer, uint8_t dmxMode, uint8_t numChannels);
		void        start        ();
		void        stop         ();
		void        set_callback (void (*callback)()) { _isrCallback = callback; }
		void        set_channel  (uint16_t chan)      { _channel = chan; }
		inline void process_ISR  ();
		
	private:
		enum {IDLE, BREAK, START, RECORDING};

		uint8_t  *_buffer;
		uint8_t  *_rxPnt;
		uint8_t   _dmxState;
		uint16_t  _rxCount;
		uint16_t  _numChannels;
		uint16_t  _channel;
		
		void (*_isrCallback)();
};

extern DMXReceiver dmxReceiver;


#endif
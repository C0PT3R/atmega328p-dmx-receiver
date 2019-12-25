#include "dmx_receiver.h"

DMXReceiver dmxReceiver;

DMXReceiver::DMXReceiver() {
	_buffer = NULL;
	_channel = 1;
	_numChannels = 8;
}

void DMXReceiver::init(uint8_t *buff, uint8_t numChannels) {
	stop();
	_buffer = buff;
	_numChannels = numChannels;
	
	cli();
	UBRR0H  = 0;
	UBRR0L  = (F_CPU / ( 250000 * 8)) - 1;
	UCSR0A |= (1<<U2X0);
	UCSR0C |= (3<<UCSZ00) | (1<<USBS0);
	sei();
	
	_dmxState = IDLE;
}

void DMXReceiver::start() {
	UCSR0B |= (1<<RXEN0) | (1<<RXCIE0);
}

void DMXReceiver::stop() {
	UCSR0B &= ~((1<<RXCIE0) | (1<<RXEN0));
}

ISR(USART_RX_vect) {
	dmxReceiver._process_ISR();
}

void DMXReceiver::_process_ISR() {
	uint8_t uState = UCSR0A;
	uint8_t rxByte = UDR0;

	if (uState & (1<<FE0)) {
		UCSR0A &= ~(1<<FE0);
		_rxCount = _channel;
		_dmxState = BREAK;
	}

	else if (_dmxState == BREAK) {
		if (rxByte == 0) {  //normal start code detected
			_dmxState = START;
			_rxPnt = _buffer + 1;
		}
		else _dmxState = IDLE;
	}

	else if (_dmxState == START) {
		if (--_rxCount == 0)	{  //start address reached?
			_dmxState = RECORDING;
			_buffer[0] = rxByte;
		}
	}

	else if (_dmxState == RECORDING) {
		*_rxPnt = rxByte;
		if (++_rxPnt >= _buffer + _numChannels) {  //all ch received?
			_dmxState = IDLE;
			if (_isrCallback) _isrCallback();
		}
	}
}
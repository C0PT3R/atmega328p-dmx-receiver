#include <avr/io.h>
#include <avr/interrupt.h>
#include "pwm_shifter.h"


#define add_one_pin_to_byte(sendbyte, _counter, outputPnt) { \
	uint16_t pwmval = *outputPnt; \
	asm volatile ("cp %0, %1" : /* No outputs */ : "r" (_counter), "r" (pwmval): ); \
	asm volatile ("ror %0" : "+r" (sendbyte) : "r" (sendbyte) : ); \
}

PWMShifter pwmShifter;

PWMShifter::PWMShifter() {
	_frequency = 0;
	_steps = 0;
	_numRegisters = 0;
	_numOutputs = 0;
	_counter = 0;
}

void PWMShifter::init(uint8_t *buff, uint8_t pwmFrequency, uint16_t pwmSteps, uint8_t registers) {
	_buffer       = buff;
	_frequency    = pwmFrequency;
	_steps        = pwmSteps - 1;
	_numRegisters = registers;
	_numOutputs   = registers * 8;

	cli();
	
	/* ----- Configure SPI ----- */
	//    SPI Enable   LSBFIRST    Master mode
	SPCR |= (1<<SPE) | (1<<DORD) | (1<<MSTR);

	// Set clock divider (SPI_CLOCK_DIV4)
	SPCR &= ~((1<<SPR0) | (1<<SPR1));
	SPSR &= ~(1<<SPI2X); // Not 2x
	
	// Set MOSI & SCK as outputs
	DDRB |= (1<<5) | (1<<3);
	
	/* ----- Configure Timer1 ----- */
	// CTC mode: clear timer on compare match
	TCCR1A &= ~((1<<WGM10) | (1<<WGM11));
	TCCR1B &= ~(1<<WGM13);
	TCCR1B |= (1<<WGM12);
	
	// Select clock source: internal I/O clock, no prescaler
	TCCR1B |= (1<<CS10);
	TCCR1B &= ~((1<<CS11) | (1<<CS12));

	// Set timer limit
	OCR1A = F_CPU / (pwmFrequency * pwmSteps) - 1;
	
	sei();
}


void PWMShifter::set_latch(volatile uint8_t *ddr, volatile uint8_t *port, uint8_t bit) {
	_latchDDR  = ddr;
	_latchPORT = port;
	_latchMask = (1<<bit);
	
	// Set pin as output
	*ddr |= _latchMask;
}


void PWMShifter::start() {
	TIMSK1 |= (1<<OCIE1A); // Enable timer interrupt
}


void PWMShifter::stop() {
	TIMSK1 &= ~(1<<OCIE1A); // Disable timer interrupt
}


void PWMShifter::set_pin(uint8_t pin, uint8_t value) {
	if (pin < _numOutputs) {
		_buffer[pin] = value;
	}
}


void PWMShifter::set_all(uint8_t value) {
	for (uint8_t pin = 0; pin < _numOutputs; pin++) {
		_buffer[pin] = value;
	}
}


ISR(TIMER1_COMPA_vect) {
	pwmShifter._process_ISR();
}


void PWMShifter::_process_ISR() {
	sei();  //enable interrupt nesting to prevent disturbing other interrupt routines

	uint8_t *outputPnt = &_buffer[_numOutputs];
	uint16_t count = _counter;

	// Disable shift registers outputs
	*_latchPORT &= ~_latchMask;
	
	SPDR = 0;  // If removed, program will hang infinitely in loop

	for (uint8_t i = _numRegisters; i > 0; i--) {
		uint8_t sendbyte;

		add_one_pin_to_byte(sendbyte, count, --outputPnt);
		add_one_pin_to_byte(sendbyte, count, --outputPnt);
		add_one_pin_to_byte(sendbyte, count, --outputPnt);
		add_one_pin_to_byte(sendbyte, count, --outputPnt);
		add_one_pin_to_byte(sendbyte, count, --outputPnt);
		add_one_pin_to_byte(sendbyte, count, --outputPnt);
		add_one_pin_to_byte(sendbyte, count, --outputPnt);
		add_one_pin_to_byte(sendbyte, count, --outputPnt);

		while (!(SPSR & (1<<SPIF)));

		SPDR = sendbyte;
	}
	
	while (!(SPSR & (1<<SPIF)));

	// Re-enable (update) shift registers outputs
	*_latchPORT |= _latchMask;

	if (_counter < _steps)
		_counter++;
	else
		_counter = 0;
}

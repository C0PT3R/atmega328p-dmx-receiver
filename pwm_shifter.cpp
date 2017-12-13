#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include "pwm_shifter.h"


#define add_one_pin_to_byte(sendbyte, counter, outputPnt) \
{ \
	uint16_t pwmval = *outputPnt; \
	asm volatile ("cp %0, %1" : /* No outputs */ : "r" (counter), "r" (pwmval): ); \
	asm volatile ("ror %0" : "+r" (sendbyte) : "r" (sendbyte) : ); 	\
}

PWMShifter pwmShifter(0);

PWMShifter::PWMShifter(uint8_t shit) {
	frequency = 0;
	steps = 0;
	numRegisters = 0;
	numOutputs = 0;
	counter = 0;
}

void PWMShifter::init_SPI() {
	//    SPI Enable   LSBFIRST    Master mode
	SPCR |= (1<<SPE) | (1<<DORD) | (1<<MSTR);

	// Set clock divider (SPI_CLOCK_DIV4)
	SPCR &= ~((1<<SPR0) | (1<<SPR1));
	SPSR &= ~(1<<SPI2X); // Not 2x
	
	// Set MOSI & SCK as outputs
	DDRB |= (1<<5) | (1<<3);
}

void PWMShifter::init_timer1(uint8_t pwmFrequency, uint16_t pwmSteps) {
	// Configure timer1 in CTC mode: clear the timer on compare match
	TCCR1A &= ~((1<<WGM10) | (1<<WGM11));
	TCCR1B &= ~(1<<WGM13);
	TCCR1B |= (1<<WGM12);
	
	// Select clock source: internal I/O clock, without a prescaler
	TCCR1B |= (1<<CS10);
	TCCR1B &= ~((1<<CS11) | (1<<CS12));

	// Set timer limit
	OCR1A = (uint16_t)((float)F_CPU / ((float)pwmFrequency * (float)pwmSteps)) - 1;
}

void PWMShifter::init(uint8_t *buff, uint8_t pwmFrequency, uint16_t pwmSteps, uint8_t registers) {
	buffer       = buff;
	frequency    = pwmFrequency;
	steps        = pwmSteps;
	numRegisters = registers;
	numOutputs   = numRegisters * 8;

	cli();
	init_SPI();
	init_timer1(pwmFrequency, pwmSteps);
	sei();
}

void PWMShifter::set_latch(volatile uint8_t *ddr, volatile uint8_t *port, uint8_t bit) {
	*ddr |= (1<<bit);

	latchDDR  = ddr;
	latchPORT = port;
	latchBit  = bit;
}

void PWMShifter::start() {
	TIMSK1 |= (1<<OCIE1A); // Enable timer interrupt
}

void PWMShifter::stop() {
	TIMSK1 &= ~(1<<OCIE1A); // Disable timer interrupt
}

void PWMShifter::set_pin(uint8_t pin, uint8_t value) {
	if (pin < numOutputs) {
		buffer[pin] = value;
	}
}

void PWMShifter::set_all(uint8_t value) {
	for (uint8_t pin = 0; pin < numOutputs; pin++) {
		buffer[pin] = value;
	}
}

ISR(TIMER1_COMPA_vect) {
	pwmShifter.process_ISR();
}

void PWMShifter::process_ISR() {
	sei(); //enable interrupt nesting to prevent disturbing other interrupt functions

	uint8_t *outputPnt = &buffer[numOutputs];
	uint16_t count = counter;

	*latchPORT &= ~(1<<latchBit);
	
	SPDR = 0; // If removed, program will hang infinitely in loop

	for (uint8_t i = numRegisters; i > 0; i--) {
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

	*latchPORT |= (1<<latchBit);

	if (counter < steps - 1) counter++;
	else                     counter = 0;
}
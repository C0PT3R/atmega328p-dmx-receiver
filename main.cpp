/* EEPROM address where to save setting */
#define EEPROM_SETTING_ADDR 0x20

/* How many frames (~5ms) to wait before saving to eeprom and deactivating display */
#define DISPLAY_TIMEOUT 1000       // 1000 = ~5s

/* Power/activity LED blink cycle length */
#define ACTIVITY_LED_CYCLE_LENGTH 12

/* Amount of daisy-chained 8-bit shift registers */
#define NUM_SHIFT_REGISTERS 3

/* PWM frequency */
#define PWM_FREQUENCY 90

/* PWM steps */
#define PWM_STEPS 256


#define FRAME_TIMEOUT     F_CPU / 10667
#define DIGIT_TIMEOUT     FRAME_TIMEOUT / 3
#define DMX_NUM_CHANNELS  8 * NUM_SHIFT_REGISTERS
#define DMX_MAX_CHANNEL   512 - DMX_NUM_CHANNELS + 1


#include <avr/eeprom.h>
#include <util/delay.h>
#include "Button.h"
#include "dmx_receiver.h"
#include "pwm_shifter.h"


// Function prototypes
void display_digits     ();
void increase_channel   ();
void decrease_channel   ();
void dmx_frame_received ();


uint16_t dmxChannel;
uint8_t  displayActive    = 1;
uint8_t  activityLedPhase = 0;


int main(void) {
	// Display anodes (all outputs except RX, all low)
	DDRD  = 0b11111110;
	PORTD = 0b00000000;
	
	// Display cathodes (outputs, high)
	DDRB  |= ((1<<PINB0) | (1<<PINB1) | (1<<PINB2));
	PORTB |= ((1<<PINB0) | (1<<PINB1) | (1<<PINB2));

	// Power/activity LED pin (output, high)
	DDRC  |= (1<<PINC5);
	PORTC |= (1<<PINC5);
	
	uint16_t prevChannel;
	uint16_t frameCount = 0;
	uint8_t dmxValues[DMX_NUM_CHANNELS];
	
	Button   channelUpButton(&DDRC, &PORTC, &PINC, PINC0, increase_channel);
	Button channelDownButton(&DDRC, &PORTC, &PINC, PINC1, decrease_channel);

	// Load setting from EEPROM
	uint16_t ch = eeprom_read_word((uint16_t*)EEPROM_SETTING_ADDR);
	dmxChannel = (ch && ch <= DMX_MAX_CHANNEL) ? ch : 1;
	
	// Start PWM shifter
	pwmShifter.init(&dmxValues[0], PWM_FREQUENCY, PWM_STEPS, NUM_SHIFT_REGISTERS);
	pwmShifter.set_latch(&DDRC, &PORTC, PINC2);
	pwmShifter.start();
	
	// Start DMX receiver
	dmxReceiver.init(&dmxValues[0], DMX_NUM_CHANNELS);
	dmxReceiver.set_rx_callback(dmx_frame_received);
	dmxReceiver.set_channel(dmxChannel);
	dmxReceiver.start();

	while (true) {
		// prevChannel must be set before updating buttons
		prevChannel = dmxChannel;

		channelUpButton.update();
		channelDownButton.update();
		
		if (displayActive) {
			if (frameCount < DISPLAY_TIMEOUT) {
				if (dmxChannel != prevChannel) {
					if      (dmxChannel > DMX_MAX_CHANNEL) dmxChannel = 1;
					else if (dmxChannel < 1)               dmxChannel = DMX_MAX_CHANNEL;
					dmxReceiver.set_channel(dmxChannel);
					frameCount = 0;
				}

				else  {
					frameCount++;
				}
				
				display_digits();
			}
			
			else {
				// Save setting to EEPROM
				eeprom_update_word((uint16_t*)EEPROM_SETTING_ADDR, dmxChannel);
				
				displayActive = 0;
				frameCount = 0;
			}
		}
		else {
			_delay_us(FRAME_TIMEOUT);
		}
		
		if (activityLedPhase > 0) {
			if (activityLedPhase < ACTIVITY_LED_CYCLE_LENGTH / 2) {
				PORTC &= ~(1<<PINC5);
			}
			else {
				PORTC |= (1<<PINC5);
			}
			
			activityLedPhase--;
		}
	}

	return 1;
}


void display_digits() {
	static uint8_t digit[] = {
		0b01111110, 0b00001100, 0b10110110, 0b10011110, 0b11001100,
		0b11011010, 0b11111010, 0b00001110, 0b11111110, 0b11011110
	};

	// Display units
	PORTD = digit[dmxChannel % 10];
	PORTB &= ~(1<<PINB2);
	_delay_us(DIGIT_TIMEOUT);
	PORTB |= (1<<PINB2);

	// Display tens
	PORTD = digit[dmxChannel / 10 % 10];
	PORTB &= ~(1<<PINB1);
	_delay_us(DIGIT_TIMEOUT);
	PORTB |= (1<<PINB1);

	// Display hundreds
	PORTD = digit[dmxChannel / 100];
	PORTB &= ~(1<<PINB0);
	_delay_us(DIGIT_TIMEOUT);
	PORTB |= (1<<PINB0);
}


void increase_channel() {
	if (displayActive) dmxChannel++;
	else displayActive = 1;
}


void decrease_channel() {
	if (displayActive) dmxChannel--;
	else displayActive = 1;
}


void dmx_frame_received() {
	if (!activityLedPhase) activityLedPhase = ACTIVITY_LED_CYCLE_LENGTH;
}
/* MCU clock frequency (Hz) */
#define F_CPU 16000000UL

/* EEPROM address where to save setting */
#define EEPROM_SETTING_ADDR 0x20

/* How many "frames" to wait before saving to eeprom and deactivating display */
#define DISPLAY_TIMEOUT 1000       // 1000 x ~5ms = ~~5s

/* Power/Activity LED blink cycle length */
#define ACTIVITY_LED_CYCLE_LENGTH 12

/* Amount of daisy-chained 8-bit shift registers */
#define NUM_SHIFT_REGISTERS 3

/* PWM frequency */
#define PWM_FREQUENCY 90

/* PWM steps */
#define PWM_STEPS 256

/* PWM shifter latch pin */
#define SHIFTER_LATCH_PORT &PORTC
#define SHIFTER_LATCH_DDR  &DDRC
#define SHIFTER_LATCH_BIT  2


#define DMX_NUM_CHANNELS 8 * NUM_SHIFT_REGISTERS
#define DMX_MAX_CHANNEL  512 - DMX_NUM_CHANNELS + 1


#include <avr/eeprom.h>
#include <util/delay.h>
#include "AVRage/AVRage.h"
#include "dmx_receiver.h"
#include "pwm_shifter.h"


// Function prototypes
void     display_digit      (DigitalPin *cathode, uint8_t n);
void     increase_channel   ();
void     decrease_channel   ();
void     dmx_frame_received ();
uint16_t load_setting       ();
void     save_setting       (uint16_t channel);


uint16_t dmxChannel       = 1;
uint8_t  displayActive    = 1;
uint8_t  activityLedPhase = 0;

const uint8_t digit[] = {
	0b01111110, 0b00001100, 0b10110110, 0b10011110, 0b11001100,
	0b11011010, 0b11111010, 0b00001110, 0b11111110, 0b11011110
};


int main(void) {
	uint16_t prevChannel;
	uint16_t frameCount = 0;

	uint8_t dmxValues[DMX_NUM_CHANNELS];

	// Port D is all outputs, except RX
	DDRD = 0b11111110;

	DigitalPin ledPin(PIN_C5, OUTPUT, HIGH);

	DigitalPin displayCathode[] = {
		DigitalPin(PIN_B0, OUTPUT, HIGH),
		DigitalPin(PIN_B1, OUTPUT, HIGH),
		DigitalPin(PIN_B2, OUTPUT, HIGH)
	};
	
	Button   channelUpButton(PIN_C0, increase_channel);
	Button channelDownButton(PIN_C1, decrease_channel);

	// Load saved setting from eeprom
	dmxChannel = load_setting();
	
	// Start PWM shifter
	pwmShifter.init(&dmxValues[0], PWM_FREQUENCY, PWM_STEPS, NUM_SHIFT_REGISTERS);
	pwmShifter.set_latch(SHIFTER_LATCH_DDR, SHIFTER_LATCH_PORT, SHIFTER_LATCH_BIT);
	pwmShifter.start();
	
	// Start DMX receiver
	dmxReceiver.init(&dmxValues[0], DMX512, DMX_NUM_CHANNELS);
	dmxReceiver.set_callback(dmx_frame_received);
	dmxReceiver.set_channel(dmxChannel);
	dmxReceiver.start();

	for (;;) {
		prevChannel = dmxChannel;

		channelUpButton.update();
		channelDownButton.update();
		
		if (displayActive) {
			if (dmxChannel != prevChannel) {
				if      (dmxChannel > DMX_MAX_CHANNEL) dmxChannel = 1;
				else if (dmxChannel < 1)               dmxChannel = DMX_MAX_CHANNEL;
				dmxReceiver.set_channel(dmxChannel);
				frameCount = 0;
			}

			else if (frameCount > DISPLAY_TIMEOUT) {
				save_setting(dmxChannel);
				displayActive = 0;
				frameCount = 0;
			}
			
			else {
				display_digit(&displayCathode[0], dmxChannel / 100);
				display_digit(&displayCathode[1], dmxChannel / 10 % 10);
				display_digit(&displayCathode[2], dmxChannel % 10);
			
				frameCount++;
			}
		}
		else {
			_delay_us(1500);
		}
		
		if (activityLedPhase > 0) {
			ledPin.write(activityLedPhase < ACTIVITY_LED_CYCLE_LENGTH / 2);
			activityLedPhase--;
		}
	}

	return 0;
}

void display_digit(DigitalPin *cathode, uint8_t n) {
	PORTD = digit[n];
	
	(*cathode).write(LOW);
	_delay_us(500);
	(*cathode).write(HIGH);
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
	if (activityLedPhase == 0) activityLedPhase = ACTIVITY_LED_CYCLE_LENGTH;
}

uint16_t load_setting() {
	uint16_t channel = eeprom_read_word((uint16_t*)EEPROM_SETTING_ADDR);
	return (channel != 0 && channel <= DMX_MAX_CHANNEL) ? channel : 1;
}

void save_setting(uint16_t channel) {
	eeprom_update_word((uint16_t*)EEPROM_SETTING_ADDR, channel);
}
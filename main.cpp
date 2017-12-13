/* MCU clock frequency (Hz) */
#define F_CPU 16000000UL

/* EEPROM address where to save setting */
#define EEPROM_SETTING_ADDR 0x20

/* How many "frames" to wait before saving to eeprom and deactivating display */
#define DISPLAY_TIMEOUT 1000       // 1000 x 5ms = ~5s

/* Power/Activity LED blink cycle length */
#define ACTIVITY_LED_CYCLE_LENGTH 12

/* Amount of 8-bit shift registers */
#define NUM_SHIFT_REGISTERS 3

/* PWM frequency */
#define PWM_FREQUENCY 90

/* PWM steps */
#define PWM_STEPS 256

/* PWM shifter latch pin */
#define SHIFTER_LATCH_PORT PORTC
#define SHIFTER_LATCH_DDR  DDRC
#define SHIFTER_LATCH_BIT  2


#include <avr/eeprom.h>
#include <util/delay.h>
#include "AVRage/AVRage.h"
#include "dmx_receiver.h"
#include "pwm_shifter.h"

// Function prototypes
uint8_t  *create_buffer      (uint8_t bSize);
void      display_digit      (uint8_t pos, uint8_t n);
uint16_t  load_setting       (uint16_t max);
void      save_setting       (uint16_t channel);
void      dmx_frame_received ();
void      increase_channel   ();
void      decrease_channel   ();

uint16_t dmxChannel       = 1;
uint8_t  displayActive    = 1;
uint8_t  activityLedPhase = 0;

const uint8_t digit[10] = {
	0b01111110, 0b00001100, 0b10110110, 0b10011110, 0b11001100,
	0b11011010, 0b11111010, 0b00001110, 0b11111110, 0b11011110
};

DigitalPin ledPin(PIN_C5, OUTPUT, HIGH);
DigitalPin  comm0(PIN_B0, OUTPUT, HIGH);
DigitalPin  comm1(PIN_B1, OUTPUT, HIGH);
DigitalPin  comm2(PIN_B2, OUTPUT, HIGH);

DigitalPin *displayCathode[3] = { &comm0, &comm1, &comm2 };

Button   channelUpButton(PIN_C0, increase_channel);
Button channelDownButton(PIN_C1, decrease_channel);

int main(void) {
	uint16_t prevChannel;
	uint16_t frameCount     = 0;
	uint16_t dmxNumChannels = NUM_SHIFT_REGISTERS * 8;
	uint16_t dmxMaxChannel  = 512 - dmxNumChannels + 1;

	// Create and zero-init RAM buffer
	uint8_t *ramBuffer = create_buffer(dmxNumChannels);

	// Port D is all outputs, except RX
	DDRD = 0b11111110;

	// Load saved setting from eeprom
	dmxChannel = load_setting(dmxMaxChannel);
	
	// Start PWM shifter
	pwmShifter.init(ramBuffer, PWM_FREQUENCY, PWM_STEPS, NUM_SHIFT_REGISTERS);
	pwmShifter.set_latch(&DDRC, &PORTC, 2);
	pwmShifter.start();
	
	// Start DMX receiver
	dmxReceiver.init(ramBuffer, DMX512, dmxNumChannels);
	dmxReceiver.set_callback(dmx_frame_received);
	dmxReceiver.set_channel(dmxChannel);
	dmxReceiver.start();

	for (;;) {
		prevChannel = dmxChannel;

		channelUpButton.update();
		channelDownButton.update();
		
		if (displayActive) {
			if (dmxChannel != prevChannel) {
				if      (dmxChannel > dmxMaxChannel) dmxChannel = 1;
				else if (dmxChannel < 1)             dmxChannel = dmxMaxChannel;
				dmxReceiver.set_channel(dmxChannel);
				frameCount = 0;
			}

			else if (frameCount > DISPLAY_TIMEOUT) {
				save_setting(dmxChannel);
				displayActive = 0;
				frameCount = 0;
			}
			
			display_digit(0, dmxChannel / 100);
			display_digit(1, dmxChannel / 10 % 10);
			display_digit(2, dmxChannel % 10);
			
			frameCount++;
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

uint8_t *create_buffer(uint8_t bSize) {
	uint8_t *buffer = (uint8_t*)malloc(bSize);
	memset((uint8_t*)buffer, 0, bSize);
	return buffer;
}

void display_digit(uint8_t pos, uint8_t n) {
	PORTD = digit[n];
	
	(*displayCathode[pos]).write(0);
	_delay_us(500);
	(*displayCathode[pos]).write(1);
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

uint16_t load_setting(uint16_t max) {
	uint16_t channel = eeprom_read_word((uint16_t*)EEPROM_SETTING_ADDR);
	return (channel != 0 && channel <= max) ? channel : 1;
}

void save_setting(uint16_t channel) {
	eeprom_update_word((uint16_t*)EEPROM_SETTING_ADDR, channel);
}
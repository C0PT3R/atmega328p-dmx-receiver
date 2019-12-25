# Atmega328P DMX receiver with PWM outputs

- Uses native USART module to decode DMX512 frames.
- Uses native SPI module to produce PWM signals through chained serial-in/parallel-out 8-bit shift registers.
- Uses a three digits 7-segment display and two buttons to select DMX channel.
- Display automatically shuts down after a few seconds of inactivity.
- Channel is saved to internal EEPROM.
- Doesn't use the Arduino library, nor its bootloader.

Demo: https://www.youtube.com/watch?v=NsYhHazGQkg

# VERY IMPORTANT NOTICE!!!
I've highly modified this code recently, but didn't test it at all. I'm currently in the process of re-building the circuit to be able to test it. Use at your own risk.

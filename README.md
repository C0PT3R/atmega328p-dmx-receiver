# Atmega328P DMX receiver with PWM outputs

- Uses native USART module to decode DMX512 frames.
- Uses native SPI module to produce PWM signals through chained serial-in/parallel-out 8-bit shift registers.
- Uses a three digits 7-segment display and two buttons to select DMX channel.
- Doesn't use the Arduino library, nor its bootloader.

Demo: https://www.youtube.com/watch?v=NsYhHazGQkg

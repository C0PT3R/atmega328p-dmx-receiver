#ifndef PWMShifter_h
#define PWMShifter_h

extern "C" { void TIMER1_COMPA_vect(void) __attribute__((__always_inline__)); };

class PWMShifter {
	friend void TIMER1_COMPA_vect(void);

	public:
		PWMShifter();
		
		void init      (uint8_t *buff, uint8_t pwmFrequency, uint16_t pwmSteps, uint8_t registers);
		void start     ();
		void stop      ();
		void set_latch (volatile uint8_t *ddr, volatile uint8_t *port, uint8_t bit);
		void set_pin   (uint8_t pin, uint8_t value);
		void set_all   (uint8_t value);

	private:
		uint8_t          *_buffer;
		uint8_t           _frequency;
		uint16_t          _steps;
		uint8_t           _numRegisters;
		uint8_t           _numOutputs;
		uint16_t          _counter;
		volatile uint8_t *_latchDDR;
		volatile uint8_t *_latchPORT;
		uint8_t           _latchMask;
		
		inline void _process_ISR();
};

extern PWMShifter pwmShifter;

#endif

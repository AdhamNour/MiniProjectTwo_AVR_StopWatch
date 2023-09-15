/*
 * MiniProject2.c
 *
 *  Created on: Sep 13, 2023
 *      Author: adham
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

unsigned char currentDigit = 0;

struct Time {
	unsigned char secondCounter;
	unsigned char minuteCounter;
	unsigned char hourCounter;

} t;
unsigned char initSeconds = 0;
unsigned char initMinutes = 0;
unsigned char initHours = 0;

void increment(unsigned char *x) {
	if (((*x) & 0x0F) == 9) {
		(*x) = ((*x) & 0xF0);

		if ((((*x) & 0xF0) >> 4) == 9) {
			(*x) = ((*x) & 0x0F);
		} else {
			(*x) = ((*x) & 0x0F) | ((((((*x) & 0xF0) >> 4) + 1) << 4) & 0xF0);

		}
	} else {
		(*x) = ((*x) & 0xF0) | (((*x) + 1) & 0x0F);

	}
}


void decrement(unsigned char *x){
	if (((*x) & 0x0F) == 0) {
			(*x) = ((*x) & 0xF0)|9;

			if ((((*x) & 0xF0) >> 4) == 0) {
				(*x) = ((*x) & 0x0F);
			} else {
				(*x) = ((*x) & 0x0F) | ((((((*x) & 0xF0) >> 4) - 1) << 4) & 0xF0);

			}
		} else {
			(*x) = ((*x) & 0xF0) | (((*x) - 1) & 0x0F);

		}
}

void incrementTime() {
	if (t.secondCounter == 0x59) {
		t.secondCounter = 0;
		if (t.minuteCounter == 0x59) {
			t.minuteCounter = 0;
			increment(&(t.hourCounter));

		} else {
			increment(&(t.minuteCounter));
		}
	} else {
		increment(&(t.secondCounter));
	}

}

void decrementTime() {
	if (t.secondCounter == 0x0) {
		t.secondCounter = 0x59;
		if (t.minuteCounter == 0x00) {
			t.minuteCounter = 0x59;
			decrement(&(t.hourCounter));

		} else {
			decrement(&(t.minuteCounter));
		}
	} else {
		decrement(&(t.secondCounter));
	}

}

ISR(TIMER2_COMP_vect) {
	PORTA = (PORTA & 0xC0) | (1 << currentDigit);
	switch (currentDigit) {
	case 0:
		PORTC = (PORTC & 0xF0) | (t.secondCounter & 0x0F);
		break;
	case 1:
		PORTC = (PORTC & 0xF0) | ((t.secondCounter & 0xF0) >> 4);
		break;
	case 2:
		PORTC = (PORTC & 0xF0) | (t.minuteCounter & 0x0F);
		break;
	case 3:
		PORTC = (PORTC & 0xF0) | ((t.minuteCounter & 0xF0) >> 4);
		break;
	case 4:
		PORTC = (PORTC & 0xF0) | (t.hourCounter & 0x0F);
		break;
	case 5:
		PORTC = (PORTC & 0xF0) | ((t.hourCounter & 0xF0) >> 4);
		break;
	default:
		PORTC = (PORTC & 0xF0);
		break;
	}
	currentDigit++;
	if (currentDigit > 5)
		currentDigit = 0;

}

ISR(TIMER1_COMPA_vect) {
	if(PORTD & (1 << PD1)){
		decrementTime();

	}
	else{
		incrementTime();

	}
}
ISR(INT0_vect) {
//	TCCR1B &=~(1<<CS11|1<<CS10);
	t.hourCounter = 0;
	t.minuteCounter = 0;
	t.secondCounter = 0;
}
ISR(INT1_vect) {
	TCCR1B &= ~(1 << CS12 | 1 << CS11 | 1 << CS10);
}
ISR(INT2_vect) {
	TCCR1B |= (1 << CS11) | (1 << CS10);
	PORTB ^= (1 << PB0);
}
int main(void) {
	t.hourCounter = initHours;
	t.minuteCounter = initMinutes;
	t.secondCounter = initSeconds;
	//Init the Output ports
	DDRA |= 0x3F;
	PORTA &= ~(0x3F);
	DDRC |= 0x0F;
	PORTC &= ~(0x0F);
	DDRB |= (1 << PB0);
	PORTB &= ~(1 << PB0);
	// init the timer2
	TCCR2 = (1 << FOC2) | (1 << WGM21) | (1 << CS21);
	TCNT2 = 0;
	TIMSK |= (1 << OCIE2);
	OCR2 = 250;
	SREG |= 1 << 7;
	// init timer1
	TCNT1 = 0; // initial value of timer register
	OCR1A = 15625;
//	OCR1A = 156;

	TIMSK |= (1 << OCIE1A); //enable compare interrupt
	TCCR1A |= (1 << FOC1A);  //force compare match
	TCCR1B |= (1 << WGM12); //CTC (clear timer on compare mode)(WGM12 =1, WGM10,11,13 =0)
	TCCR1B |= (1 << CS11) | (1 << CS10);

	//Init INT0 (Res)
	DDRD &= ~(1 << PD2);
	PORTD |= (1 << PD2);

	MCUCR |= (1 << ISC01);
	MCUCR &= ~(1 << ISC00);
	GICR |= (1 << INT0);

	//Init INT1 (Stop)
	DDRD &= ~(1 << PD3);

	MCUCR |= (1 << ISC10 | 1 << ISC11);
	GICR |= (1 << INT1);
	//Init INT2 (Stop)
	DDRB &= ~(1 << PB2);
	PORTB |= (1 << PB2);
	MCUCSR &= ~(1 << ISC2);
	GICR |= (1 << INT2);

	DDRD |= (1 << PD0) | (1 << PD1);
	PORTD |= (1 << PD0);
	PORTD &= ~(1 << PD1);
	DDRD &= ~(1 << PD4);

	DDRB &= ~((1 << PB3) | (1 << PB4) | (1 << PB5));

	while (1) {
		if (PIND & (1 << PD4)) {
			_delay_ms(30);
			if (PIND & (1 << PD4)) {
				PORTD ^= (1 << PD0) | (1 << PD1);
				TCCR1B &= ~(1 << CS12 | 1 << CS11 | 1 << CS10);

				while (PIND & (1 << PD4))
					;
			}
		}
		if (PORTD & (1 << PD1)) {

			if (PINB & (1 << PB3)) {
				_delay_ms(30);
				if (PINB & (1 << PB3)) {
					increment(&(t.secondCounter));
					while (PINB & (1 << PB3))
						;
				}
			}
			if (PINB & (1 << PB4)) {
				_delay_ms(30);
				if (PINB & (1 << PB4)) {
					increment(&(t.minuteCounter));
					while (PINB & (1 << PB4))
						;
				}
			}
			if (PINB & (1 << PB5)) {
				_delay_ms(30);
				if (PINB & (1 << PB5)) {
					increment(&(t.hourCounter));
					while (PINB & (1 << PB5))
						;
				}
			}
			if(t.secondCounter==0 && t.minuteCounter==0 && t.hourCounter==0){
				TCCR1B &= ~(1 << CS12 | 1 << CS11 | 1 << CS10);

			}
		}

	}

}

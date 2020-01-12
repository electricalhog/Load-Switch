/*
 * Load_Switch_00.c
 *
 * Created: 12/27/2019 12:33:11 PM
 * Author : Daniel Raymond
 */ 

#include <avr/io.h>
#include <util/delay.h>

#define elif else if

#define outpin 0

#define outset(pin_num) PORTB |=  (1<<pin_num)
#define outclr(pin_num) PORTB &= ~(1<<pin_num)
#define outtgl(pin_num) PORTB ^=  (1<<pin_num)

#define dirset(pin_num) DDRB |= (1<<pin_num)
#define dirclr(pin_num) DDRB &= ~(1<<pin_num)

long first_time;
long second_time;
long tick_width;
long pulse_width;

int main(void)
{	
	// setup tccr for input capture
	TCCR0A  = (0x00);//set all to 0
	TCCR0B |= (1<<6)//set edge capture to rising
			  |(0b010<<0);//set prescaler to 8
	TCCR0C  = (0x00);//set all to 0
	TIMSK0  |= (1<<5);//enable input capture flag
	
	
	dirset(outpin);
	outclr(outpin); // default to off on bootup
	
    while (1){
		//wait for rising edge and capture
		TCCR0B |= (1<<6);//set edge capture to rising
		while (!(TIFR0 & (1<<5)));//wait for rising edge
		TIFR0 = 1<<5;//clear flag
		first_time = ICR0;//record time of event
		
		//wait for falling edge and capture
		TCCR0B &= ~(1<<6);//set edge capture to falling
		while (!(TIFR0 & (1<<5)));//wait for falling edge
		TIFR0 = 1<<5;//clear flag
		second_time = ICR0;//record time of event
		
		//clear
		TCNT0 = 0x0000;//clear counter value to prevent overflow
		
		//convert to us (done via measure and check approx)
		tick_width = second_time-first_time;//calculate the width of the pulse in counter increments
		pulse_width = (tick_width*8333)/1000;//calcuate the width of the pulse in microseconds
		
		//set the value of the mosfet based on the pulse width
		//	there are dead bands between 1.4->1.6ms and 2.15->2.20ms
		//	this is done to reduce jitter and flicker at a given point
		//	only by exiting one of the dead bands will the value change, like schmit trigger!
		//	if the input value is greater than 2.2ms it will shut off 
		if (pulse_width < 1400){//turn off the LED if the pulse is small
			outclr(outpin);
		}
		elif ((pulse_width > 1600) && (pulse_width < 2150)){//turn on the MOSFET if the pulse if large
			outset(outpin);
		} 
		elif (pulse_width >= 2200){
			outclr(outpin);
		}
    }
}


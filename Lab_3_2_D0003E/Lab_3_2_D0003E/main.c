
 
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "tinythreads.h"

int writeChar(char ch, int pos);
void LCD_Init(void);
void writeLong(long i);
void primes();
bool is_prime(long i);
int writeReg(int num, int reg, bool shift);
int main(void);


bool switchLight = false;
long numbPress = 0;
bool toggle = false;

void printAt(long num, int pos) {
	int pp = pos;
	writeChar( (num % 100) / 10 + '0', pp);
	pp++;
	writeChar( num % 10 + '0', pp);
}

void button() {
	unsigned char i;
	while(1) {
		lock(&but);
		printAt(numbPress, 4);
		i = (PINB & 0x80);
		if (i != 0x80) {
			if(toggle) {
				LCDDR2 &= 0x9F;
				LCDDR2 |= 0x06;
			} else {
				LCDDR2 &= 0xF9;
				LCDDR2 |= 0x60;
			}
		}
	}
}

void blink() {
	while(1) {
		lock(&bli);
		if (switchLight) {
			LCDDR0 &= 0xF9;
			LCDDR0 |= 0x6;
		}
		else {
			LCDDR0 &= 0xF9;
		}
	}
}
ISR(TIMER1_COMPA_vect){
	switchLight = !switchLight;
	unlock(&bli);
	//yield();
}

ISR(PCINT1_vect){
	int i = PINB & 0x80;
	if (i != 0x80) {
		numbPress++;
		toggle = !toggle;
	}
	unlock(&but);
	//yield();
}


int main(void)
{
	CLKPR = 0x80;
	CLKPR = 0x00;
    LCD_Init();

	lock(&bli); lock(&but);
	spawn(blink, 1);
	yield();
	spawn(button, 2);
	yield();
	primes();
}

void LCD_Init(void){

	LCDCRB = (1<<LCDMUX1)|(1<<LCDMUX0)|(1<<LCDPM2)|(1<<LCDPM1)|(1<<LCDPM2);
	LCDFRR = (0<<LCDPS2)|(0<<LCDPS1)|(0<<LCDPS0)|(1<<LCDCD2)|(1<<LCDCD1)|(1<<LCDCD0);
	LCDCCR = (0<<LCDDC2)|(0<<LCDDC1)|(0<<LCDDC0)|(1<<LCDCC3)|(1<<LCDCC2)|(1<<LCDCC1)|(1<<LCDCC0);
	LCDCRA = (1<<LCDEN)|(1<<LCDAB)|(0<<LCDIE)|(0<<LCDBL);
	
	TCCR1B = (0<<WGM13)|(1<<WGM12)|(0<<WGM11)|(0<<WGM10)|(1<<CS12)|(0<<CS11)|(1<<CS10);
	
}
void writeLong(long i) {
	int n = 0;
	while (n < 6) {		// a while loop for getting the first 6 digits of a number.
		int temp = i % 10;
		writeChar((char)temp + '0', 5-n);
		i = floor(i / 10);
		n += 1;
	}	
}

void primes() {
	for(long count=0; count < 50000; count+= 1) {
		if (is_prime(count))
			printAt(count, 0);
	}
}


bool is_prime(long i) {
	int n = 2;
	while (i % n != 0 && i > 1){	// uses an inefficient but standard way of looking for primes.
		if (i - 1 == n) return true;
		n++;
	}
	return false;
	
}


int writeChar(char ch, int pos){
	int SCC_X_0 = 0, SCC_X_1 = 0, SCC_X_2 = 0, SCC_X_3 = 0;
		
	switch (ch)					/* a switch statement for which character is to be printed on the display.
								   sets various bytes of SCC_X to certain values, depending on which areas
								   should light up. These values are derived from the documentation.    */
	{
	case '0' :
		SCC_X_0 = 0x1;
		SCC_X_1 = 0x5;
		SCC_X_2 = 0x5;
		SCC_X_3 = 0x1;
		
		break;
	case '1' :
		SCC_X_0 = 0x0;
		SCC_X_1 = 0x1;
		SCC_X_2 = 0x1;
		SCC_X_3 = 0x0;
		break;
	case '2' :
		SCC_X_0 = 0x1;
		SCC_X_1 = 0x1;
		SCC_X_2 = 0xE;
		SCC_X_3 = 0x1;
		break;
	case '3' :
		SCC_X_0 = 0x1;
		SCC_X_1 = 0x1;
		SCC_X_2 = 0xb;
		SCC_X_3 = 0x1;
		break;
	case '4' :
		SCC_X_0 = 0x0;
		SCC_X_1 = 0x5;
		SCC_X_2 = 0xb;
		SCC_X_3 = 0x0;
		break;
	case '5' :
		SCC_X_0 = 0x1;
		SCC_X_1 = 0x4;
		SCC_X_2 = 0xb;
		SCC_X_3 = 0x1;
		break;
	case '6' :
		SCC_X_0 = 0x1;
		SCC_X_1 = 0x4;
		SCC_X_2 = 0xf;
		SCC_X_3 = 0x1;
		break;
	case '7' :
		SCC_X_0 = 0x1;
		SCC_X_1 = 0x1;
		SCC_X_2 = 0x1;
		SCC_X_3 = 0x0;
		break;
	case '8' :
		SCC_X_0 = 0x1;
		SCC_X_1 = 0x5;
		SCC_X_2 = 0xf;
		SCC_X_3 = 0x1;
		break;
	case '9' :
		SCC_X_0 = 0x1;
		SCC_X_1 = 0x5;
		SCC_X_2 = 0xb;
		SCC_X_3 = 0x1;
		break;
	default:
		return 2;
	}
	
	switch(pos){										/* a switch statement for the position of the digit. Depending on the position 
														   we will write to different registers. We use a boolean value to show if 
														   the lower byte of the register or not, since the positions are paired up 
														   per register. */
	case 0:
		LCDDR0 &= 0xF6;
		LCDDR0 |=writeReg(SCC_X_0, LCDDR0, false);
		LCDDR5=writeReg(SCC_X_1, LCDDR5, false);
		LCDDR10=writeReg(SCC_X_2, LCDDR10, false);
		LCDDR15=writeReg(SCC_X_3, LCDDR15, false);		
		break;
	case 1:
		LCDDR0 &= 0x6F;
		LCDDR0 |=writeReg(SCC_X_0, LCDDR0, true);
		LCDDR5=writeReg(SCC_X_1, LCDDR5, true);
		LCDDR10=writeReg(SCC_X_2, LCDDR10, true);
		LCDDR15=writeReg(SCC_X_3, LCDDR15, true);
		break;
	case 2:
		LCDDR1 = writeReg(SCC_X_0, LCDDR1, false);
		LCDDR6 = writeReg(SCC_X_1, LCDDR6, false);
		LCDDR11 = writeReg(SCC_X_2, LCDDR11, false);
		LCDDR16 = writeReg(SCC_X_3, LCDDR16, false);
		break;
	case 3:
		LCDDR1 = writeReg(SCC_X_0, LCDDR1, true);
		LCDDR6 = writeReg(SCC_X_1, LCDDR6, true);
		LCDDR11 = writeReg(SCC_X_2, LCDDR11, true);
		LCDDR16 = writeReg(SCC_X_3, LCDDR16, true);
		break;
	case 4:
		LCDDR2 &= 0xF6;
		LCDDR2 |= writeReg(SCC_X_0, LCDDR2, false);
		LCDDR7 = writeReg(SCC_X_1, LCDDR7, false);
		LCDDR12 = writeReg(SCC_X_2, LCDDR12, false);
		LCDDR17 = writeReg(SCC_X_3, LCDDR17, false);
		break;
	case 5:
		LCDDR2 &= 0x6F;
		LCDDR2 |= writeReg(SCC_X_0, LCDDR2, true);
		LCDDR7 = writeReg(SCC_X_1, LCDDR7, true);
		LCDDR12 = writeReg(SCC_X_2, LCDDR12, true);
		LCDDR17 = writeReg(SCC_X_3, LCDDR17, true);
		break;
	default:
		return 1;
	}
	return 0;
}


/* A function for writing to a register, specifically the registers for 
   lighting up areas of the display. **/
int writeReg(int num, int reg, bool shift){
	if(!shift){
		reg &= 0xF0;
		reg |= num;
	}
	else{
		reg &= 0x0F;
		reg |= (num<<4);
	}
	return reg;
}
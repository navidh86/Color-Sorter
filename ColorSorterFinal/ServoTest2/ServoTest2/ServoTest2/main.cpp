/*
 * ServoTest2.cpp
 *
 * Created: 05-Sep-19 1:20:31 AM
 * Author : navid
 */ 

#define F_CPU 16000000

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#define gateMotor 1
#define railMotor 2

int gateCur = -1, railCur = -1;
void gateRotate(int target) {
	//d5
	if (gateCur == -1) {
		gateCur = target;
		OCR1A = gateCur;
		return;
	}
	while (target > gateCur) {
		gateCur += 10;
		OCR1A = gateCur;
		_delay_ms(1);
	}
	while (target < gateCur) {
		gateCur -= 10;
		OCR1A = gateCur;
		_delay_ms(1);
	}
}
void railRotate(int target) {
	//d4
	if (railCur == -1) {
		railCur = target;
		OCR1B = railCur;
		return;
	}
	while (target > railCur) {
		railCur += 10;
		OCR1B = railCur;
		_delay_ms(1);
	}
	while (target < railCur) {
		railCur -= 10;
		OCR1B = railCur;
		_delay_ms(1);
	}
}

void rotate(int motor, int angle)
{
	int val = 1450; //+ angle * 800 / 90;
	
	if(angle == -90)
	{
		val = 520;
	}
	else if(angle == -30)
	{
		val = 1000;
	}
	else if(angle == 30)
	{
		val = 1860;
	}
	else if(angle == 90)
	{
		val = 2360;
	}
	
	if (motor == gateMotor) {
		gateRotate(ICR1 - val);
	} else {
		railRotate(ICR1 - val);
	}
	
	_delay_ms(50);	
}

int main(void)
{
	DDRD |= 0xFF;
	TCCR1A |= 1<<WGM11 | 1<<COM1A1 | 1<<COM1A0 | 1<<COM1B0 | 1<<COM1B1;  //com1a for d5, com1b for d4, d5 = ocr1a, d4 = ocr1b
	TCCR1B |= 1<<WGM13 | 1<<WGM12 | 1<<CS10;
	ICR1 = 19999;

	while (1)
	{	
		rotate(gateMotor, 30);
		rotate(gateMotor, 0);
		rotate(railMotor, -30);
		
		rotate(gateMotor, -30);
		rotate(gateMotor, 30);
		rotate(gateMotor, 0);
		rotate(railMotor, 30);
		
		rotate(gateMotor, -30);
	}
}


/*
 * ColorSensor.cpp
 *
 * Created: 21-Aug-19 8:41:39 PM
 * Author : solaiman, navid, avijit
 */

// PORT A (pin 2-7) LCD purpose
// PORT B (pin 1) Sensor input
// PORT D (pin 0, 1) Sensor Color Mode Select
// PORT D (pin 4) Gate Motor Control
// PORT D (pin 5) Rail Motor Control


#define F_CPU 16000000

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#define RS eS_PORTA2
#define EN eS_PORTA3
#define D4 eS_PORTA4
#define D5 eS_PORTA5
#define D6 eS_PORTA6
#define D7 eS_PORTA7

#include "lcd.h"

#define S2 0
#define S3 1

#define TCS_OUT_PORT PINB
#define TCS_OUT_PIN 1

#define CLEAR 0
#define RED 1
#define GREEN 2
#define BLUE 3

#define YELLOW 4
#define WHITE 5

#define gateMotor 1
#define railMotor 2

void init()
{
	DDRA = 0xFF;
	DDRB = 0b11111101;
	DDRD = 0xFF;
	Lcd4_Init();
}

void sensorInit() {
	TCCR1A = 0;
	TCCR1B = 0b11000000;
}

void motorInit() {
	TCCR1A = 1<<WGM11 | 1<<COM1A1 | 1<<COM1A0 | 1<<COM1B0 | 1<<COM1B1;  //COM1A for d5, COM1B for d4, d5 = OCR1A, d4 = OCR1B
	TCCR1B = 1<<WGM13 | 1<<WGM12 | 1<<CS10;
	ICR1 = 19999;
}


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
	motorInit();
	int val = 1450; //+ angle * 800 / 90;
	
	if(angle == -90)
	{
		val = 520;
	}
	else if(angle == -30)
	{
		val = 1000;
	}
	else if(angle == -10)
	{
		val = 1300;
	}
	else if(angle == 10)
	{
		val = 1600;
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

void setColorMode(int mode)
{
	PORTD &= 0b11111100;

	if(mode == CLEAR)
	{
		PORTD |= (1<<S2);
	}
	else if(mode == RED)
	{

	}
	else if(mode == GREEN)
	{
		PORTD |= (1<<S2);
		PORTD |= (1<<S3);
	}
	else
	{
		PORTD |= (1<<S3);
	}
	int a = 5;
}


uint32_t normalize(uint32_t x) {
	//return x;
	return (double(sqrt(x)))/2000*99;
}

uint32_t getFrequency()
{
	if(!(TCS_OUT_PORT & (1<<TCS_OUT_PIN)))
	{
		while(!(TCS_OUT_PORT & (1<<TCS_OUT_PIN)));
	}

	while(TCS_OUT_PORT & (1<<TCS_OUT_PIN));

	TCNT1 = 0x0000;
	TCCR1B |= 1;

	while(!(TCS_OUT_PORT & (1<<TCS_OUT_PIN)));

	uint32_t period = TCNT1;
	TCCR1B ^= 1;
	
	//return 100 - period;
	if (period == 0) return 0;
	return normalize( F_CPU / ( (double)(2 * period) ) );
}

const int SAMPLE_SIZE = 50;
uint32_t getColor(int mode)
{
	setColorMode(mode);

	double total = 0;
	for (int i = 0; i < SAMPLE_SIZE; i++) {
		total += getFrequency();
	}

	total /= SAMPLE_SIZE;

	return total;
}



void printInt(uint32_t x, int row, int col) {
	char str[16];
	dtostrf((double)x, 2, 0, str);

	Lcd4_Set_Cursor(row,col);
	Lcd4_Write_String(str);
}

int detectColor(uint32_t r, uint32_t g, uint32_t b) {
	//return rand() % 3 + 1;
	
	if (g >= 90)				return RED;
	if (b <= 60 && r+g >= 140)	return WHITE;
	if (r + b >= 168)			return BLUE;
	if (120 <= r+g && r+g <= 146)	return YELLOW;
	
	return WHITE;
}

const int THRESHOLD = 50;
void loop() {
	Lcd4_Clear();
	Lcd4_Set_Cursor(1,0);
	Lcd4_Write_String("WAITING");
	
	rotate(gateMotor, 30);
	rotate(gateMotor, 0);
	
	Lcd4_Clear();
	Lcd4_Set_Cursor(1,0);
	Lcd4_Write_String("SENSOR READING");
	
	sensorInit();
	//while (getColor(RED)+getColor(BLUE)+getColor(GREEN) < THRESHOLD);

	Lcd4_Clear();
	Lcd4_Set_Cursor(2,0);
	Lcd4_Write_String("R:__ G:__ B:__");
	
	uint32_t r = getColor(RED);
	uint32_t g = getColor(GREEN);
	uint32_t b = getColor(BLUE);
	printInt(r, 2, 2);
	printInt(g, 2, 7);
	printInt(b, 2, 12);
	
	int color = detectColor(r, g, b);
	
	Lcd4_Set_Cursor(1,0);
	
	if (color == RED) {
		Lcd4_Write_String("RED");
		rotate(railMotor, -30);
	} else if (color == YELLOW) {
		Lcd4_Write_String("YELLOW");
		rotate(railMotor, -10);
	} else if (color == BLUE) {
		Lcd4_Write_String("BLUE");
		rotate(railMotor, 30);
	} else if (color == WHITE) {
		Lcd4_Write_String("WHITE");
		rotate(railMotor, 10);	
	}
	
	rotate(gateMotor, -30);
}


void test_oop() {
	Lcd4_Clear();
	Lcd4_Set_Cursor(1,0);
	Lcd4_Write_String("WAITING");
	
	rotate(gateMotor, 30);
	rotate(gateMotor, 0);
	//rotate(gateMotor, 30);
	//rotate(gateMotor, 0);
	
	Lcd4_Clear();
	Lcd4_Set_Cursor(1,0);
	Lcd4_Write_String("SENSOR READING");
	
	sensorInit();
	//while (getColor(RED)+getColor(BLUE)+getColor(GREEN) < THRESHOLD);
	
	Lcd4_Clear();
	Lcd4_Set_Cursor(1,0);
	Lcd4_Write_String("RED");
	uint32_t r = getColor(RED);
	printInt(r, 2, 0);
	_delay_ms(100);
	
	Lcd4_Clear();
	Lcd4_Set_Cursor(1,0);
	Lcd4_Write_String("GREEN");
	uint32_t g = getColor(GREEN);
	printInt(g, 2, 0);
	_delay_ms(100);
	
	Lcd4_Clear();
	Lcd4_Set_Cursor(1,0);
	Lcd4_Write_String("BLUE");
	uint32_t b = getColor(BLUE);
	printInt(b, 2, 0);
	_delay_ms(100);	
}


int main(void)
{
	init();
	//motorInit();
	//rotate(GATE_MOTOR, 0);

	Lcd4_Clear();
	Lcd4_Set_Cursor(1,0);
	Lcd4_Write_String("STARTING");
	
	motorInit();
	rotate(gateMotor, 0);
	rotate(railMotor, 0);

    while (1)
    {
		loop();
		//test_oop();
	}
}


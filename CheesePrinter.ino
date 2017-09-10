/*
Name:    Cheese Printer
Created: 9/7/2017 9:21:18 PM
Author:  Tony and Richard
*/

#include <Servo.h>
#include <SPI.h>
#include <Pixy.h>

#define spd 600 //ms between pulses
#define slowspd 2000
#define mm *40 // 80 steps per millimeter
#define pulseDelay 70 //ms delay

Servo cheese; //servo for cheese trigger
Pixy pixy; //load the pixi cam

long int xpos, ypos, zpos; //variables to hold current positions
int outputs[6] = { 2, 3, 4, 5, 6, 7 }; //Stepper ports
int endstops[3] = { 14, 15, 16 }; //endstops on A0, A1, A2

bool crackalackin = true; //Are their crackers
int crackers[50][2];
uint16_t numCrackers = 0;
char buf[32]; //for printing

void setup() {
	Serial.begin(9600);
	cheese.attach(8); //servo data is on pin 8
	for (int a = 0; a < 6; a++) {
		pinMode(outputs[a], OUTPUT); //all stepper control outputs
		digitalWrite(outputs[a], 1);
	}
	for (int a = 0; a < 3; a++) {
		pinMode(endstops[a], INPUT_PULLUP); //endstop inputs
	}

	pixy.init(); //probably won't work until we connect it.

	homePrinter();
	movexy(5 mm, 5 mm); //pull back from the endstops
	movez(10 mm);
	xpos = 0;
	ypos = 0;
	zpos = 0;
}

void loop() {

	//getCrackers();
	//delay(2000);
	//printCrackers();
	//drawCheeseSquare(55, 55, 50, 50);
	applyCheese(15, 45);
	applyCheese(46, 90);
	applyCheese(15, 33);
	applyCheese(100, 222);
}

void getCrackers() { //method checks pixycam for cracker blocks and add them to the cracker array
	uint16_t blocks;
	int i;
	blocks = pixy.getBlocks();
	if (blocks) {
		for (i = 0; i < numCrackers; i++)
		{
			//pixy.blocks[i].print(); //prints info to serial
			crackers[i][0] = pixy.blocks[i].x;
			crackers[i][1] = pixy.blocks[i].y;
		}
		numCrackers = blocks;
	}
	else numCrackers = 0;

}

void printCrackers()
{
	Serial.print("Number of Crackers found: ");
	Serial.println(numCrackers);
	for (int i = 0; i < numCrackers; i++)
	{
		Serial.print("Cracker:");
		Serial.print(i);
		Serial.print(" x:");
		Serial.print(crackers[i][0]);
		Serial.print(" y:");
		Serial.println(crackers[i][1]);
	}
}

void homePrinter()
{
	Serial.print("Homing Z...");
	while (digitalRead(16)) { //home z
		pulsez(0);
		delayMicroseconds(spd);
	}
	Serial.println("Homed");
	Serial.print("Homing x...");
	while (!digitalRead(14)) { //home x
		pulsex(0);
		delayMicroseconds(spd);
	}
	Serial.println("Homed");
	Serial.print("Homing y...");
	while (!digitalRead(15)) { //home y
		pulsey(0);
		delayMicroseconds(spd);
	}
	Serial.println("Homed");
}

void drawCheeseSquare(long int x_start, long int y_start, long int x_size, long int y_size)
{
	movexy(x_start mm, y_start mm);
	movez(55 mm); //raise the printbed
	squeezeCheese(1); //start cheesing
	movexy(x_size mm, 0);
	movexy(0, y_size mm);
	movexy(-x_size mm, 0);
	movexy(0, -y_size mm); //draw a square
	squeezeCheese(0);
	movez(-55 mm);
	home(); //return back to beginnning
	delay(500);
}

void squeezeCheese(bool on) {
	if (on) {
		cheese.write(120); //trigger on angle
		Serial.println("Squeezing the Cheese");
	}
	else {
		cheese.write(40); //trigger off angle
		Serial.println("No more Cheese Please");
	}
	delay(150);  //servo move delay need to test this
}


void moveto(long int x, long int y)
{
	movexy(x - xpos, y - ypos);

}


void movexy(long int x, long int y) { //moves to a new x and y position
	Serial.println("Moving XY");
	Serial.print("x: ");
	Serial.print(x);
	Serial.print(" y: ");
	Serial.println(y);
	xpos += x;
	ypos += y;
	//Serial.print("x = ");
	//Serial.println(xpos);
	// Serial.print("y = ");
	//Serial.println(ypos);
	bool xdir = x > 0;
	bool ydir = y > 0;
	x = abs(x);
	y = abs(y);
	long int steps = max(x, y);
	float xstep;
	if (x) { //dont divide by zero
		xstep = abs(steps / x);
	}
	else {
		xstep = abs(steps) + 1;
	}
	float ystep;
	if (y) {
		ystep = abs(steps / y);
	}
	else {
		ystep = abs(y) + 1;
	}
	float xcount = 0;
	float ycount = 0;
	long int xset = 0;
	long int yset = 0;
	while (yset != y || xset != x) { 
		xcount++;
		ycount++;
		if (xcount >= xstep && xset != x) {
			xset++;
			//Serial.print("x = ");
			//Serial.println(xset);
			xcount -= xstep;
			pulsex(xdir);
			delayMicroseconds(spd);
		}
		if (ycount >= ystep && yset != y) {
			yset++;
			// Serial.print("y = ");
			// Serial.println(yset);
			ycount -= ystep;
			pulsey(ydir);
			delayMicroseconds(spd);
		}
	}
}

void movez(long int z) {
	Serial.println("Moving Z");
	zpos += z;
	bool zdir = z > 1;
	for (int a = 0; a < abs(z); a++) {
		pulsez(zdir);
		delayMicroseconds(spd);
	}
}

void movezslow(long int z) {
	Serial.println("Moving Z");
	zpos += z;
	bool zdir = z > 1;
	for (int a = 0; a < abs(z); a++) {
		pulsez(zdir);
		delayMicroseconds(slowspd);
	}
}

void home() {
	movez(-zpos);
	movexy(-xpos, -ypos);
}

void pulsex(bool dir) {
	digitalWrite(3, dir);
	digitalWrite(2, 0);
	delayMicroseconds(pulseDelay);
	digitalWrite(2, 1);
}
void pulsey(bool dir) {
	digitalWrite(5, dir);
	digitalWrite(4, 0);
	delayMicroseconds(pulseDelay);
	digitalWrite(4, 1);
}
void pulsez(bool dir) {
	digitalWrite(7, !dir); //made this one go down
	digitalWrite(6, 0);
	delayMicroseconds(pulseDelay);
	digitalWrite(6, 1);
}

void applyCheese(long int x, long int y)
{
	//TODO: make sure it's at the right location
	moveto(x mm, y mm);
	movez(25 mm);
	squeezeCheese(1);
	movezslow(-25 mm); //move the nozzle up while dispensing cheese
	squeezeCheese(0);
}

void camToPrintbed()//convert from pixy cam coordinates to printer bed
{
	


}
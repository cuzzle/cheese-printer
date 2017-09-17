/*
Name:    Cheese Printer
Created: 9/7/2017 9:21:18 PM
Author:  Tony and Richard
*/

#include <Servo.h>
#include <Wire.h>
#include <PixyI2C.h>

#define spd 600 //ms between pulses
#define xyspd 440 //ms for x or y movement
#define slowspd 2500  //for dairyqueen style things
#define mm *40 // 40 steps per millimeter
#define pulseDelay 70 //ms delay

Servo cheese; //servo for cheese trigger
PixyI2C pixy(0x54);
int stepsPerMM = 40;

long int xpos, ypos, zpos; //variables to hold current positions
int outputs[6] = { 2, 3, 4, 5, 6, 7 }; //Stepper ports
int endstops[3] = { 14, 15, 16 }; //endstops on A0, A1, A2
int buttonPin = 11;

long int maxX = 575; //find and set maximum values and check whenever you move.
long int maxY = 425; //far back
long int maxZ = 0;

void setup() {
	Serial.begin(9600);
	cheese.attach(8); //servo data is on pin 8
	cheese.write(0); //check to see if this calms it down
	pinMode(buttonPin, INPUT_PULLUP);

	for (int a = 0; a < 6; a++) {
		pinMode(outputs[a], OUTPUT); //all stepper control outputs
		digitalWrite(outputs[a], 1);
	}

	for (int a = 0; a < 3; a++) {
		pinMode(endstops[a], INPUT_PULLUP); //endstop inputs
	}

	Wire.setClock(100000); //Try and change the speed

	pixy.init(); //probably won't work until we connect it.

	homePrinter();
	movexy(5 mm, 5 mm); //pull back from the endstops
	movez(7); //cracker taking picture level
	xpos = 0;
	ypos = 0;
	zpos = 0;
}

void loop() {
	bool buttonState = 1;
	buttonState = digitalRead(buttonPin);
	if (buttonState == 0)
	{
		Serial.println("You pressed the button!");
		getCrackers();
	}

}

void buttonPress()
{
	int buttonState = 0;
	buttonState = digitalRead(buttonPin);
	//Serial.print(buttonState);
	if (buttonState == 1)
	{
		Serial.println("pressed button");
		getCrackers();
	}
	else Serial.println("Seriosly Dude");

	delay(500);
}


void cheeseCracker(int x, int y)
{

	//delay(8000);
	Serial.print("Currently at: ");
	Serial.print(xpos);
	Serial.print(" , ");
	Serial.println(ypos);
	Serial.print("Apply Cheese to: ");
	Serial.print(x);
	Serial.print(" , ");
	Serial.println(y);

	movez(130); //move around height
	applyCheese(x, y);
	movez(-130); //back to starting height
	delay(500);
}

void getCrackers() { //method checks pixycam for cracker blocks and add them to the cracker array
	uint16_t blocks;
	Serial.println("Pre Get Crackers");
	blocks = pixy.getBlocks();
	Serial.println("Getting Crackers");
	Serial.println(blocks);
	if (blocks) {
		for (int i = 0; i < blocks; i++)
		{
			Serial.print("Pixy coordinates:");
			Serial.print(pixy.blocks[i].x);
			Serial.print(" = x, y = ");
			Serial.println(pixy.blocks[i].y);

			Serial.print("Pritner coordinates:");
			Serial.print( map(pixy.blocks[i].x, 90, 255, 50, 575) );
			Serial.print(" = x, y = ");
			Serial.println( map(pixy.blocks[i].y, 30, 166, 125, 435) );

			Serial.print("Get Crackers Currently at: ");
			Serial.print(xpos);
			Serial.print(" , ");
			Serial.println(ypos);
			cheeseCracker( map(pixy.blocks[i].x, 90, 255, 50, 575), map(pixy.blocks[i].y, 30, 166, 125, 435)); //maps pixy cam coordinates to printbed
		}
		moveto(0, 400 mm);
		delay(5000);
		home();
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
		delayMicroseconds(xyspd);
	}
	Serial.println("Homed");
	Serial.print("Homing y...");
	while (!digitalRead(15)) { //home y
		pulsey(0);
		delayMicroseconds(xyspd);
	}
	Serial.println("Homed");
}

void squeezeCheese(bool on) {
	if (on) {
		cheese.write(40); //trigger on angle
		Serial.println("Squeezing the Cheese");
	}
	else {
		cheese.write(0); //trigger off angle
		Serial.println("No more Cheese Please");
	}
	delay(150);  //servo move delay need to test this
}

void moveto(long int x, long int y)
{
	Serial.print("Currently at: ");
	Serial.print(xpos);
	Serial.print(" , ");
	Serial.println(ypos);
	Serial.print("Moving to: ");
	Serial.print(x);
	Serial.print(" , ");
	Serial.println(y);
	movexy(x - xpos, y - ypos);
}


void movexy(long int x, long int y) { //moves to a new x and y position
	Serial.println("Moving XY");
	xpos += x;
	ypos += y;
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
			delayMicroseconds(xyspd);
		}
		if (ycount >= ystep && yset != y) {
			yset++;
			// Serial.print("y = ");
			// Serial.println(yset);
			ycount -= ystep;
			pulsey(ydir);
			delayMicroseconds(xyspd);
		}
	}
}

void movez(long int z) {
	Serial.println("Moving Z");
	z *= stepsPerMM;
	zpos += z;
	bool zdir = z > 1;
	for (int a = 0; a < abs(z); a++) {
		pulsez(zdir);
		delayMicroseconds(spd);
	}
}

void movezslow(long int z) {
	z *= stepsPerMM;
	Serial.println("Moving Z");
	zpos += z;
	bool zdir = z > 1;
	for (int a = 0; a < abs(z); a++) {
		pulsez(zdir);
		delayMicroseconds(slowspd);
	}
}

void home() {
	//Serial.print
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
	Serial.print("Currently at: ");
	Serial.print(xpos);
	Serial.print(" , ");
	Serial.println(ypos);
	Serial.print("Moving to: ");
	Serial.print(x);
	Serial.print(" , ");
	Serial.println(y);

	moveto(x mm, y mm);
	movez(70);
	squeezeCheese(1);
	movezslow(-20); //move the nozzle up while dispensing cheese
	squeezeCheese(0);
	movez(-50);
}

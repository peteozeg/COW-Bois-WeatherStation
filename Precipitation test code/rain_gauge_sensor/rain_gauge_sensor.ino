#include <ESP32Servo.h>
#include <Arduino.h>

#define servoPin 15
#define adcPin_1 36

Servo drainageServo;  // create servo object to control a servo
// 16 servo objects can be created on the ESP32

int pos = 0;    // variable to store the servo position
bool needToDrain = false;
bool draining = false;
int waterValue = 0;
int counter = 0;

int checkDelay = 1000;
int printDelay = 1000;

unsigned long timer = 0;
unsigned long printTimer = 0;
unsigned long drainTimer = 0;


void setup() {

	// Setup the serial monitor
	Serial.begin(115200);
	Serial.println("Begin");
	Serial.println();
	delay(5000);
	drainageServo.setPeriodHertz(50);    // standard 50 hz servo
	drainageServo.attach(servoPin, 1000, 2000);
	// using default min/max of 1000us and 2000us
	// different servos may require different min/max settings
	// for an accurate 0 to 180 sweep

	timer = millis();
	printTimer = millis();
	drainTimer = millis();
}

void loop() {
	if(millis() - timer >= checkDelay) {
		waterValue = analogRead(adcPin_1);
		if (waterValue >= 1850) {
			needToDrain = true;
		}
		timer += checkDelay;
	}
	if (millis() - printTimer >= printDelay) {
		Serial.print("Water Value = ");
		Serial.println(waterValue);
		printTimer += printDelay;
	}
	if (needToDrain) {
		drainTimer = millis();
		draining = true;
	}
	if (draining) {
		drainDevice();
	}
	drainageServo.write(pos);
}

void drainDevice() {
	needToDrain = false;
	pos = 60;
	if (millis() - drainTimer >= 1000) {
		counter++;
		Serial.println(counter);
		if (counter > 20) {
			counter = 0;
			pos = 0;
			draining = false;
		}
		drainTimer += 1000;
	}

}
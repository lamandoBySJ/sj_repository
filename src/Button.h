#pragma once
#include "Arduino.h"
#include <FunctionalInterrupt.h>

class Button
{
public:
	Button(uint8_t reqPin) : PIN(reqPin){
		pinMode(PIN, INPUT);
		attachInterrupt(PIN, std::bind(&Button::isr,this), RISING);
	};
	~Button() {
		detachInterrupt(PIN);
	}

	void  isr() {
		detachInterrupt(PIN);
		_shakeFilter.attach(mbed::callback(this,&Button::setPressed),std::chrono::milliseconds(1000));
	}
	void setPressed(){
		numberKeyPresses += 1;
		pressed = true;
		Serial.println(numberKeyPresses);
		attachInterrupt(PIN, std::bind(&Button::isr,this), RISING);
	}
	void checkPressed() {
		if (pressed) {
			Serial.printf("Button on pin %u has been pressed %u times\n", PIN, numberKeyPresses);
			pressed = false;
		}
	}

private:
	const uint8_t PIN;
    volatile uint32_t numberKeyPresses;
    volatile bool pressed;
	mbed::Timeout _shakeFilter;
};
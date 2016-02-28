#include <Arduino.h>
#include "display.h"



unsigned long  timerOffset(Display *display){
	unsigned long currentMicros = micros();
	unsigned long t = currentMicros - display->timer;
	if (t >= ( 6 * display->interval) ) {
		display->timer = currentMicros;
	}	
	return t;
}



void renderBank(Display *display, int bank, int value){
	unsigned long interval = display->interval;
	unsigned long  offset = timerOffset(display);
	if(offset >= (bank) * interval && 
		offset < (bank+1) * interval &&
		!display->lock[bank]){
			
		if(bank < 0 || bank >5)
			return;
		if(value < 0 || value > 9){
			Serial.println(value);
			return;
		}
	
		int corrected = value == 0 ? 0 : 10-value;
	
		if(corrected < 0 || corrected > 9){
			Serial.println(corrected);
			return;
		}
	
		for(int i = 0; i < 4; i++){
			digitalWrite(display->bcdPins[i], 
				display->bcdTable[corrected][i]);
		}
		digitalWrite(display->anodePins[bank], HIGH);
		display->lock[bank] = true;
	}else{
		digitalWrite(display->anodePins[bank], LOW);
		display->lock[bank] = false;
	}
}

void render(Display *display, char data[6]){
	
	 for (int i = 0; i < 6; i++){
		renderBank(display, i, data[i]); 
	 }
}

void initialize(Display *display){
	for(int i = 0; i < 4; i++){
		pinMode(display->bcdPins[i], OUTPUT);
	}
	for(int i = 0; i < 6; i++){
		pinMode(display->anodePins[i], OUTPUT);
	}
	display->timer = micros();
}


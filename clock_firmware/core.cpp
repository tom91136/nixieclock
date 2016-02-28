#include <Arduino.h>
#include <SoftwareSerial.h>
#include <string.h>
#include "display.h"

#define H_12 true

SoftwareSerial debug(18, 19); // RX, TX

typedef struct Status{
	long long current;
	long long offset;
	long long interval;
	String mode;
} Status;

Display display;
Status status;

inline long long currentTime(Status *status){
	return status->current + status->offset;
}

char nthdigit(unsigned long value, int n){
    static unsigned long powersof10[] = {1, 10, 100, 1000, 10000ul, 100000ul};
    return ((value / powersof10[n]) % 10);
}

void renderStatus(Status *status){
	char mode = status->mode[0];
	switch(mode){
		case 's':{
			// standard HMS
			long long current = currentTime(status);
			int s = (current / 1000L) % 60 ;
			int m = ((current / 60000L) % 60);
			int h = ((current / 3600000L) % 24);
			
			// 12 hour 
			if(H_12){
				h = h > 12 ? h - 12 : h;
			}
			
			//Serial.print('>');
			//Serial.print(s);
			//Serial.print(':');
			//Serial.print(m);
			//Serial.print(':');
			//Serial.println(h);
			char data []= {h/10, h%10, m/10, m%10, s/10, s%10};
			render(&display, data);
			break;
		}
		case 'c':{
			// counter + unit
			long i = 0;
			char data[] = {nthdigit(i, 5), nthdigit(i, 4), nthdigit(i, 3), nthdigit(i, 2), nthdigit(i, 1), nthdigit(i, 0)};
			render(&display, data);
			break;
		}
	}
}

void writeSuccess(int success){
	Serial.println(success ? 1 : 0);
	debug.println(success ? 1 : 0);
}

// h<string>:<string>
void heartbeat(Status *status, String input){
	Serial.println(input);
}

long long unsafe_atoll(const char *instr){
  long long retval;
  int i;

  retval = 0;
  for (; *instr; instr++) {
    retval = 10*retval + (*instr - '0');
  }
  return retval;
}

//s <long>:<bool>
void setTime(Status *status, String input){
	status->offset = unsafe_atoll(input.c_str()) - millis();
	writeSuccess(true);
}

//c:<long>
void getTime(Status *status, String input){
	long long time = currentTime(status);
	// magic
	char buffer[100];
	sprintf(buffer, "%0ld", time/1000000L); 
	Serial.print(buffer);  
	sprintf(buffer, "%0ld", time%1000000L); 
	Serial.println(buffer);  
	
	//Serial.println(currentTime(status));
}

//r:<bool>
void invalidate(Status *status, String input){
	renderStatus(status);
	writeSuccess(true);
}

//t <int>:<bool>
void invalidateInterval(Status *status, String input){
	status->interval = input.toInt();
	writeSuccess(true);
}

//m <[c+(H|M|S|s)]|[s]>:<bool>
void setMode(Status *status, String input){
	status->mode = input;
	renderStatus(status);
	writeSuccess(true);
}

void decode(Status *status, String input){
	input.trim();
	int length = input.length();
	if(length == 0) return;
	if(input[0] != '@') return;
	
	char op = input[1];
	
	// we got arguments
	if(length > 3){
		input = input.substring(3);
	}
	Serial.print(op);
	Serial.print('>');
	Serial.println(input);
	
	switch(op){
		case 'h':
			heartbeat(status, input);
		break;
		case 's':
			setTime(status, input);
		break;
		case 'c':
			getTime(status, input);
		break;
		case 'r':
			invalidate(status, input);
		break;
		case 't':
			invalidateInterval(status, input);
		break;
		case 'm':
			setMode(status, input);
		break;
	}
}



String buffer = "";         // a string to hold incoming data
boolean flush = false;  // whether the string is complete



void setup(){
	
	pinMode(18, INPUT);
	pinMode(19, OUTPUT);
	debug.begin(9600);
	
	debug.println(F("Debug OK"));
	Serial.begin(9600);
	
	buffer.reserve(200);
	
	display = (Display){
		.bcdPins = {2, 3, 4, 5}, 
		.anodePins = {10, 11, 12, 13, 8, 9},
		.bcdTable = {
						{0, 0, 0, 0},
						{1, 0, 0, 0},
						{0, 1, 0, 0},
						{1, 1, 0, 0},
						{0, 0, 1, 0},
						{1, 0, 1, 0},
						{0, 1, 1, 0},
						{1, 1, 1, 0},
						{0, 0, 0, 1},
						{1, 0, 0, 1}
					},
		.lock = { false },
		.timer = millis(),
		.interval = 1024
	};
	status = (Status) {millis(), 0, 1000, "s"};
	initialize(&display);
	
}



unsigned long previousMillis = 0;

void loop(){
	
	if(flush){
		//Serial.println(buffer);
		decode(&status, buffer);
		buffer = "";
		flush = false;
	}
	
	renderStatus(&status);
	
	 unsigned long currentMillis = millis();
	 if (currentMillis - previousMillis >= status.interval) {
		previousMillis = currentMillis;
		status.current = millis();
	 }
}

void serialEvent() {
	while (Serial.available()) {
		char c = (char)Serial.read();
		debug.print('[');
		debug.print(c);
		debug.print(']');
		buffer += c;
		if (c == '\n') {
			flush = true;
		}
	}
}





#ifndef DISPLAY_H
#define DISPLAY_H

#include "Arduino.h"

typedef struct {
	int bcdPins[4];
	int anodePins[6];
	int bcdTable[10][4];
	bool lock[6];
	unsigned long timer;
	unsigned long interval;
} Display;

void render(Display *display, char data[6]);

void initialize(Display *display);

#endif

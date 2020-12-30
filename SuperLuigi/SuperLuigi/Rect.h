#pragma once
#include <stdio.h>

class Rect {
public:
	int x, y, w, h;
	
	Rect() {
		x = y = w = h = 0;
	}
	
	Rect(int x, int y, int w, int h) {
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
	}
};
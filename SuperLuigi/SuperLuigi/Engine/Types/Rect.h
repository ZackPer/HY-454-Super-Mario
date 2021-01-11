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

	bool operator==(const Rect b) const {
		return (
			this->x == b.x &&
			this->y == b.y &&
			this->w == b.w &&
			this->h == b.h
		);
	}
};


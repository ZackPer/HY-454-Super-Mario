#pragma once
#include <stdio.h>

class Point {
public:
	int x, y;

	Point() {
		x = y = 0;
	}

	Point(int x, int y) {
		this->x = x;
		this->y = y;
	}
};
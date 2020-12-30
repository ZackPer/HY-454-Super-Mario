#pragma once

#include <allegro5/allegro.h>
#include <stdio.h>
#include "Rect.h"

class ViewWindow {
public:
	ALLEGRO_BITMAP* camera;
	Rect dimensions;
	Rect displayArea;

	ViewWindow() {
		camera = nullptr;
		dimensions = Rect(0, 0, 0, 0);
		displayArea = Rect(0, 0, 0, 0);
	}

	ViewWindow(Rect dimensions, Rect displayArea) {
		this->dimensions = dimensions;
		this->displayArea = displayArea;
		this->camera = al_create_bitmap(dimensions.h, dimensions.w);
	}

	ViewWindow(
		int dimWidth, int dimHeight, int dimX, int dimY,
		int dispWidth, int dispHeight, int dispX, int dispY
	) {
		this->dimensions = Rect(dimX, dimY, dimWidth, dimHeight);
		this->displayArea = Rect(dispX, dispY, dispWidth, dispHeight);
		this->camera = al_create_bitmap(dimensions.w, dimensions.h);
	}

};
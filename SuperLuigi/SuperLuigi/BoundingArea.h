#pragma once
#include "Types.h"

class BoundingBox;

class BoundingArea {
public:
	BoundingArea() {}
	virtual bool Intersects(const BoundingArea* shape) const = 0;
	virtual bool Intersects(const BoundingBox* box2) const = 0;
};

class BoundingBox : BoundingArea {
public:
	Rect* area;
	BoundingBox() {}

	bool Intersects(const BoundingArea* shape) const {
		return shape->Intersects(this);
	}
	//TO FIX
	bool Intersects(const BoundingBox* box2) const{
		return(
			((area->x < box2->area->x && box2->area->x < area->x + area->w) && (area->y < box2->area->y&& box2->area->y < area->y + area->h)) ||
			((box2->area->x < area->x && area->x < box2->area->x + box2->area->w) && (box2->area->y < area->y && area->y < box2->area->y + box2->area->h))
		);
	}
};
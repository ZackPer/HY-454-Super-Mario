#pragma once
#include <iostream>
#include <functional>
#include "Types.h"

template <typename Tnum>
int number_sign(Tnum x) {
	return x > 0 ? 1 : x < 0 ? -1 : 0;
}

// generic quantizer, can be used to filter motion with any terrain
// motion filtering function
class MotionQuantizer {
public:
	using Mover = std::function<void(Rect& r, int* dx, int* dy)>;
protected:
	int horizMax = 0, vertMax = 0;
	Mover mover; // filters requested motion too!
	bool used = false;
public:
	MotionQuantizer& SetUsed(bool val) { this->used = val; };
	MotionQuantizer& SetRange(int h, int v)
	{
		horizMax = h, vertMax = v; used = true; return *this;
	}
	MotionQuantizer& SetMover(const Mover & f)
	{
		mover = f; return *this;
	}
	
	void Move(Rect& r, int* dx, int* dy) {
		if (!used)
			mover(r, dx, dy);
		else
			do {
				auto sign_x = number_sign(*dx);
				auto sign_y = number_sign(*dy);
				auto dxFinal = sign_x * std::min(horizMax, sign_x * *dx);
				auto dyFinal = sign_y * std::min(vertMax, sign_y * *dy);
				mover(r, &dxFinal, &dyFinal);
				if (!dxFinal) // X motion denied
					*dx = 0;
				else
					*dx -= dxFinal;
				if (!dyFinal) // Y motion denied
					*dy = 0;
				else
					*dy -= dyFinal;
			} while (*dx || *dy);
	}

	MotionQuantizer(void) = default;
	MotionQuantizer(const MotionQuantizer&) = default;
};


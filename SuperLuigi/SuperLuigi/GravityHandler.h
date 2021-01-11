#pragma once

#include <iostream>
#include <functional>
#include "Types.h"

class GravityHandler {
public:
	using OnSolidGroundPred = std::function<bool(Rect&)>;
	using OnStartFalling = std::function<void(void)>;
	using OnStopFalling = std::function<void(void)>;
protected:
	bool gravityAddicted = false;
	bool isFalling = false;
	OnSolidGroundPred onSolidGround;
	OnStartFalling onStartFalling;
	OnStopFalling onStopFalling;
public:

	void SetGravityAddicted(bool addiction) {
		gravityAddicted = addiction;
	}

	bool GetGravityAddicted() {
		return gravityAddicted;
	}

	template <typename T> void SetOnStartFalling(const T& f){
		onStartFalling = f;
	}
	template <typename T> void SetOnStopFalling(const T& f){
		onStopFalling = f;
	}
	template <typename T> void SetOnSolidGround(const T& f){
		onSolidGround = f;
	}

	void Reset(void) { 
		isFalling = false; 
	}

	void Check(Rect& r) {
		if (gravityAddicted) {
			if (onSolidGround(r)) {
				if (isFalling) {
					isFalling = false;
					if (onStopFalling)
						onStopFalling(); //stop falling animation
				}
			}
			else
				if (!isFalling) {
					isFalling = true;
					if(onStartFalling)
						onStartFalling(); //start falling animation
				}	
		}
	}
};
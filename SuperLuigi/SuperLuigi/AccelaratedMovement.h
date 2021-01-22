#pragma once

#include <iostream>
#include <math.h>

class AccelaratedMovement {
public:
	float			height = 80;
	float			U;
	float			Uo;
	float			a;
	uint64_t		duration = pow(10, 6);
	uint64_t		startTime;
	uint64_t		endTime;

	//This dictates if the movement is accelarated or decelarated
	enum Direction {
		POSITIVE = 1, NEGATIVE = -1
	};
	Direction direction;

	AccelaratedMovement() = default;
	AccelaratedMovement(float height, uint64_t duration, Direction direction){
		this->height = height;
		this->duration = duration;
		this->direction = direction;

	}

	float FindInitialVelocity() {
		float t = toSeconds(duration);
		return - direction * 2 * height / t;
	}

	float FindAcceleration() {
		float t = toSeconds(duration);
		return - direction * 2 * height / pow(t, 2);
	}

	float FindCurrentVelocity(uint64_t currTime) {
		auto t = toSeconds(currTime);
		return Uo * t + direction * (a * pow(t, 2)) / 2;
	}

	int FindCurrentOffset(uint64_t currTime) {
		auto tmicro = currTime - startTime;
		float t = toSeconds(currTime - startTime);
		return Uo * t + direction * (a * pow(t, 2)) / 2;
	}

	void PrepareAccelarationPhysics(uint64_t currTime) {
		Uo = 0;
		a = 300;
		direction = POSITIVE;
		startTime = currTime;
		endTime = -1;
	}

	void PrepareDecelarationPhysics(uint64_t currTime) {
		Uo = FindInitialVelocity();
		a = FindAcceleration();
		startTime = currTime;
		endTime = startTime + duration;
	}

	float toSeconds(uint64_t micro) {
		return micro * pow(10, -6);
	}
};
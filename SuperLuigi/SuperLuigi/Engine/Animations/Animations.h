#pragma once

#include <iostream>

class Animation {
protected:
	std::string id;
public:
	const std::string& GetId(void) { return id; }
	void				SetId(const std::string& _id) { id = _id; };
	virtual Animation* Clone(void) const = 0;
	Animation(const std::string& _id) : id(_id) {}
	Animation() {}
	virtual ~Animation() {}
};

class MovingAnimation : public Animation {
protected:
	unsigned	reps = 1; // 0 = inf
	int			dx = 0, dy = 0;
	unsigned	delay = 0;
public:
	using Me = MovingAnimation;
	int GetDx(void) const { return dx; }
	Me& SetDx(int v) { dx = v; return *this; }
	int GetDy(void) const { return dy; }
	Me& SetDy(int v) { dy = v; return *this; }
	unsigned GetDelay(void) const { return delay; }
	Me& SetDelay(unsigned v) { delay = v; return *this; }
	unsigned GetReps(void) const { return reps; }
	Me& SetReps(unsigned n) { reps = n; return *this; }
	bool IsForever(void) const { return !reps; }
	Me& SetForever(void) { reps = 0; return *this; }
	Animation* Clone(void) const override
	{
		return new MovingAnimation(id, reps, dx, dy, delay);
	}

	MovingAnimation(
		const std::string& _id, unsigned _reps, int _dx, int _dy, unsigned _delay
	) : Animation(_id), reps(_reps), dx(_dx), dy(_dy), delay(_delay) {}

	MovingAnimation() {}
};

class FrameRangeAnimation : public MovingAnimation {
protected:
	unsigned start = 0, end = 0;
public:
	using Me = FrameRangeAnimation;
	unsigned GetStartFrame(void) const { return start; }
	Me& SetStartFrame(unsigned v) { start = v; return *this; }
	unsigned GetEndFrame(void) const { return end; }
	Me& SetEndFrame(unsigned v) { end = v; return *this; }
	Animation* Clone(void) const override {
		return new FrameRangeAnimation(
			id, start, end, GetReps(), GetDx(), GetDy(), GetDelay()
		);
	}
	FrameRangeAnimation(
		const std::string& _id,
		unsigned start, unsigned end,
		unsigned reps, int dx, int dy, int delay
	) : start(start), end(end), MovingAnimation(id, reps, dx, dy, delay) {}

	FrameRangeAnimation() {}
};
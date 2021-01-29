#pragma once
#include "../Engine/SystemClock.h"
#include "../Engine/Sprite/Sprite.h"
#include "../Engine/Animations/Animations.h"
#include "../Engine/Animations/Animators.h"


class EntityHolder;

class DeathModule {
private:
	Sprite*			sprite;
	MovingAnimator	animator;
	MovingAnimation animation;
	AnimationFilm*	film;
	bool			dead = false;
	bool			hasFinished = false;
	
public:
	//since only mario will have it, bad solution but okee
	DeathModule(){}

	DeathModule(Sprite* s);
	
	bool GetDead() {
		return dead;
	}

	void Die();
};
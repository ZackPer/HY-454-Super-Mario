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
	bool			dead;
	
public:
	//since only mario will have it, bad solution but okee
	DeathModule(){}

	DeathModule(Sprite* s);

	bool			hasFinished;

	void Die() {
		sprite->SetCurrFilm(film);
		sprite->SetFrame(0);
		animator.Start(&animation, SystemClock::Get().micro_secs());
	}
};
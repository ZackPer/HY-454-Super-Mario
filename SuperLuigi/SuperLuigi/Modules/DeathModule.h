#pragma once
#include "../Engine/SystemClock.h"
#include "../Engine/Sprite/Sprite.h"
#include "../Engine/Animations/Animations.h"
#include "../Engine/Animations/Animators.h"

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

	DeathModule(Sprite* s) {
		dead = false;
		sprite = s;
		film = AnimationFilmHolder::Get().GetFilm("mario.death");
		
		animation =  MovingAnimation(
			"mario.death",
			8,
			0,
			-5,
			30000
		);

		animator.SetOnAction(
			[=](Animator* animator, const Animation& anim) {	
				sprite->SetHasDirectMotion(true).Move(((const MovingAnimation&)anim).GetDx(), ((const MovingAnimation&)anim).GetDy()).SetHasDirectMotion(true);
			}
		);

		animator.SetOnFinish(
			[=](Animator* animator) {
					animation = MovingAnimation(
						"mario.death",
						0,
						0,
						5,
						30000
					);
					((MovingAnimator*)animator)->Start(&animation, SystemClock::Get().micro_secs());
					animator->SetOnFinish(
						[=](Animator* animator) {
							animation = MovingAnimation(
								"mario.death",
								4,
								0,
								-8,
								30000
							);
						}
					);				
			}
		);
	}

	void Die() {
		sprite->SetCurrFilm(film);
		sprite->SetFrame(0);
		animator.Start(&animation, SystemClock::Get().micro_secs());
	}
};
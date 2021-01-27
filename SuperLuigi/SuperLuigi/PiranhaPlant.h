#include <iostream>
#include <functional>

#include "Engine/Sprite/Sprite.h"
#include "SpriteEntity.h";

class PiranhaPlant : SpriteEntity {
public:
	PiranhaPlant() = default;
	PiranhaPlant(int x, int y) 
		: SpriteEntity(x + 8, y + 8, AnimationFilmHolder::Get().GetFilm("piranha.plant"), "piranha.plant")
	{
		GetSelf()->SetVisibility(true);
		GetSelf()->SetHasDirectMotion(true);
		frameAnimation = FrameRangeAnimation("piranha.plant", 0, 2, 0, 0, 0, 400000);
		frameAnimator.Start(this->GetSelf(), &frameAnimation, SystemClock::Get().micro_secs());
		
		movingAnimator.SetOnFinish(
			[=](Animator *animator) {
				sign *= -1;
				Move(sign);
			}
		);
		Move(1);
	}

	void Move(int sign) {
		movingAnimation = MovingAnimation("piranha.plant.move", 30, 0, 1, 60000);
		movingAnimator.SetOnAction(
			[=](Animator *animator, const Animation& animation) {
				this->GetSelf()->Move(0, sign * speed);
			}
		);
		movingAnimator.Start(&movingAnimation, SystemClock::Get().micro_secs());
	}

private:
	FrameRangeAnimation		frameAnimation;
	FrameRangeAnimator		frameAnimator;
	MovingAnimation			movingAnimation;
	MovingAnimator			movingAnimator;
	int						speed = 1;
	int						sign = 1;


	void OnStartFalling() {

	}
	void OnStopFalling() {

	}
};
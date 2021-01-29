#include <iostream>
#include <functional>

#include "Engine/Sprite/Sprite.h"
#include "./Engine/Physics/CollisionHander.h";
#include "SpriteEntity.h";
#include "EntityHolder.h"

class PiranhaPlant : SpriteEntity {
public:
	MovingAnimator			movingAnimator;
	MovingAnimator			pauseAnimator;

	PiranhaPlant() = default;
	PiranhaPlant(int x, int y) 
		: SpriteEntity(x, y, AnimationFilmHolder::Get().GetFilm("piranha.plant"), "piranha.plant")
	{
		GetSelf()->SetVisibility(true);
		GetSelf()->SetHasDirectMotion(true);
		frameAnimation = FrameRangeAnimation("piranha.plant", 0, 2, 0, 0, 0, 400000);
		frameAnimator.Start(this->GetSelf(), &frameAnimation, SystemClock::Get().micro_secs());
		
		movingAnimator.SetOnFinish(
			[=](Animator *animator) {
				sign *= -1;
				pauseAnimator.Start(&movingAnimation, SystemClock::Get().micro_secs());
			}
		);
		pauseAnimator.SetOnFinish(
			[this](Animator *animator) {
				if (sign == -1 && isHidden) {
					pauseAnimator.Start(&movingAnimation, SystemClock::Get().micro_secs());
				}
				else
					Move(sign);
			}
		);
		Move(1);

		hideTimerAnimation = MovingAnimation("piranha.hide.timer", 1, 0, 0, 1000000);
		hideTimerAnimator.SetOnStart(
			[this](Animator *animator) {
				this->SetHidden(true);
			}
		);
		hideTimerAnimator.SetOnFinish(
			[this](Animator *animator) {
				this->SetHidden(false);
			}
		);

		std::function<void(Sprite *, Sprite *)> onCollision = [=](Sprite *supermario, Sprite *piranha) {
			EntityHolder::Get().GetSuperMario()->Die();
		};
		CollisionChecker::GetSingleton().Register(EntityHolder::Get().GetSuperMario()->GetSelf(), GetSelf(), onCollision);
	}

	void Move(int sign) {
		movingAnimation = MovingAnimation("piranha.plant.move", 25, 0, 1, 60000);
		movingAnimator.SetOnAction(
			[=](Animator *animator, const Animation& animation) {
				this->GetSelf()->SetBoundingArea();
				this->GetSelf()->Move(0, sign * speed);
			}
		);
		movingAnimator.Start(&movingAnimation, SystemClock::Get().micro_secs());
	}

	bool IsHidden() {
		return isHidden;
	}
	void SetHidden(bool isHidden) {
		this->isHidden = isHidden;
	}
	
	void Hide() {
		hideTimerAnimator.Start(&hideTimerAnimation, SystemClock::Get().micro_secs());
	}

private:
	FrameRangeAnimation		frameAnimation;
	FrameRangeAnimator		frameAnimator;
	MovingAnimation			movingAnimation;
	
	MovingAnimation			hideTimerAnimation;
	MovingAnimator			hideTimerAnimator;
	bool					isHidden = false;
	int						speed = 1;
	int						sign = 1;


	void OnStartFalling() {

	}
	void OnStopFalling() {

	}
};
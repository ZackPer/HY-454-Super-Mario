#pragma once

#include <iostream>
#include <functional>
#include "Engine/Sprite/Sprite.h"
#include "Engine/Physics/CollisionHander.h"
#include "MovingEntity.h"
#include "EntityHolder.h"
#include "PrimitiveHolder.h"

void DeathJump(MovingEntity* entity);

class Shell : MovingEntity {
public:
	using CollisionCallback = std::function<void(Sprite *s1, Sprite *s2)>;

	Shell(int x, int y, std::string color, GridLayer *myGrid)
		: MovingEntity(x, y, AnimationFilmHolder::Get().GetFilm(color + ".koopa.shell"), "shell", myGrid) 
	{
		FrameRangeAnimation move = FrameRangeAnimation(color + ".koopa.shell", 1, 2, 0, 0, 0, 300000);
		this->SetWalkLeft(move);
		this->SetWalkRight(move);
		this->SetSpeed(0);
		this->StartMoving();
		this->GetSelf()->SetVisibility(true);

		// On first collision:
		std::function<void(Sprite *, Sprite *)> f = [=](Sprite *mario, Sprite *shellSprite) {
			Rect marioBox = mario->GetBox();
			Rect shellBox = shellSprite->GetBox();
			Shell *self = this;
			int marioXCenter = (marioBox.x + marioBox.w) / 2;
			int shellXCenter = (shellBox.x + shellBox.w) / 2;

			this->SetSpeed(3);
			if (marioXCenter < shellXCenter) {
				this->SetSign(1);
			}
			else if (marioXCenter >= shellXCenter) {
				this->SetSign(-1);
			}
			CollisionChecker::GetSingleton().Cancel(shellSprite);

			//This is used as a ticking bomb; After 0.5 seconds collide wwith all.
			MovingAnimation *animation = new MovingAnimation("", 1, 0, 0, 300000);
			MovingAnimator  *animator = new MovingAnimator();
			animator->SetOnAction(
				[self](Animator *animator, const Animation& animation) {
					CollisionCallback deathOnCollide = [](Sprite *shell, Sprite *sprite) {
						Mario *mario = (Mario *)EntityHolder::Get().GetSuperMario();
						mario->Die();
					};
					CollisionCallback deathJumpOnCollide = [](Sprite *shell, Sprite *sprite) {
						MovingEntity *entity= (MovingEntity *)EntityHolder::Get().GetSpriteEntity(sprite);
						DeathJump(entity);
					};
					Mario *supermario = EntityHolder::Get().GetSuperMario();
					for (auto &it : EntityHolder::Get().FindByType("enemy"))
						if (self != it && supermario != it)
							CollisionChecker::GetSingleton().Register(self->GetSelf(), it->GetSelf(), deathJumpOnCollide);

					CollisionChecker::GetSingleton().Register(self->GetSelf(), supermario->GetSelf(), deathOnCollide);
				}
			);
			animator->Start(animation, SystemClock::Get().micro_secs());
		};
		CollisionChecker::GetSingleton().Register(EntityHolder::Get().GetSuperMario()->GetSelf(), this->GetSelf(), f);
	}

private:

};
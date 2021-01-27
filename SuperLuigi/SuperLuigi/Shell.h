#pragma once

#include <iostream>
#include <functional>
#include "Engine/Sprite/Sprite.h"
#include "Engine/Physics/CollisionHander.h"
#include "MovingEntity.h"
#include "EntityHolder.h"

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
					CollisionCallback killOnCollide = [](Sprite *shell, Sprite *sprite) {
						SpriteEntity *entity= EntityHolder::Get().GetSpriteEntity(sprite);
						entity->Die();
					};
					auto spriteList = SpriteManager::GetSingleton().GetDisplayList();
					auto asd = EntityHolder::Get().GetEntityList();
					self->GetSelf();
					for (auto &it : EntityHolder::Get().GetEntityList())
						if (self != it)
							CollisionChecker::GetSingleton().Register(self->GetSelf(), it->GetSelf(), killOnCollide);
				}
			);
			animator->Start(animation, SystemClock::Get().micro_secs());
		};
		CollisionChecker::GetSingleton().Register(EntityHolder::Get().GetSuperMario()->GetSelf(), this->GetSelf(), f);
	}

private:

};
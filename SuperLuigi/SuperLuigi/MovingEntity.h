#pragma once

#include <iostream>
#include <functional>
#include "Engine/Animations/Animations.h"
#include "Engine/Animations/Animators.h"
#include "Engine/Sprite/Sprite.h"
#include "Engine/SystemClock.h"
#include "Engine/Grid/Grid.h"
#include "SpriteEntity.h"
#include "MovementAI.h"

class MovingEntity : public SpriteEntity {
public:
	MovingEntity() = default;
	MovingEntity(int x, int y, AnimationFilm *film, std::string type, GridLayer *myGrid)
		: SpriteEntity(x, y, film, type)
	{
		this->myGrid = myGrid;
		self->SetVisibility(true);
		InitGravity(myGrid);

		movement = MovementAI(self, 1);
		movement.Init(myGrid);
		movement.SetOnSignChange(
			[=](int sign) {
			animator.Stop();
			if (sign == 1) {
				auto film = AnimationFilmHolder::Get().GetFilm(walkRight.GetId());
				GetSelf()->SetCurrFilm(film);
				animator.Start(self, &walkRight, SystemClock::Get().micro_secs());
			}
			else if (sign == -1) {
				auto film = AnimationFilmHolder::Get().GetFilm(walkLeft.GetId());
				GetSelf()->SetCurrFilm(film);
				animator.Start(self, &walkLeft, SystemClock::Get().micro_secs());
			}
			else
				assert(0);
		}
		);
	}
	
	void SetWalkLeft(FrameRangeAnimation animation) {
		this->walkLeft = animation;
	}
	
	void SetWalkRight(FrameRangeAnimation animation) {
		this->walkRight = animation;
	}
	
	void StartMoving() {
		movement.Start();
		animator.Start(self, &walkRight, SystemClock::Get().micro_secs());
		self->Move(0, 0);
	}
	
	void StopMoving() {
		movement.Stop();
		animator.Stop();
	}
	
	void SetEdgeDetection(bool edgeDetection) {
		movement.SetEdgeDetection(edgeDetection);
	}

protected:
	MovementAI			movement;
	FrameRangeAnimation walkLeft;
	FrameRangeAnimation walkRight;
	FrameRangeAnimation death;
	FrameRangeAnimator	animator;
	GridLayer			*myGrid;

	void OnStartFalling() {
		gravityModule.SetIsFalling(true);
		gravityModule.StartFalling();
	}
	void OnStopFalling() {
		gravityModule.SetIsFalling(false);
		gravityModule.StopFalling();
	}
};

std::function<void()> Prepare_MovingEntityOnDeath(MovingEntity *entity, FrameRangeAnimation *deathAnimation) {
	return [entity, deathAnimation]() {
		entity->StopMoving();
		CollisionChecker::GetSingleton().Cancel(entity->GetSelf());
		FrameRangeAnimator	*deathAnimator = new FrameRangeAnimator();
		deathAnimator->SetOnFinish(
			[entity](Animator *animator) {
				entity->GetSelf()->SetVisibility(false);
				SpriteManager::GetSingleton().Remove(entity->GetSelf());
			}
		);
		entity->GetSelf()->SetCurrFilm(AnimationFilmHolder::Get().GetFilm(deathAnimation->GetId()));
		deathAnimator->Start(entity->GetSelf(), deathAnimation, SystemClock::Get().micro_secs());
	};
}
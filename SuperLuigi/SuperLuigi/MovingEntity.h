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
	
	void SetSpeed(int speed) {
		movement.SetSpeed(speed);
	}

	void SetEdgeDetection(bool edgeDetection) {
		movement.SetEdgeDetection(edgeDetection);
	}

	void SetSign(int sign) {
		movement.SetSign(sign);
	}

	//Those are implemented bellow the Primitive holder code for linking purposes
	std::function<void()> Prepare_DefaultOnDeath(MovingEntity *entity);
	std::function<void()> Prepare_DefaultOnDeath(MovingEntity *entity, FrameRangeAnimation *deathAnimation);

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

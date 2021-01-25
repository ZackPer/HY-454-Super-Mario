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
		InitGravity(myGrid);
	}
	
	void SetWalkLeft(FrameRangeAnimation animation) {
		this->walkLeft = animation;
	}
	void SetWalkRight(FrameRangeAnimation animation) {
		this->walkRight = animation;
	}
	void SetDeath(FrameRangeAnimation animation) {
		this->death = animation;
	}

	void Start() {
		movement = MovementAI(self, 1);
		movement.SetEdgeDetection(true);
		movement.Init(myGrid);
		self->Move(0, 0);
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
		animator.Start(self, &walkRight, SystemClock::Get().micro_secs());
	}

	SpriteEntity *Clone(int x, int y) {
		MovingEntity *clone = new MovingEntity(x, y, self->GetCurrFilm(), self->GetTypeId(), myGrid);
		return clone;
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
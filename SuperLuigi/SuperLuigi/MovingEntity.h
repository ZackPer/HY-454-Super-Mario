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

		animation = FrameRangeAnimation(self->GetCurrFilm()->GetId(), 0, 2, 0, 0, 0, 500000);
		animator.Start(self, &animation, SystemClock::Get().micro_secs());

		movement = MovementAI(self, 1);
		movement.SetEdgeDetection(true);
		movement.Init(myGrid);
		self->Move(0, 0);
	}

	SpriteEntity *Clone(int x, int y) {
		MovingEntity *clone = new MovingEntity(x, y, self->GetCurrFilm(), self->GetTypeId(), myGrid);
		return clone;
	}

protected:
	MovementAI			movement;
	FrameRangeAnimation animation;
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
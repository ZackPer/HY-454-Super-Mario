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
		InitGravity(myGrid);

		animation = new FrameRangeAnimation(self->GetCurrFilm()->GetId(), 0, 2, 0, 0, 0, 500000);
		animator = new FrameRangeAnimator();
		animator->Start(self, animation, SystemClock::Get().micro_secs());

		movement = MovementAI(self, 1);
		movement.SetEdgeDetection(true);
		movement.Init(myGrid);
		self->Move(0, 0);
		self->SetBoundingArea();
	}

protected:
	MovementAI			movement;
	FrameRangeAnimation *animation;
	FrameRangeAnimator	*animator;

	void OnStartFalling() {
		gravityModule.SetIsFalling(true);
		gravityModule.StartFalling();
	}
	void OnStopFalling() {
		gravityModule.SetIsFalling(false);
		gravityModule.StopFalling();
	}
};
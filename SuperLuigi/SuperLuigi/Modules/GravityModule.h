#pragma once

#include <iostream>
#include <math.h>
#include <cstdint>
#include "../Engine/SystemClock.h"
#include "../Engine/Sprite/Sprite.h"
#include "../Engine/Animations/Animations.h"
#include "../Engine/Animations/Animators.h"

#include "../AccelaratedMovement.h"

class GravityModule {
public:
	GravityModule() = default;
	GravityModule(Sprite *self) {
		this->self = self;
		animation = new FrameRangeAnimation("", 0, 0, 0, 0, 0, 10000);
		animator = new  FrameRangeAnimator();
	}
	
	// Initialize Animators
	void Init() {
		assert(animator && animation);
		animator->SetOnAction(
			[=](Animator *animator, const Animation& animation) {
				int dh = physics.FindCurrentOffset(SystemClock::Get().micro_secs());
				if (dh + (startPos - self->GetBox().y) > 0)
					self->Move(0, dh + (startPos - self->GetBox().y));
			}
		);
		
		animator->SetOnFinish(
			[=](Animator *animator) {

			}
		);
	}

	// Prepare Physics and start falling
	void StartFalling() {
		assert(animator && animation);
		PreparePhysics();
		animator->Start(self, animation, SystemClock::Get().micro_secs());
	}

	// StopMoving the falling animation.
	void StopFalling() {
		assert(animator && animation);
		animator->Stop();
	}

	// Util
	void SetIsFalling(bool isFalling) {
		this->isFalling = isFalling;
	}

	bool GetIsFalling() {
		return isFalling;
	}

	bool *GetIsFallingRef() {
		return &isFalling;
	}

private: 
	Sprite				*self;
	AccelaratedMovement physics;
	FrameRangeAnimation *animation;
	FrameRangeAnimator	*animator;
	uint64_t			animDelay;
	int					startPos;
	bool				isFalling;

	// For physics for freefall.
	void PreparePhysics() {
		physics = AccelaratedMovement();
		physics.PrepareAccelarationPhysics(SystemClock::Get().micro_secs());
		startPos = self->GetBox().y;
	}

};
#pragma once

#include <iostream>
#include <math.h>
#include <cstdint>
#include "../Engine/SystemClock.h"
#include "../Engine/Sprite/Sprite.h"
#include "../Engine/Animations/Animations.h"
#include "../Engine/Animations/Animators.h"

#include "../AccelaratedMovement.h"

class JumpModule {
public:
	JumpModule() = default;
	JumpModule(Sprite *self) {
		this->self = self;
		animation = new FrameRangeAnimation("littlemario.jump.right", 0, 0, 0, 0, 0, 10000);
		jumpAnimator = new  FrameRangeAnimator();
	}

	// Initialize Animators
	void Init() {
		assert(jumpAnimator && animation);

		//Basic JumpAnimator;
		jumpAnimator->SetOnStart(
			[=](Animator *) {
				self->gravity.SetGravityAddicted(false);
				isJumping = true;
				*isFalling = true;
			}
		);
		jumpAnimator->SetOnAction(
			[=](Animator *animator, const Animation& animation) {
				int dh = physics.FindCurrentOffset(SystemClock::Get().micro_secs());	
				if (dh + (self->GetBox().y - startPos) > 0) {
					self->Move(0, -(dh + (self->GetBox().y - startPos)));
				}
			}
		);
		jumpAnimator->SetOnFinish(
			[=](Animator *animator) {
				self->gravity.SetGravityAddicted(true);
			}
		);
	}

	// Normal Jump, it can reach as height is height given
	void Jump() {
		assert(jumpAnimator && animation);
		isJumping = true;
		PrepareJumpPhysics(height, jumpDuration);
		jumpAnimator->Start(self, animation, physics.startTime);
	}

	// From the moment it is being called it jumps 16 pixels heighter and then renables gravity
	void ForceEndJump() {
		assert(jumpAnimator && animation);
		auto currTime = SystemClock::Get().micro_secs();
		auto dh = physics.FindCurrentOffset(currTime);

		if (isJumping &&
			!isStopped &&
			physics.endTime > currTime &&
			dh < height - 16)
		{
			jumpAnimator->Stop();
			isStopped = true;
			PrepareJumpPhysics(16, forceStopDuration);
			jumpAnimator->Start(self, animation, physics.startTime);
		}
	}

	// Enable next jump
	void Reset() {
		isJumping = false;
		isStopped = false;
	}

	void SetIsFalling(bool *isFalling) {
		this->isFalling = isFalling;
	}
	bool IsJumping() {
		return isJumping;
	}

private:
	Sprite				*self;
	FrameRangeAnimation *animation;
	FrameRangeAnimator	*jumpAnimator;
	bool				*isFalling;
	bool				isJumping = false;
	bool				isStopped = false;
	int					height = 80;
	int					animRepeats;
	int					startPos;
	uint64_t			animDelay = 10000;
	uint64_t			jumpDuration = (float)pow(10, 6) * 0.7;
	uint64_t			forceStopDuration = pow(10, 6) * 0.3;
	AccelaratedMovement physics;

	// For each kind of jump, prepare physics and animation delays.
	void PrepareJumpPhysics(int height, uint64_t duration) {
		physics = AccelaratedMovement(height, duration, physics.NEGATIVE);
		physics.PrepareDecelarationPhysics(SystemClock::Get().micro_secs());
		startPos = self->GetBox().y;
		animRepeats = duration / animDelay;
		animation->SetDelay(animDelay);
		animation->SetReps(animRepeats);
	}

};
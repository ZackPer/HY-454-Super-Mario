#pragma once
#include "Engine/Animations/AnimationFilm.h"
#include "Engine/Animations/Animations.h"
#include "Engine/Animations/Animators.h"

enum MovingState {
	NOT_MOVING = 0,
	MOVING_RIGHT = 1,
	MOVING_LEFT = 2
};

enum AnimationState {
	WALKING = 1,
	JUMPING = 2,
	GROWING = 3,
	DYING = 4
};

enum Direction {
	RIGHT = 0,
	LEFT = 1,
	UP = 2,
	DOWN = 3,
	NO = 4
};

class MarioMover {
	FrameRangeAnimator	FramesAnimator;
	MovingAnimator		MoveAnimator;
	AnimationFilm*		walkRightFilm = AnimationFilmHolder::Get().GetFilm("littlemario.walk.right");
	AnimationFilm*		walkLeftFilm = AnimationFilmHolder::Get().GetFilm("littlemario.walk.left");

	FrameRangeAnimation RightWalkFramesAnimation;
	MovingAnimation		RightWalkMoveAnimation;

	FrameRangeAnimation LeftWalkFramesAnimation;
	MovingAnimation		LeftWalkMoveAnimation;

	MovingAnimator		RemainingMoveAnimator;
	MovingAnimation		RemainingRightMoveAnimation;
	MovingAnimation		RemainingLeftMoveAnimation;
						
	FrameRangeAnimator	RemainingMoveFramesAnimator;
	FrameRangeAnimation	RemainingMoveRightFrameAnimation;
	FrameRangeAnimation	RemainingMoveLeftFrameAnimation;

	MovingState			movingState;
	AnimationState		animationState;

	MovingAnimation		RightJumpMoveAnimation;
	MovingAnimation		LeftJumpMoveAnimation;

	Sprite* mario;
	float acceleration = 1;
	bool super = false;
public:
	MarioMover() {}

	MarioMover(Sprite* s) {
		mario = s;
		movingState = NOT_MOVING;

		MoveAnimator.SetOnAction(
			[=](Animator* animator, const Animation& anim) {
				mario->Move(((const MovingAnimation&)anim).GetDx() * acceleration, ((const MovingAnimation&)anim).GetDy() * acceleration);
			}
		);

		//Move Mario Animations & Animators
		RightWalkMoveAnimation = MovingAnimation(
			walkRightFilm->GetId(),
			0,
			2,
			0,
			25000
		);
		
		LeftWalkMoveAnimation = MovingAnimation(
			walkLeftFilm->GetId(),
			0,
			-2,
			0,
			25000
		);
		
		//Frame Mario Animations & Animators
		FramesAnimator.SetOnAction(
			[=](Animator* animator, const Animation& anim) {
				FramesAnimator.FrameRange_Action(mario, animator, (const FrameRangeAnimation&)anim);
			}
		);

		LeftWalkFramesAnimation = FrameRangeAnimation(
			walkLeftFilm->GetId(),
			1,
			walkLeftFilm->GetTotalFrames(),
			0,
			0,
			0,
			75000
		);

		RightWalkFramesAnimation = FrameRangeAnimation(
			walkRightFilm->GetId(),
			1,
			walkRightFilm->GetTotalFrames(),
			0,
			0,
			0,
			75000
		);
		
		//Remaining Move Animations
		RemainingMoveAnimator.SetOnAction(
			[=](Animator* animator, const Animation& anim) {
				mario->Move(((const MovingAnimation&)anim).GetDx() * acceleration, ((const MovingAnimation&)anim).GetDy() * acceleration);
			}
		);

		RemainingRightMoveAnimation = MovingAnimation(
			walkRightFilm->GetId(),
			2,
			3,
			0,
			25000
		);

		RemainingLeftMoveAnimation = MovingAnimation(
			walkLeftFilm->GetId(),
			2,
			-3,
			0,
			25000
		);

		//Remaining Frame Animations
		RemainingMoveFramesAnimator.SetOnAction(
			[=](Animator* animator, const Animation& anim) {
				FramesAnimator.FrameRange_Action(mario, animator, (const FrameRangeAnimation&)anim);
			}
		);

		RemainingMoveLeftFrameAnimation = FrameRangeAnimation(
			walkLeftFilm->GetId(),
			0,
			walkLeftFilm->GetTotalFrames(),
			2,
			-2,
			0,
			20000
		);

		RemainingMoveRightFrameAnimation = FrameRangeAnimation(
			walkRightFilm->GetId(),
			0,
			walkRightFilm->GetTotalFrames(),
			2,
			2,
			0,
			20000
		);

		//setting jump moving animations
		LeftJumpMoveAnimation = MovingAnimation(
			"left.jump",
			0,
			-2,
			0,
			16666
		);
		RightJumpMoveAnimation = MovingAnimation(
			"right.jump",
			0,
			2,
			0,
			16666
		);
		
	}

	void SetWalkFilms(AnimationFilm* right, AnimationFilm* left) {
		if(right)
			walkRightFilm = right;
		if(left)
			walkLeftFilm  = left;
	}

	AnimationFilm* GetwalkRightFilm() const {
		return walkRightFilm;
	}

	AnimationFilm* GetwalkLeftFilm() const {
		return walkRightFilm;
	}

	void ApplyAcceleration(bool isRunning) {
		if (isRunning) {
			if (acceleration <= 2)
				acceleration += 0.03;
		}
		else
			acceleration = 1;
	}

	void StopAllAnimators() {		
		MoveAnimator.Stop();
		FramesAnimator.Stop();
		RemainingMoveFramesAnimator.Stop();
		RemainingMoveAnimator.Stop();
	}

	void Move(Direction dir, bool isRunning, bool super, Direction& looking, AnimationState animationState) {
		ApplyAcceleration(isRunning);

		switch (dir) {
		case RIGHT:
			//stop all other animations			
			mario->setCurrFilm(walkRightFilm);
			if (movingState != MOVING_RIGHT) {
				acceleration = 1;
				MoveAnimator.Start(&RightWalkMoveAnimation, SystemClock::Get().micro_secs());
				FramesAnimator.Start(mario, &RightWalkFramesAnimation, SystemClock::Get().micro_secs());
			}
			movingState = MOVING_RIGHT;
			looking = RIGHT;
			break;
		case LEFT:
			//stop all other animations
			mario->setCurrFilm(walkLeftFilm);
			if (movingState != MOVING_LEFT) {
				acceleration = 1;
				MoveAnimator.Start(&LeftWalkMoveAnimation, SystemClock::Get().micro_secs());
				FramesAnimator.Start(mario, &LeftWalkFramesAnimation, SystemClock::Get().micro_secs());
			}
			movingState = MOVING_LEFT;
			looking = LEFT;
			break;
		default:
			if (movingState == MOVING_RIGHT && isRunning) {
				RemainingMoveFramesAnimator.Start(mario, &RemainingMoveRightFrameAnimation, SystemClock::Get().micro_secs());
			}
			else if (movingState == MOVING_LEFT && isRunning) {
				RemainingMoveFramesAnimator.Start(mario, &RemainingMoveLeftFrameAnimation, SystemClock::Get().micro_secs());
			}
			acceleration = 1;
			movingState = NOT_MOVING;
			MoveAnimator.Stop();
			FramesAnimator.Stop();
			mario->SetFrame(0);
			mario->Move(0, 0);
		}	
	}
};
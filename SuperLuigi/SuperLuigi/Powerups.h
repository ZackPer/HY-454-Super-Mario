#pragma once
#include "Engine/SystemClock.h"
#include "Engine/Animations/AnimationFilm.h"
#include "Engine/Animations/Animations.h"
#include "Engine/Animations/Animators.h"
class GrowerAndShrinker {
private:
	Sprite* mario;

	AnimationFilm* rightGrowFilm	= AnimationFilmHolder::Get().GetFilm("mario.grow.right");
	//AnimationFilm* leftGrowFilm	= AnimationFilmHolder::Get().GetFilm("mario.grow.left");

	//AnimationFilm* rightShrinkFilm = AnimationFilmHolder::Get().GetFilm("mario.shrink.right");
	//AnimationFilm* leftShrinkFilm	 = AnimationFilmHolder::Get().GetFilm("mario.shrink.left");


	FrameRangeAnimation RightGrowAnimation;
	FrameRangeAnimation LeftGrowAnimation;

	FrameRangeAnimator	frameAnimator;
public:
	GrowerAndShrinker() {}

	GrowerAndShrinker(Sprite* s) {
		mario = s;
		RightGrowAnimation = FrameRangeAnimation(
			rightGrowFilm->GetId(),
			0,
			rightGrowFilm->GetTotalFrames(),
			2,
			0,
			0,
			50000
		);

		frameAnimator.SetOnAction(
			[=](Animator* animator, const Animation& anim) {
				frameAnimator.FrameRange_Action(mario, animator, (const FrameRangeAnimation&)anim);
			}
		);
	}

	void Grow() {
		mario->setCurrFilm(rightGrowFilm);
		frameAnimator.Start(mario, &RightGrowAnimation, SystemClock::Get().micro_secs());
	}

	void Shrink() {

	}

};

class SuperMushroom {
	
};
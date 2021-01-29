#pragma once
#include "Mario.h"

class GrowerAndShrinker {
private:
	Sprite* mario;

	AnimationFilm* rightGrowFilm	= AnimationFilmHolder::Get().GetFilm("mario.grow.right");
	AnimationFilm* leftGrowFilm	= AnimationFilmHolder::Get().GetFilm("mario.grow.left");

	AnimationFilm* rightShrinkFilm = AnimationFilmHolder::Get().GetFilm("mario.shrink.right");
	AnimationFilm* leftShrinkFilm	 = AnimationFilmHolder::Get().GetFilm("mario.shrink.left");


	FrameRangeAnimation rightGrowAnimation;
	FrameRangeAnimation rightShrinkAnimation;

	FrameRangeAnimation leftGrowAnimation;
	FrameRangeAnimation leftShrinkAnimation;

	FrameRangeAnimator	frameAnimator;
public:
	GrowerAndShrinker() {}

	GrowerAndShrinker(Sprite* s) {
		mario = s;
		rightGrowAnimation = FrameRangeAnimation(
			rightGrowFilm->GetId(),
			0,
			rightGrowFilm->GetTotalFrames(),
			4,
			0,
			0,
			150000
		);

		rightShrinkAnimation = FrameRangeAnimation(
			rightShrinkFilm->GetId(),
			0,
			rightShrinkFilm->GetTotalFrames(),
			4,
			0,
			0,
			150000
		);

		leftGrowAnimation = FrameRangeAnimation(
			leftGrowFilm->GetId(),
			0,
			leftGrowFilm->GetTotalFrames(),
			4,
			0,
			0,
			150000
		);

		leftShrinkAnimation = FrameRangeAnimation(
			leftShrinkFilm->GetId(),
			0,
			leftShrinkFilm->GetTotalFrames(),
			4,
			0,
			0,
			150000
		);


		frameAnimator.SetOnAction(
			[=](Animator* animator, const Animation& anim) {
				frameAnimator.FrameRange_Action(mario, animator, (const FrameRangeAnimation&)anim);
			}
		);
	}

	void Grow(Direction dir) {
		if (dir == RIGHT) {
			mario->setCurrFilm(rightGrowFilm);
			mario->SetFrame(1);
			mario->SetFrame(0);
			mario->SetPos(mario->GetBox().x, mario->GetBox().y - 16);
			frameAnimator.Start(mario, &rightGrowAnimation, SystemClock::Get().micro_secs());
		}
		else {
			mario->setCurrFilm(leftGrowFilm);
			mario->SetFrame(1);
			mario->SetFrame(0);
			mario->SetPos(mario->GetBox().x, mario->GetBox().y - 16);
			frameAnimator.Start(mario, &leftGrowAnimation, SystemClock::Get().micro_secs());
		}
		
	}

	void Shrink(Direction dir) {
		if (dir == RIGHT) {
			mario->setCurrFilm(rightShrinkFilm);
			mario->SetFrame(1);
			mario->SetFrame(0);
			frameAnimator.Start(mario, &rightShrinkAnimation, SystemClock::Get().micro_secs());
		}
		else {
			mario->setCurrFilm(leftShrinkFilm);
			mario->SetFrame(1);
			mario->SetFrame(0);
			frameAnimator.Start(mario, &leftShrinkAnimation, SystemClock::Get().micro_secs());
		}

	}

	FrameRangeAnimator& GetframeAnimator() {
		return frameAnimator;
	}
};

class SuperMushroom {
	
};

class StarAnimation {
	std::vector<ALLEGRO_COLOR> frameColors, basicColor;
	int frame;
	bool started, changeFrameColor = false;
	uint64_t startTime;

public:
	StarAnimation() {
		frame = 0;
		frameColors.push_back(al_map_rgba_f(1, 1, 1, 1)); 
		frameColors.push_back(al_map_rgba_f(0, 1, 0.5, 0.5));
		frameColors.push_back(al_map_rgba_f(1, 1, 1, 0.5));
		frameColors.push_back(al_map_rgba_f(0.5, 0.5, 0.5, 0.8));
	}

	void StartAnimation(uint64_t time) {
		started = true;
		startTime = time;
	}

	void ChangeSpriteTint(Sprite* s) {			
			s->SetTint(frameColors[frame]);
			frame++;
			if (frame == frameColors.size())
				frame = 0;				
	}

	//If mario has the star, changes tint for the animation
	//returns whether mario has the star or not
	bool Update(uint64_t time, uint64_t X, Sprite* s) {
		if (!started) return false;
		
		changeFrameColor = !changeFrameColor;
				
		if (time - startTime > X) {
			//stop the count
			started = false;	
			//set color to the basic one
			s->SetTint(frameColors[0]);
			return false;
		}
		else if (changeFrameColor) {
			ChangeSpriteTint(s);
		}
		return true;

	}

};
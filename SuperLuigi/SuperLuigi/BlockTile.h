#pragma once
#include "Engine/Animations/AnimationFilm.h"
#include "Engine/Animations/Animations.h"
#include "Engine/Animations/Animators.h"
#include "Engine/Sprite/Sprite.h"
#include "Engine/SystemClock.h"
enum State {
	IDLE = 0,
	BUMPED = 1,
};

using CollisionFunc = std::function<void(Sprite* s1, Sprite* s2)>;

class BlockSprite {
protected:
	Sprite s;
	State state;

	AnimationFilm* currFilm;
	FrameRangeAnimation idleAnimation;

	FrameRangeAnimator	frameAnimator;
	CollisionFunc collided, action;
public:
	BlockSprite() {
		collided = [=](Sprite* mario, Sprite* self) {
			if (state != BUMPED) {
				//if mario hit from the bottom and is near the middle
				if (mario->GetBox().y > self->GetBox().y)
					if (abs(mario->GetBox().x - self->GetBox().x) <= 5) {
						state = BUMPED;
						if (action)
							action(mario, self);
					}
			}
		};
	}
	
	void OnCollision(Sprite* s1, Sprite* s2) {
		if (collided)
			collided(s1, s2);
	}

	auto GetOnCollison() {
		return collided;
	}
};

class MysteryTile: public BlockSprite {	
public:

	MysteryTile() {}
	MysteryTile(GridLayer* myGrid) {
		//initialize default Film, Animation
		currFilm = AnimationFilmHolder::Get().GetFilm("mysterytile.idle");	
		idleAnimation = FrameRangeAnimation(
			currFilm->GetId(),
			0,
			currFilm->GetTotalFrames(),
			0,
			0,
			0,
			450000
		);		

		//initialize sprite
		s = Sprite(45, 80, currFilm, "mysteryTile"); //placeholder
		s.SetMover(s.MakeSpriteGridLayerMover(myGrid, &s));
		s.SetBoundingArea();

		//initialize default Animator
		frameAnimator.SetOnAction(
			[=](Animator* animator, const Animation& anim) {
				frameAnimator.FrameRange_Action(&s, animator, (const FrameRangeAnimation&)anim);
			}
		);
		frameAnimator.Start(&s, &idleAnimation, SystemClock::Get().micro_secs());
		
		//set bumping action
		action = [=] (Sprite* mario, Sprite* self){
			frameAnimator.Stop();
			
		};
	}

	Sprite& GetSprite() {
		return s;
	}
};
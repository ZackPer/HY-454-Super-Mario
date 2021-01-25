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
	Sprite sprite;
	State state;

	AnimationFilm*		currFilm;
	FrameRangeAnimation idleAnimation;

	FrameRangeAnimator	frameAnimator;
	CollisionFunc		collided, action;

	MovingAnimator		hitStartAnimator;
	MovingAnimator		hitEndAnimator;
	MovingAnimation*	bouncingAnimation;

	bool DetectIfHit(int x1, int xSelf, int y1, int ySelf) {
		if (y1 == ySelf)
			if (abs(x1 - xSelf) <= 5) {
				return true;
			}
		return false;
	}
public:
	BlockSprite() {
		collided = [=](Sprite* mario, Sprite* self) {
			if (state != BUMPED) {
				//if mario hit from the bottom and is near the middle
				if ((mario->GetBox().y - mario->GetBox().h) == self->GetBox().y)
					if (abs(mario->GetBox().x - self->GetBox().x) <= 5) {
						state = BUMPED;
						if (action)
							action(mario, self);
					}
			}		
		};

		bouncingAnimation = new MovingAnimation(
			"bouncing.upas",
			4,
			0,
			-1,
			30000
		);

		hitStartAnimator.SetOnAction(
			[=](Animator* animator, const Animation& anim) {
				sprite.Move(((const MovingAnimation&)anim).GetDx(), ((const MovingAnimation&)anim).GetDy());
			}
		);

		hitStartAnimator.SetOnFinish(
			[=](Animator* animator) {
				bouncingAnimation->SetDy(1);
				hitEndAnimator.Start(bouncingAnimation, SystemClock::Get().micro_secs());
			}
		);

		hitEndAnimator.SetOnAction(
			[=](Animator* animator, const Animation& anim) {
				sprite.Move(((const MovingAnimation&)anim).GetDx(), ((const MovingAnimation&)anim).GetDy());
			}
		);

		hitEndAnimator.SetOnFinish(
			[=](Animator* animator) {
				bouncingAnimation->SetDy(-1);
			}
		);
	}
	
	void OnCollision(Sprite* s1, Sprite* s2) {
		if (collided)
			collided(s1, s2);
	}

	auto GetOnCollison() {
		return collided;
	}

	Sprite& GetSprite() {
		return sprite;
	}
};

class MysteryTile: public BlockSprite {	
private:
		Sprite*			containedSprite = nullptr;
		MovingAnimator	containedSpawnAnimator;
		MovingAnimation containedSpawnAnimation;
		AnimationFilm*	containedFilm;
		GridLayer*		myGrid;
		std::string		mysteryID;
		
		void spawnItem(std::string MysteryID, GridLayer* myGrid) {
			//need to free the pointer after spawn entity is created.
			containedFilm = AnimationFilmHolder::Get().GetFilm(MysteryID);
			//Initialize Sprite
			containedSprite = new Sprite(
				sprite.GetBox().x,
				sprite.GetBox().y,
				containedFilm,
				MysteryID + ".sprite"
			);
			containedSprite->SetMover(containedSprite->MakeSpriteGridLayerMover(myGrid, containedSprite));
			containedSprite->SetBoundingArea();
			containedSprite->SetZorder(0);

			//Initialize spawn Animation and spawn Animator
			containedSpawnAnimation = MovingAnimation(
				MysteryID + ".spawnAnimation",
				8,
				0,
				-2,
				75000
			);

			containedSpawnAnimator.SetOnAction(
				[=](Animator* animator, const Animation& anim) {
					containedSprite->Move(((const MovingAnimation&)anim).GetDx(), ((const MovingAnimation&)anim).GetDy());
				}
			);

			//Set on finish to create the entity from the sprite, so when spawn finishes, object is instanciated.
			//containedSpawnAnimator.SetOnFinish();

			//Start Spawn animation.
			containedSpawnAnimator.Start(&containedSpawnAnimation, SystemClock::Get().micro_secs());
		}
public:
	MysteryTile() {}

	MysteryTile(GridLayer* myGrid, std::string id) {
		mysteryID = id;
		this->myGrid = myGrid;
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
		sprite = Sprite(45, 80, currFilm, "mysteryTile"); //placeholder
		sprite.SetMover(sprite.MakeSpriteGridLayerMover(myGrid, &sprite));
		sprite.SetBoundingArea();

		//initialize default frame Animator
		frameAnimator.SetOnAction(
			[=](Animator* animator, const Animation& anim) {
				frameAnimator.FrameRange_Action(&sprite, animator, (const FrameRangeAnimation&)anim);
			}
		);
		frameAnimator.Start(&sprite, &idleAnimation, SystemClock::Get().micro_secs());

		//set bumping action
		action = [=] (Sprite* mario, Sprite* self){
			frameAnimator.Stop();
			sprite.SetCurrFilm(AnimationFilmHolder::Get().GetFilm("mysterytile.bumped"));
			sprite.SetFrame(0);
			hitStartAnimator.Start(bouncingAnimation,SystemClock::Get().micro_secs());
		};

		hitEndAnimator.SetOnFinish(
			[=](Animator* animator) {
				bouncingAnimation->SetDy(-1);
				spawnItem(mysteryID, myGrid);
			}
		);
	}

	Sprite* GetcontainedSprite() {
		return containedSprite;
	}
	
};

class BrickTile : public BlockSprite {
public:
	BrickTile() {}

	BrickTile(GridLayer* myGrid) {
		currFilm = AnimationFilmHolder::Get().GetFilm("bricktile.idle");

		sprite = Sprite(61, 80, currFilm, "bricktile"); //placeholder
		sprite.SetMover(sprite.MakeSpriteGridLayerMover(myGrid, &sprite));
		sprite.SetBoundingArea();

		hitEndAnimator.SetOnFinish(
			[=](Animator* animator) {
				bouncingAnimation->SetDy(-1);
				state = IDLE;
			}
		);

		action = [=](Sprite* mario, Sprite* self) {
			hitStartAnimator.Start(bouncingAnimation, SystemClock::Get().micro_secs());
		};

	}
};
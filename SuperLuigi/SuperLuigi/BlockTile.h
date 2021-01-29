#pragma once
#include "Engine/Animations/AnimationFilm.h"
#include "Engine/Animations/Animations.h"
#include "Engine/Animations/Animators.h"
#include "Engine/Sprite/Sprite.h"
#include "Engine/SystemClock.h"
#include "PrimitiveHolder.h"

enum State {
	IDLE = 0,
	BUMPED = 1,
};

using CollisionFunc = std::function<void(Sprite* s1, Sprite* s2)>;

class BlockSprite {
protected:
	Sprite *sprite;
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
			if (abs(x1 - xSelf) == 8) {
				return true;
			}
		return false;
	}
public:
	BlockSprite() {
		collided = [=](Sprite* mario, Sprite* self) {
			if (state != BUMPED) {
				//if mario hit from the bottom and is near the middle
				if (mario->GetBox().y == (self->GetBox().y + self->GetBox().h))
					if (abs((mario->GetBox().x + mario->GetBox().w / 2 ) - (self->GetBox().x + self->GetBox().w / 2 )) <= 8) {
						state = BUMPED;
						if (action)
							action(mario, self);
					}
			}		
		};

		bouncingAnimation = new MovingAnimation(
			"bouncing.up",
			4,
			0,
			-1,
			30000
		);

		hitStartAnimator.SetOnAction(
			[=](Animator* animator, const Animation& anim) {
				sprite->Move(((const MovingAnimation&)anim).GetDx(), ((const MovingAnimation&)anim).GetDy());
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
				sprite->Move(((const MovingAnimation&)anim).GetDx(), ((const MovingAnimation&)anim).GetDy());
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

	Sprite *GetSprite() {
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
				sprite->GetBox().x,
				sprite->GetBox().y,
				containedFilm,
				MysteryID
			);
			containedSprite->SetMover(containedSprite->MakeSpriteGridLayerMover(myGrid, containedSprite));
			containedSprite->SetBoundingArea();
			containedSprite->SetZorder(0);
			containedSprite->SetVisibility(true);

			//Initialize spawn Animation and spawn Animator
			if (MysteryID == "star") {
				containedSpawnAnimation = MovingAnimation(
					MysteryID + ".spawnAnimation",
					6,
					0,
					-3,
					75000
				);
			}
			else {
				containedSpawnAnimation = MovingAnimation(
					MysteryID + ".spawnAnimation",
					6,
					0,
					-3,
					75000
				);
			}
				

			containedSpawnAnimator.SetOnAction(
				[=](Animator* animator, const Animation& anim) {
					containedSprite->Move(((const MovingAnimation&)anim).GetDx(), ((const MovingAnimation&)anim).GetDy());
				}
			);

			if (MysteryID == "mushroom") {
				containedSpawnAnimator.SetOnFinish(
					[=](Animator* a) {
						PrimitiveHolder::Get().CreateSuperMushroom(containedSprite->GetBox().x, containedSprite->GetBox().y, containedSprite);
					}
				);
			}
			else if(MysteryID == "star") {
				containedSpawnAnimator.SetOnFinish(
					[=](Animator* a) {
						PrimitiveHolder::Get().CreateStar(containedSprite->GetBox().x, containedSprite->GetBox().y, containedSprite);
					}
				);
			}
				

			//Start Spawn animation.
			containedSpawnAnimator.Start(&containedSpawnAnimation, SystemClock::Get().micro_secs());
		}
public:
	MysteryTile() {}

	MysteryTile(int x, int y, GridLayer* myGrid, std::string id) {
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
		sprite = new Sprite(x, y, currFilm, "mysteryTile", 5);
		sprite->SetMover(sprite->MakeSpriteGridLayerMover(myGrid, sprite));
		sprite->SetBoundingArea();

		//initialize default frame Animator
		frameAnimator.SetOnAction(
			[=](Animator* animator, const Animation& anim) {
				frameAnimator.FrameRange_Action(sprite, animator, (const FrameRangeAnimation&)anim);
			}
		);
		frameAnimator.Start(sprite, &idleAnimation, SystemClock::Get().micro_secs());

		//set bumping action
		action = [=] (Sprite* mario, Sprite* self){
			frameAnimator.Stop();
			sprite->SetCurrFilm(AnimationFilmHolder::Get().GetFilm("mysterytile.bumped"));
			sprite->SetFrame(0);
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
private:
	bool HitEntityFromBelow(Rect self, Rect entity) {
		if (entity.y + entity.h == self.y) {
			if (entity.x <= self.x + self.w && self.x + self.w <= entity.x + entity.w)
				return true;
		}
		return false;
	}

	//for destruction
	Sprite*	topRightSprite;
	Sprite*	topLeftSprite;
	Sprite*	bottomRightSprite;
	Sprite*	bottomLeftSprite;

	FrameRangeAnimator	topRightFramesAnimator;
	FrameRangeAnimator	topLeftFramesAnimator;
	FrameRangeAnimator	bottomRightFramesAnimator;
	FrameRangeAnimator	bottomLeftFramesAnimator;

	FrameRangeAnimation	topRightFramesAnimation;
	FrameRangeAnimation	topLeftFramesAnimation;
	FrameRangeAnimation	bottomRightFramesAnimation;
	FrameRangeAnimation	bottomLeftFramesAnimation;

	void SetOnBreakAnimatorFinish(FrameRangeAnimator* animator, FrameRangeAnimation* animation, Sprite* s, int dx, int dy, int reps) {
		animator->SetOnFinish(
			[=](Animator* anim) {
				animation->SetDy(dy);
				animation->SetDx(dx);
				animation->SetReps(reps);
				animator->Start(s, animation, SystemClock::Get().micro_secs());
			}
		);
	}

	void InitializePiecesSprites() {
		topLeftSprite = new Sprite(
			sprite->GetBox().x, 
			sprite->GetBox().y, 
			AnimationFilmHolder::Get().GetFilm("brokenbrick.topleft"), 
			"brick.piece", 
			4
		);
		topRightSprite = new Sprite(
			sprite->GetBox().x + 8, 
			sprite->GetBox().y, 
			AnimationFilmHolder::Get().GetFilm("brokenbrick.topright"), 
			"brick.piece", 
			4
		);
		bottomLeftSprite = new Sprite(
			sprite->GetBox().x, 
			sprite->GetBox().y + 8, 
			AnimationFilmHolder::Get().GetFilm("brokenbrick.bottomleft"), 
			"brick.piece", 
			4
		);
		bottomRightSprite = new Sprite(
			sprite->GetBox().x + 8, 
			sprite->GetBox().y + 8, 
			AnimationFilmHolder::Get().GetFilm("brokenbrick.bottomright"), 
			"brick.piece", 
			4
		);
	} 

	void InitializePiecesFrameAnimations() {
		topRightFramesAnimation = FrameRangeAnimation(
			"brick.destroy.topRight",
			0,
			4,
			1,
			4,
			-8,
			50000
		);
		topLeftFramesAnimation = FrameRangeAnimation(
			"brick.destroy.topleft",
			0,
			4,
			1,
			-4,
			-8,
			50000
		);
	
		bottomRightFramesAnimation = FrameRangeAnimation(
			"brick.destroy.topRight",
			0,
			4,
			1,
			4,
			-8,
			50000
		);
		bottomLeftFramesAnimation = FrameRangeAnimation(
			"brick.destroy.topRight",
			0,
			4,
			1,
			-4,
			-8,
			50000
		);		

		SetOnBreakAnimatorFinish(&topRightFramesAnimator, &topRightFramesAnimation, topRightSprite, 0, 8, 8);
		SetOnBreakAnimatorFinish(&bottomRightFramesAnimator, &bottomRightFramesAnimation, bottomRightSprite, 0, 12, 8);
		SetOnBreakAnimatorFinish(&topLeftFramesAnimator, &topLeftFramesAnimation, topLeftSprite, 0, 8, 8);
		SetOnBreakAnimatorFinish(&bottomLeftFramesAnimator, &bottomLeftFramesAnimation, bottomLeftSprite, 0, 12, 8);

		topRightFramesAnimator.SetOnAction(
			[=](Animator* animator, const Animation& anim) {
				topRightSprite->SetHasDirectMotion(true).Move(((const MovingAnimation&)anim).GetDx(), ((const MovingAnimation&)anim).GetDy()).SetHasDirectMotion(true);
			}
		);

		bottomRightFramesAnimator.SetOnAction(
			[=](Animator* animator, const Animation& anim) {
				bottomRightSprite->SetHasDirectMotion(true).Move(((const MovingAnimation&)anim).GetDx(), ((const MovingAnimation&)anim).GetDy()).SetHasDirectMotion(true);
			}
		);

		topLeftFramesAnimator.SetOnAction(
			[=](Animator* animator, const Animation& anim) {
				topLeftSprite->SetHasDirectMotion(true).Move(((const MovingAnimation&)anim).GetDx(), ((const MovingAnimation&)anim).GetDy()).SetHasDirectMotion(true);
			}
		);

		bottomLeftFramesAnimator.SetOnAction(
			[=](Animator* animator, const Animation& anim) {
				bottomLeftSprite->SetHasDirectMotion(true).Move(((const MovingAnimation&)anim).GetDx(), ((const MovingAnimation&)anim).GetDy()).SetHasDirectMotion(true);
			}
		);

	}

	void StartPiecesAnimators(){
		topRightFramesAnimator.Start(topRightSprite, &topRightFramesAnimation, SystemClock::Get().micro_secs());
		bottomRightFramesAnimator.Start(bottomRightSprite, &bottomRightFramesAnimation, SystemClock::Get().micro_secs());
		topLeftFramesAnimator.Start(topLeftSprite, &topLeftFramesAnimation, SystemClock::Get().micro_secs());
		bottomLeftFramesAnimator.Start(bottomLeftSprite, &bottomLeftFramesAnimation, SystemClock::Get().micro_secs());

		topRightSprite->SetVisibility(true);
		bottomRightSprite->SetVisibility(true);
		topLeftSprite->SetVisibility(true);
		bottomLeftSprite->SetVisibility(true);
	}

public:
	BrickTile() {}

	BrickTile(int x, int y, GridLayer* myGrid) {
		currFilm = AnimationFilmHolder::Get().GetFilm("bricktile.idle");

		sprite = new Sprite(x, y, currFilm, "bricktile", 1); //placeholder
		sprite->SetMover(sprite->MakeSpriteGridLayerMover(myGrid, sprite));
		sprite->SetBoundingArea();

		hitEndAnimator.SetOnFinish(
			[=](Animator* animator) {
				bouncingAnimation->SetDy(-1);
				state = IDLE;
			}
		);

		InitializePiecesSprites();
		InitializePiecesFrameAnimations();

		action = [=](Sprite* mario, Sprite* self) {
			if (mario->GetBox().h == 32) {
				EntitySpawner::Get().MakeBrickTileEmpty(self->GetBox().x, self->GetBox().y, (*myGrid));
				self->SetVisibility(false);
				StartPiecesAnimators();
			}
			else {			
				hitStartAnimator.Start(bouncingAnimation, SystemClock::Get().micro_secs());
			}
			//kill enemies below
			auto enemies = EntityHolder::Get().FindByType("enemy");
			for (auto& it : enemies) {
				if (HitEntityFromBelow(self->GetBox(), it->GetSelf()->GetBox())) {
					DeathJump((MovingEntity*)it);
				}
			}
			//bounce powerups
			auto powerups = EntityHolder::Get().FindByType("powerup");
			for (auto& it : powerups) {
				if (HitEntityFromBelow(self->GetBox(), it->GetSelf()->GetBox())) {
					((MovingEntity*)it)->StartJump(1);
				}
			}
		};



	}
};
#pragma once
#include "Engine/Sprite/Sprite.h"
#include "Engine/Animations/Animations.h"
#include "Engine/Animations/Animators.h"
#include "Engine/SystemClock.h"
#include "Engine/Grid/Grid.h"
#include "Modules/GravityModule.h"
#include "Modules/JumpModule.h"
#include "SpriteEntity.h"
#include "Powerups.h"
#include "Modules/DeathModule.h"

class Mario : public SpriteEntity {
public:
	bool isSuper = false;
	bool isRunning = false;
	AnimationState animationState = WALKING;
	Direction direction = NO, looking = RIGHT;
	Star star;

	Mario() = default;
	Mario(int x, int y, GridLayer *myGrid, Rect *viewWindow) 
		: SpriteEntity(x, y, AnimationFilmHolder::Get().GetFilm("littlemario.walk.right"), "mario")
	{	
		this->myGrid = myGrid;
		this->viewWindow = viewWindow;

		//My module Initialization
		jumpModule = JumpModule(self);
		jumpModule.SetIsFalling(gravityModule.GetIsFallingRef());
		jumpModule.Init();

		// Super's Initializations
		PrepareMoverWithViewWindowCheck(myGrid, viewWindow);
		self->SetRange(1, 1);
		InitGravity(myGrid);
		self->Move(0, 0); //This is to calculate gravity at least once.
		deathModule = new DeathModule(self);
		selfMover = new MarioMover(self);
		growerAndShrinker = new GrowerAndShrinker(self);
	}

	void InputPoll() {
		uint64_t currentTime = SystemClock::Get().micro_secs();
		if (currentTime - inputDelay > lastInput) {
			lastInput = currentTime;
			ALLEGRO_KEYBOARD_STATE keyState;
			al_get_keyboard_state(&keyState);
			if (animationState != DYING && animationState != GROWING) {
				star.EveryXMilliSecs(SystemClock::Get().milli_secs(), 10000, self);
				//jumping
				if (al_key_down(&keyState, ALLEGRO_KEY_UP) &&
					!gravityModule.GetIsFalling() &&
					!jumpModule.IsJumping())
				{
					jumpModule.Jump();
					star.StartTheCount(SystemClock::Get().milli_secs());
				}
				else if (!al_key_down(&keyState, ALLEGRO_KEY_UP) && jumpModule.IsJumping()) {
					jumpModule.ForceEndJump();
				}

				if (jumpModule.IsJumping())
					animationState = JUMPING;
				else
					animationState = WALKING;

				//moving and running
				if (al_key_down(&keyState, ALLEGRO_KEY_RIGHT)) {
					direction = RIGHT;
				}
				else if (al_key_down(&keyState, ALLEGRO_KEY_LEFT)) {
					direction = LEFT;
				}
				if (al_key_down(&keyState, ALLEGRO_KEY_SPACE)) {
					isRunning = true;
				}

				//misc
				if (al_key_down(&keyState, ALLEGRO_KEY_1)) {
					Grow(looking);
				}
				else if (al_key_down(&keyState, ALLEGRO_KEY_2)) {
					Shrink(looking);
				}
			}
			
		}
	}
	
	void PrepareMoverWithViewWindowCheck(GridLayer *myGrid, Rect *viewWindow) {
		auto defaultMover = self->MakeSpriteGridLayerMover(myGrid, self);
		auto viewWindowBoundriesCheck = [=](Rect& box, int *dx, int *dy) {
			auto position = self->GetBox();
			if (position.x + *dx < viewWindow->x) {
				*dx = viewWindow->x - position.x;
			}
			else if (position.x + *dx > viewWindow->x + viewWindow->w) {
				*dx = viewWindow->x + viewWindow->w - position.x;
			}
			defaultMover(box, dx, dy);
		};
		self->SetMover(viewWindowBoundriesCheck);

	}

	void Bounce() {
		
	}

	void Shrink(Direction dir) {
		animationState = GROWING;
		selfMover->StopAllAnimators();
		growerAndShrinker->Shrink(dir);
		growerAndShrinker->GetframeAnimator().SetOnFinish(
			[=](Animator* animator) {
				SuperChangeFilms(false, dir);
				animationState = WALKING;
			}
		);
		self->SetBoundingArea();
	}

	void Grow(Direction dir) {
		animationState = GROWING;
		selfMover->StopAllAnimators();
		growerAndShrinker->Grow(dir);
		growerAndShrinker->GetframeAnimator().SetOnFinish(
			[=](Animator* animator) {
				SuperChangeFilms(true, dir);
				animationState = WALKING;
			}
		);
		self->SetBoundingArea();
	}

	void SuperChangeFilms(bool b, Direction looking) {
		if (b != isSuper) {
			isSuper = b;
			if (isSuper) {
				selfMover->SetWalkFilms(
					AnimationFilmHolder::Get().GetFilm("supermario.walk.right"),
					AnimationFilmHolder::Get().GetFilm("supermario.walk.left")
				);
			}
			else {
				selfMover->SetWalkFilms(
					AnimationFilmHolder::Get().GetFilm("littlemario.walk.right"),
					AnimationFilmHolder::Get().GetFilm("littlemario.walk.left")
				);
			}

			if (looking == RIGHT)
				self->setCurrFilm(selfMover->GetwalkRightFilm());
			else if (looking == LEFT)
				self->setCurrFilm(selfMover->GetwalkLeftFilm());

			if(!isSuper)
				self->SetPos(self->GetBox().x, self->GetBox().y + 16);

			self->SetFrame(1);
			self->SetFrame(0);
		}
	}

	void ChangeInvincible(bool b) {

	}

	bool GetSuper() {
		return isSuper;
	}

	void SetisSuper(bool b) {
		isSuper = b;
	}

	void Die() {
		animationState = DYING;
		selfMover->StopAllAnimators();
		jumpModule.Reset();
		deathModule->Die();
	}

	MarioMover* GetselfMover(){
		return selfMover;
	}

protected:
	GridLayer			*myGrid;
	Rect				*viewWindow;
	JumpModule			jumpModule;
	DeathModule*		deathModule;
	int					dx;
	int					dy;
	uint64_t			lastInput = 0;
	uint64_t			inputDelay = 8000;
	MarioMover*			selfMover;
	GrowerAndShrinker*	growerAndShrinker;

	void OnStartFalling() {
		gravityModule.SetIsFalling(true);
		gravityModule.StartFalling();
	}
	void OnStopFalling() {
		gravityModule.SetIsFalling(false);
		gravityModule.StopFalling();
		jumpModule.Reset();
	}
};
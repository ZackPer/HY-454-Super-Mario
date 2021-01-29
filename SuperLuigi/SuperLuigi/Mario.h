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

		marioJumpingLeft = FrameRangeAnimation("littlemario.jump.left", 0, 2, 0, 0, 0, 100000);
		marioJumpingRight = FrameRangeAnimation("littlemario.jump.right", 0, 2, 0, 0, 0, 100000);
		supermarioJumpingLeft = FrameRangeAnimation("supermario.jump.left", 0, 2, 0, 0, 0, 100000);
		supermarioJumpingRight = FrameRangeAnimation("supermario.jump.right", 0, 2, 0, 0, 0, 100000);

		//My module Initialization
		jumpModule = JumpModule(self);
		jumpModule.SetIsFalling(gravityModule.GetIsFallingRef());
		jumpModule.Init();

		deathModule = new DeathModule(self);
		selfMover = new MarioMover(self);
		growerAndShrinker = new GrowerAndShrinker(self);

		// Super's Initializations
		PrepareMoverWithViewWindowCheck(myGrid, viewWindow);
		self->SetRange(1, 1);
		InitGravity(myGrid);
		self->Move(0, 0); //This is to calculate gravity at least once.

		InputEnabled = true;
	}

	//This shouldnt be here but last day whatever 
	void JumpAnimation() {
		animationState = JUMPING;
		selfMover->StopFrameAnimator();
		if (looking == RIGHT) {
			if (!isSuper) {
				self->SetCurrFilm(AnimationFilmHolder::Get().GetFilm(marioJumpingRight.GetId()));
				jumpingFrameAnimator.Start(self, &marioJumpingRight, SystemClock::Get().micro_secs());
			}
			else {
				self->SetCurrFilm(AnimationFilmHolder::Get().GetFilm(supermarioJumpingRight.GetId()));
				jumpingFrameAnimator.Start(self, &supermarioJumpingRight, SystemClock::Get().micro_secs());
			}
		}
		else if (looking == LEFT) {
			if (!isSuper) {
				self->SetCurrFilm(AnimationFilmHolder::Get().GetFilm(marioJumpingLeft.GetId()));
				jumpingFrameAnimator.Start(self, &marioJumpingLeft, SystemClock::Get().micro_secs());
			}
			else {
				self->SetCurrFilm(AnimationFilmHolder::Get().GetFilm(supermarioJumpingLeft.GetId()));
				jumpingFrameAnimator.Start(self, &supermarioJumpingLeft, SystemClock::Get().micro_secs());
			}
		}
	}
	void StopJumpAnimation() {
		animationState = WALKING;
		direction = looking;
		jumpingFrameAnimator.Stop();
		selfMover->ForceStartFrameAnimator(direction, isRunning, isSuper, looking, animationState);
	}

	void InputPoll() {
		uint64_t currentTime = SystemClock::Get().micro_secs();
		if (currentTime - inputDelay > lastInput && (InputEnabled)) {
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
					JumpAnimation();
					jumpModule.Jump(80, 1000000 * 0.7, false);
				}
				else if (!al_key_down(&keyState, ALLEGRO_KEY_UP) && jumpModule.IsJumping()){
					jumpModule.ForceEndJump();
				}

				if (al_key_down(&keyState, ALLEGRO_KEY_3))
				{
					star.StartTheCount(SystemClock::Get().milli_secs());
				}

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
				else if (al_key_down(&keyState, ALLEGRO_KEY_ESCAPE)) {
					exit(EXIT_SUCCESS);
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
		OnStopFalling();
		self->gravity.Reset();
		jumpModule.Jump(20, 1000000 * 0.2, true);
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
		jumpingFrameAnimator.Stop();
		jumpModule.Reset();
		deathModule->Die();
	}

	MarioMover* GetselfMover(){
		return selfMover;
	}

	void SetInput(bool in) {
		InputEnabled = in;
	}

	bool GetInputEnabled() {
		return InputEnabled;
	}

	int GetPoints() {
		return points;
	}
	void SetPoints(int points){
		this->points = points;
	}
	void AddPoints(int amount) {
		this->points += amount;
	}

protected:
	GridLayer			*myGrid;
	Rect				*viewWindow;
	JumpModule			jumpModule;
	DeathModule			*deathModule;
	int					dx;
	int					dy;
	uint64_t			lastInput = 0;
	uint64_t			inputDelay = 8000;
	MarioMover*			selfMover;
	GrowerAndShrinker*	growerAndShrinker;
	bool				InputEnabled;
	int					points;

	FrameRangeAnimation marioJumpingLeft;
	FrameRangeAnimation marioJumpingRight;
	FrameRangeAnimation supermarioJumpingLeft;
	FrameRangeAnimation supermarioJumpingRight;
	FrameRangeAnimator	jumpingFrameAnimator;

	void OnStartFalling() {
		gravityModule.SetIsFalling(true);
		gravityModule.StartFalling();
	}
	void OnStopFalling() {
		gravityModule.SetIsFalling(false);
		gravityModule.StopFalling();
		jumpModule.Reset();
		StopJumpAnimation();
	}
};

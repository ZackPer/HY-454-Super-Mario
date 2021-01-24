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
class Mario : public SpriteEntity {
public:
	bool isSuper = false;
	bool isRunning = false;
	Direction direction = NO, looking = RIGHT;

	Mario() = default;
	Mario(GridLayer *myGrid, Rect *viewWindow) 
		: SpriteEntity(0, 0, AnimationFilmHolder::Get().GetFilm("littlemario.walk.right"), "mario")
	{	
		//My module Initialization
		jumpModule = JumpModule(self);
		jumpModule.SetIsFalling(gravityModule.GetIsFallingRef());
		jumpModule.Init();

		// Super's Initializations
		PrepareMoverWithViewWindowCheck(myGrid, viewWindow);
		InitGravity(myGrid);
		self->Move(0, 0); //This is to calculate gravity at least once.
		selfMover = new MarioMover(self);
		growerAndShrinker = new GrowerAndShrinker(self);
	}

	void InputPoll() {
		uint64_t currentTime = SystemClock::Get().micro_secs();
		if (currentTime - inputDelay > lastInput) {
			lastInput = currentTime;
			ALLEGRO_KEYBOARD_STATE keyState;
			al_get_keyboard_state(&keyState);
			
			if (al_key_down(&keyState, ALLEGRO_KEY_UP) &&
				!gravityModule.GetIsFalling() &&
				!jumpModule.IsJumping())
			{
				jumpModule.Jump();
			}
			else if (!al_key_down(&keyState, ALLEGRO_KEY_UP) && jumpModule.IsJumping()){
				jumpModule.ForceEndJump();
			}

			al_get_keyboard_state(&keyState);
			//actions on keys
			if (al_key_down(&keyState, ALLEGRO_KEY_DOWN)) {
				direction = DOWN;
			}
			else if (al_key_down(&keyState, ALLEGRO_KEY_UP)) {
				direction = UP;
			}
			if (al_key_down(&keyState, ALLEGRO_KEY_RIGHT)) {
				direction = RIGHT;
			}
			else if (al_key_down(&keyState, ALLEGRO_KEY_LEFT)) {
				direction = LEFT;
			}
			if (al_key_down(&keyState, ALLEGRO_KEY_SPACE)) {
				isRunning = true;
			}
			if (al_key_down(&keyState, ALLEGRO_KEY_1)) {
				ChangeSuper(true, looking);
			}
			if (al_key_down(&keyState, ALLEGRO_KEY_2)) {
				ChangeSuper(false, looking);
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

	void ChangeSuper(bool b, Direction looking) {
		if (b != isSuper) {
			isSuper = b;
			if (isSuper) {
				selfMover->SetWalkFilms(
					AnimationFilmHolder::Get().GetFilm("supermario.walk.right"),
					AnimationFilmHolder::Get().GetFilm("supermario.walk.left")
				);
				self->SetPos(self->GetBox().x, self->GetBox().y - 16);
			}
			else {
				selfMover->SetWalkFilms(
					AnimationFilmHolder::Get().GetFilm("littlemario.walk.right"),
					AnimationFilmHolder::Get().GetFilm("littlemario.walk.left")
				);
				self->SetPos(self->GetBox().x, self->GetBox().y + 16);
			}

			if (looking == RIGHT)
				self->setCurrFilm(selfMover->GetwalkRightFilm());
			else if (looking == LEFT)
				self->setCurrFilm(selfMover->GetwalkLeftFilm());
			self->SetFrame(1);
			self->SetFrame(0);
		}
	}

	bool GetSuper() {
		return isSuper;
	}

	void SetisSuper(bool b) {
		isSuper = b;
	}

	MarioMover* GetselfMover(){
		return selfMover;
	}

protected:
	JumpModule			jumpModule;
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

	void StopMovingAnimations() {

	}

};
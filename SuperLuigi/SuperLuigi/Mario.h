#pragma once
#include "Engine/Sprite/Sprite.h"
#include "Engine/Animations/Animations.h"
#include "Engine/Animations/Animators.h"
#include "Engine/SystemClock.h"
#include "Engine/Grid/Grid.h"

#include "Modules/GravityModule.h"
#include "Modules/JumpModule.h"
#include "SpriteEntity.h"

class Mario : public SpriteEntity {
public:
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
	}

	void InputPoll(Direction& direction, bool& isRunning, bool& super) {
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
				super = true;
			}
			if (al_key_down(&keyState, ALLEGRO_KEY_2)) {
				super = false;
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

protected:
	JumpModule		jumpModule;
	int				dx;
	int				dy;
	uint64_t		lastInput = 0;
	uint64_t		inputDelay = 8000;

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
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
	Mario(GridLayer *myGrid) 
		: SpriteEntity(0, 0, AnimationFilmHolder::Get().GetFilm("littlemario.walk.right"), "mario")
	{
		//My module Initialization
		jumpModule = JumpModule(self);
		jumpModule.SetIsFalling(gravityModule.GetIsFallingRef());
		jumpModule.Init();

		// Super's Initializations
		SetMover(myGrid);
		InitGravity(myGrid);
		self->Move(0, 0); //This is to calculate gravity at least once.
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

			if (al_key_down(&keyState, ALLEGRO_KEY_RIGHT)) {
				dx += speed;
			}
			if (al_key_down(&keyState, ALLEGRO_KEY_LEFT)) {
				dx -= speed;
			}

			self->Move(dx, dy);
			dx = 0;
			dy = 0;
		}
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
#pragma once

#include <iostream>
#include "Engine/Sprite/Sprite.h"
#include "Modules/GravityModule.h"

// Abstract class where all entities will derive from.
class SpriteEntity {
public:
	// Constructors
	SpriteEntity() = default;
	SpriteEntity(int x, int y, AnimationFilm *film, std::string typeId) {
		self = new Sprite(x, y, film, typeId);
		self->SetBoundingArea();
		self->SetRange(1, 1);
	}

	// Sets self mover to be the default mover.
	void SetMover(GridLayer *myGrid) {
		self->SetMover(self->MakeSpriteGridLayerMover(myGrid, self));
		self->SetRange(1, 1);
	}

	// Initializes gravity module and sprite's gravity handler.
	void InitGravity(GridLayer *myGrid) {
		gravityModule = GravityModule(self);
		gravityModule.Init();

		PrepareSpriteGravityHandler(myGrid, self);
		self->GetGravityHandler().SetOnStartFalling(
			[=]() {
				OnStartFalling();
			}
		);
		self->GetGravityHandler().SetOnStopFalling(
			[=]() {
				OnStopFalling();
			}
		);
		
	}

	// Util Functions
	Sprite *GetSelf() {
		return self;
	}

protected:
	std::string		id;
	Sprite			*self;
	int				speed = 2;
	GravityModule	gravityModule;

	// Callback functions for class customization
	virtual void OnStartFalling() = 0;
	virtual void OnStopFalling() = 0;

private:
};
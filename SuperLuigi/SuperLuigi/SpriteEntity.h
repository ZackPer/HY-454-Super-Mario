#pragma once

#include <iostream>
#include "Engine/Sprite/Sprite.h"
#include "Modules/GravityModule.h"

// Abstract class where all entities will derive from.
class SpriteEntity {
public:
	using OnDeath = std::function<void()>;

	// Constructors
	SpriteEntity() = default;
	SpriteEntity(int x, int y, AnimationFilm *film, std::string typeId) {
		self = new Sprite(x, y, film, typeId);
		self->SetBoundingArea();
		self->SetRange(1, 1);
		this->type = typeId;
	}

	SpriteEntity(int x, int y, AnimationFilm* film, std::string typeId, Sprite* s) {
		self = s;
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
			[this]() {
				OnStartFalling();
			}
		);
		self->GetGravityHandler().SetOnStopFalling(
			[this]() {
				OnStopFalling();
			}
		);
	}

	// Util Functions
	Sprite *GetSelf() {
		return self;
	}

	void Die() {
		if (onDeath)
			onDeath();
		else {
			//TODO: Remove later,
			self->SetVisibility(true);
		}
	};

	void SetOnDeath(OnDeath onDeath) {
		this->onDeath = onDeath;

	}

	void SetType(std::string type) {
		this->type = type;
	}
	std::string GetType() {
		return this->type;
	}

	GravityModule& GetgravityModule() {
		return gravityModule;
	}

protected:
	std::string		type;
	Sprite			*self;
	int				speed = 2;
	GravityModule	gravityModule;

	// Callback functions for class customization
	virtual void OnStartFalling() = 0;
	virtual void OnStopFalling() = 0;
	std::function<void()> onDeath;
};
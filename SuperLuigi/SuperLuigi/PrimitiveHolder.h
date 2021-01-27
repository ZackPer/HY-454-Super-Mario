#pragma once

#include <iostream>
#include <functional>
#include "./Engine/Sprite/Sprite.h";
#include "./Engine/Physics/CollisionHander.h";
#include "./Engine/Animations/Animations.h";
#include "./Engine/Animations/Animators.h"
#include "./Engine/Animations/AnimationFilm.h"

#include "EntityHolder.h"
#include "SpriteEntity.h"
#include "MovingEntity.h"
#include "Shell.h"
#include "Mario.h"

// Singleton class that holds all the primitive callback functions for entity creation.
class PrimitiveHolder {
public:
	using PrimitiveCallback = std::function<SpriteEntity *(int x, int y)>;
	using CollisionCallback = std::function<void(Sprite *s1, Sprite *s2)>;

	//Singleton Structure
	static PrimitiveHolder& Get() {
		static PrimitiveHolder singleton;
		return singleton;
	}
	PrimitiveHolder() = default;
	PrimitiveHolder(PrimitiveHolder const&) = delete;
	void operator=(PrimitiveHolder const&) = delete;

	// Primitive creation callbacks
	SpriteEntity *CreateMario(int x, int y) {
		Mario *supermario = new Mario(x, y - 1, myGrid, viewWindow);
		supermario->GetSelf()->SetVisibility(true);
		return supermario;
	}
	SpriteEntity *CreateGoomba(int x, int y) {
		MovingEntity *goomba = new MovingEntity(x, y, AnimationFilmHolder::Get().GetFilm("goomba.walk"), "goomba", myGrid);
		FrameRangeAnimation walk = FrameRangeAnimation("goomba.walk", 0, 2, 0, 0, 0, 300000);
		FrameRangeAnimation *death = new FrameRangeAnimation("goomba.death", 0, 1, 1, 0, 0, 400000);
		goomba->SetWalkLeft(walk);
		goomba->SetWalkRight(walk);
		goomba->SetOnDeath(
			goomba->Prepare_DefaultOnDeath(goomba, death)
		);
		goomba->StartMoving();
		goomba->SetSpeed(1);
		goomba->SetEdgeDetection(false);

		Mario *supermario = EntityHolder::Get().GetSuperMario();
		CollisionChecker::GetSingleton().Register(supermario->GetSelf(), goomba->GetSelf(), PrepareKillBounce(supermario, goomba));

		EntityHolder::Get().Add(goomba);
		return goomba;
	}
	SpriteEntity *CreateGreenKoopa(int x, int y) {
		MovingEntity *greenKoopa = (MovingEntity *)CreateKoopa(x, y, "green");
		greenKoopa->SetEdgeDetection(false);
		return greenKoopa;
	}
	SpriteEntity *CreateRedKoopa(int x, int y) {
		MovingEntity *greenKoopa = (MovingEntity *)CreateKoopa(x, y, "green");
		greenKoopa->SetEdgeDetection(true);
		return greenKoopa;
	}
	SpriteEntity *CreateShell(int x, int y, std::string color) {
		auto film = AnimationFilmHolder::Get().GetFilm(color + ".koopa.shell");
		Shell *shell = new Shell(x, y, color, myGrid);
		EntityHolder::Get().Add((SpriteEntity *)shell);
		return (SpriteEntity*)shell;
	}

	void SetMyGrid(GridLayer *myGrid) {
		this->myGrid = myGrid;
	}
	void SetViewWindow(Rect *viewWindow) {
		this->viewWindow = viewWindow;
	}
private:
	GridLayer	*myGrid;
	Rect		*viewWindow;
	
	SpriteEntity *CreateKoopa(int x, int y, std::string color) {
		MovingEntity *koopa = new MovingEntity(x, y - 8, AnimationFilmHolder::Get().GetFilm(color + ".koopa.walk.left"), color + ".koopa", myGrid);
		FrameRangeAnimation walkLeft = FrameRangeAnimation(color + ".koopa.walk.left", 0, 2, 0, 0, 0, 300000);
		FrameRangeAnimation walkRight = FrameRangeAnimation(color + ".koopa.walk.right", 0, 2, 0, 0, 0, 300000);

		auto defaultDeath = koopa->Prepare_DefaultOnDeath(koopa);
		koopa->SetOnDeath(
			[koopa, defaultDeath, color]() {
			defaultDeath();
			Rect box = koopa->GetSelf()->GetBox();
			PrimitiveHolder::Get().CreateShell(box.x, box.y + 5, color);
		}
		);
		koopa->SetWalkLeft(walkLeft);
		koopa->SetWalkRight(walkRight);
		koopa->StartMoving();
		koopa->SetEdgeDetection(false);
		koopa->GetSelf()->SetVisibility(true);

		auto supermario = EntityHolder::Get().GetSuperMario();
		CollisionChecker::GetSingleton().Register(supermario->GetSelf(), koopa->GetSelf(), PrepareKillBounce(supermario, koopa));

		EntityHolder::Get().Add(koopa);
		return koopa;
	}

	CollisionCallback PrepareKillBounce(Mario *supermario, SpriteEntity *entity) {
		return [entity](Sprite *s1, Sprite *s2) {
			Mario *supermario = EntityHolder::Get().GetSuperMario();
			Rect marioBox = s1->GetBox();
			Rect entityBox = s2->GetBox();
			if (marioBox.y + marioBox.h < entityBox.y + 5) {
				entity->Die();
				supermario->Bounce();
			}
			else {
				supermario->Die();
			}
		};		
	}
};


std::function<void()> MovingEntity::Prepare_DefaultOnDeath(MovingEntity *entity) {
	return [entity]() {
		entity->StopMoving();
		entity->GetSelf()->SetVisibility(false);
		EntityHolder::Get().Remove(entity);
		CollisionChecker::GetSingleton().Cancel(entity->GetSelf());
		SpriteManager::GetSingleton().Remove(entity->GetSelf());
	};
}

std::function<void()> MovingEntity::Prepare_DefaultOnDeath(MovingEntity *entity, FrameRangeAnimation *deathAnimation) {
	return [entity, deathAnimation]() {
		entity->StopMoving();
		EntityHolder::Get().Remove(entity);

		CollisionChecker::GetSingleton().Cancel(entity->GetSelf());
		FrameRangeAnimator	*deathAnimator = new FrameRangeAnimator();
		deathAnimator->SetOnFinish(
			[entity](Animator *animator) {
			entity->GetSelf()->SetVisibility(false);
			SpriteManager::GetSingleton().Remove(entity->GetSelf());
		}
		);
		entity->GetSelf()->SetCurrFilm(AnimationFilmHolder::Get().GetFilm(deathAnimation->GetId()));
		deathAnimator->Start(entity->GetSelf(), deathAnimation, SystemClock::Get().micro_secs());
	};
}

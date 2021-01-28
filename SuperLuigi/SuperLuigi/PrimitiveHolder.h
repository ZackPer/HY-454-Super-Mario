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
#include "SimpleEntity.h"
#include "Shell.h"
#include "Mario.h"
#include "PiranhaPlant.h"

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
		MovingEntity *greenKoopa = (MovingEntity *)CreateKoopa(x, y, "red");
		greenKoopa->SetEdgeDetection(true);
		return greenKoopa;
	}
	SpriteEntity *CreateShell(int x, int y, std::string color) {
		auto film = AnimationFilmHolder::Get().GetFilm(color + ".koopa.shell");
		Shell *shell = new Shell(x, y, color, myGrid);
		EntityHolder::Get().Add((SpriteEntity *)shell);
		return (SpriteEntity*)shell;
	}
	SpriteEntity *CreatePiranhaPlant(int x, int y) {
		PiranhaPlant *piranha = new PiranhaPlant(x, y);
		return (SpriteEntity *)piranha;
	}
	SpriteEntity *CreatePipe(int x, int y) {
		SimpleEntity *pipe = new SimpleEntity(x, y, AnimationFilmHolder::Get().GetFilm("pipe"), "pipe");
		pipe->GetSelf()->SetVisibility(true);
		pipe->GetSelf()->SetZorder(5);
		return (SpriteEntity *)pipe;
	}
	SpriteEntity *CreateTransportPipe1(int x, int y) {
		CreatePipe(x, y);
		auto first_pipe = new SimpleEntity(x, y, AnimationFilmHolder::Get().GetFilm("green.pipe.up"), "transportPipe1");
		EntityHolder::Get().Add(first_pipe);
		first_pipe->GetSelf()->SetVisibility(true);
		//set up collision with our mario
		Mario *supermario = EntityHolder::Get().GetSuperMario();
		std::function<void(Sprite* s1, Sprite* s2)> pipeF = [first_pipe, supermario](Sprite* s1, Sprite* s2) {
			ALLEGRO_KEYBOARD_STATE key;
			al_get_keyboard_state(&key);
			
			auto marioBox = supermario->GetSelf()->GetBox();
			auto pipeBox = first_pipe->GetSelf()->GetBox();
			if (
				al_key_down(&key, ALLEGRO_KEY_DOWN) && supermario->GetInputEnabled()
				&& ((marioBox.x + marioBox.w >= pipeBox.x + 16) && (marioBox.x + marioBox.w < pipeBox.x + pipeBox.w))
				)
			{
				MovingAnimation* pipe_anim = new MovingAnimation("pipe_down", supermario->GetSelf()->GetBox().h, 1, 0, 2 * 25000);
				MovingAnimator* pipe_animator = new MovingAnimator();
				pipe_animator->SetOnAction(
					[=](Animator* animator, const Animation &anim_ref) {
						supermario->GetSelf()->Move(0, 1);
					}
				);
				pipe_animator->SetOnStart(
					[=](Animator* animator) {
						supermario->SetInput(false);
						supermario->GetSelf()->SetHasDirectMotion(true);
					}
				);
				pipe_animator->SetOnFinish(
					[=](Animator* animator) {
						extern CameraMover cameraMover;
						auto results = EntityHolder::Get().Find("transportPoint");
						if (results.size() == 1) {
							SimpleEntity *transportPoint = (SimpleEntity *)results[0];
							Rect box = transportPoint->GetSelf()->GetBox();
							supermario->GetSelf()->SetPos(box.x, box.y);
							supermario->GetSelf()->Move(0, 0);
							cameraMover.SetRightMost(supermario->GetSelf()->GetBox().x);
							myTile.viewWin.dimensions.x = supermario->GetSelf()->GetBox().x - 48;
							supermario->SetInput(true);
							supermario->GetSelf()->SetHasDirectMotion(false);
						}
						else // If you have transporting pipes, you need to have a transportPoint
							assert(0);
					}
				);
				pipe_animator->Start(pipe_anim, SystemClock::Get().micro_secs());
			}
		};
		CollisionChecker::GetSingleton().Register(supermario->GetSelf(), first_pipe->GetSelf(), pipeF);

		return first_pipe;
	}
	SpriteEntity *CreateTransportPoint(int x, int y) {
		auto transportPoint = new SimpleEntity(x, y, AnimationFilmHolder::Get().GetFilm("empty"), "transportPoint");
		EntityHolder::Get().Add(transportPoint);
		return transportPoint;
	}
	SpriteEntity *CreateTransportPipe2(int x, int y) {
		auto return_first_pipe = new SimpleEntity(x, y, AnimationFilmHolder::Get().GetFilm("green.pipe.left"), "pipe");
		return_first_pipe->GetSelf()->SetVisibility(true);
		EntityHolder::Get().Add(return_first_pipe);
		Mario *supermario = EntityHolder::Get().GetSuperMario();
		std::function<void(Sprite* s1, Sprite* s2)> pipeReturn = [return_first_pipe, supermario, this](Sprite* s1, Sprite* s2) {
			ALLEGRO_KEYBOARD_STATE key;
			al_get_keyboard_state(&key);
			auto marioBox = supermario->GetSelf()->GetBox();
			auto pipeBox = return_first_pipe->GetSelf()->GetBox();
			if (
				al_key_down(&key, ALLEGRO_KEY_RIGHT) && supermario->GetInputEnabled()
				&& marioBox.y + marioBox.h > pipeBox.y + 16
				)
			{
				MovingAnimation* pipe_anim = new MovingAnimation("pipe_left", supermario->GetSelf()->GetBox().h, 1, 0, 2 * 25000);
				MovingAnimator* pipe_animator = new MovingAnimator();
				pipe_animator->SetOnAction(
					[=](Animator* animator, const Animation &anim_ref) {
						supermario->GetSelf()->Move(1, 0);
					}
				);
				pipe_animator->SetOnStart(
					[=](Animator* animator) {
						supermario->SetInput(false);
						supermario->GetSelf()->SetHasDirectMotion(true);
					}
				);
				pipe_animator->SetOnFinish(
					[=](Animator* animator) {
						extern CameraMover cameraMover;
						std::vector<SpriteEntity *> results = EntityHolder::Get().Find("transportPipe1");
						if (results.size() == 0)
							assert(0);

						SimpleEntity *firstPipe = (SimpleEntity *)results[0];
						Rect firstPipeBox = firstPipe->GetSelf()->GetBox();
						supermario->GetSelf()->SetPos(firstPipeBox.x + 8, firstPipeBox.y - supermario->GetSelf()->GetBox().h);
						supermario->GetSelf()->Move(0, 0);
						cameraMover.SetRightMost(supermario->GetSelf()->GetBox().x);
						myTile.viewWin.dimensions.x = supermario->GetSelf()->GetBox().x - 48;
						supermario->SetInput(true);
						supermario->GetSelf()->SetHasDirectMotion(false);
					}
				);
				pipe_animator->Start(pipe_anim, SystemClock::Get().micro_secs());
			}
		};
		CollisionChecker::GetSingleton().Register(supermario->GetSelf(), return_first_pipe->GetSelf(), pipeReturn);
		return return_first_pipe;
	}
	SpriteEntity *CreateCoin(int x, int y) {
		SimpleEntity *coin = new SimpleEntity(x, y, AnimationFilmHolder::Get().GetFilm("coin.idle"), "coin");
		coin->GetSelf()->SetVisibility(true);
		FrameRangeAnimation *frameAnimation = new FrameRangeAnimation("coin.idle", 0, 3, 0, 0, 0, 250000);
		FrameRangeAnimator *frameAnimator = new FrameRangeAnimator();
		frameAnimator->Start(coin->GetSelf(), frameAnimation, SystemClock::Get().micro_secs());

		CollisionCallback onCollision = [=](Sprite *supermario, Sprite *coin) {

			EntityHolder::Get().GetSuperMario()->AddPoints(100);
			EntityHolder::Get().GetSuperMario()->AddCoin(1);
			if (std::stoi(EntityHolder::Get().GetSuperMario()->GetCoins()) >= 100) {
				EntityHolder::Get().GetSuperMario()->CoinsToLives();
			}
			coin->SetVisibility(false);
			SimpleEntity *coinEntity = (SimpleEntity *)EntityHolder::Get().GetSpriteEntity(coin);
			EntityHolder::Get().Remove(coinEntity);
			CollisionChecker::GetSingleton().Cancel(coin);
		};
		CollisionChecker::GetSingleton().Register(EntityHolder::Get().GetSuperMario()->GetSelf(), coin->GetSelf(), onCollision);

		return coin;
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

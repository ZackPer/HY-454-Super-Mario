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


void DeathJump(MovingEntity* entity);
// Singleton class that holds all the primitive callback functions for entity creation.
class PrimitiveHolder {
public:
	using PrimitiveCallback = std::function<SpriteEntity *(int x, int y)>;
	using CollisionCallback = std::function<void(Sprite *s1, Sprite *s2)>;
	using ShowTextCallback = std::vector < std::function<void()>>;

	ShowTextCallback showText;
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
		goomba->SetTpe("enemy");
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

		InitiateJumpModule(goomba, goomba->GetSelf());
		PrepareEnemyColliders(goomba);

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
		SimpleEntity *pipe = new SimpleEntity(x, y, AnimationFilmHolder::Get().GetFilm("pipe"), "pipe");
		pipe->GetSelf()->SetVisibility(true);
		pipe->GetSelf()->SetZorder(5);
		PiranhaPlant *piranha = new PiranhaPlant(x + 8, y - 23);

		Mario *supermario = EntityHolder::Get().GetSuperMario();
		CollisionCallback onPipeCollision = [piranha, supermario](Sprite *supermario, Sprite *pipe) {
			piranha->Hide();
		};
		CollisionChecker::GetSingleton().Register(supermario->GetSelf(), pipe->GetSelf(), onPipeCollision);

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

		ALLEGRO_SAMPLE *soundEf = al_load_sample("Sounds/smb_pipe.wav");

		std::function<void(Sprite* s1, Sprite* s2)> pipeF = [first_pipe, supermario, soundEf](Sprite* s1, Sprite* s2) {
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
						al_play_sample(soundEf, 0.3, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
					}
				);
				pipe_animator->SetOnFinish(
					[=](Animator* animator) {
						extern CameraMover cameraMover;
						auto results = EntityHolder::Get().FindBySpriteTypeId("transportPoint");
						if (results.size() > 0) {
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
			ALLEGRO_SAMPLE *soundEf = al_load_sample("Sounds/smb_pipe.wav");
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
						al_play_sample(soundEf, 0.5, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
					}
				);
				pipe_animator->SetOnFinish(
					[=](Animator* animator) {
						extern CameraMover cameraMover;
						std::vector<SpriteEntity *> results = EntityHolder::Get().FindByType("transportPipe1");
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

		ALLEGRO_SAMPLE *soundEf = al_load_sample("Sounds/smb_coin.wav");
		CollisionCallback onCollision = [=](Sprite *supermario, Sprite *coin) {
			ALLEGRO_FONT* font = al_load_ttf_font("SuperPlumberBrothers.ttf", 32, NULL);
			std::cout << y << std::endl;
			std::cout << coin->GetBox().x << "  " << coin->GetBox().y << std::endl;
			showText.push_back([=] {
				al_draw_text(font, al_map_rgb(255, 255, 255), x+x, y+y+y/2, ALLEGRO_ALIGN_CENTER, "100");
			});

			al_play_sample(soundEf, 0.5, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
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
	SpriteEntity* CreateSuperMushroom(int x, int y, Sprite* s) {
		MovingEntity* mushroom = new MovingEntity(x, y, AnimationFilmHolder::Get().GetFilm("mushroom"), "mushroom", myGrid, s);
		mushroom->SetTpe("powerup");
		FrameRangeAnimation walk = FrameRangeAnimation("mushroom", 0, 1, 0, 0, 0, 300000);
		FrameRangeAnimation* death = new FrameRangeAnimation("empty", 0, 1, 1, 0, 0, 400000);
		mushroom->SetWalkLeft(walk);
		mushroom->SetWalkRight(walk);
		mushroom->SetOnDeath(
			mushroom->Prepare_DefaultOnDeath(mushroom, death)
		);
		mushroom->StartMoving();
		mushroom->SetSpeed(2);
		mushroom->SetSign(1);
		mushroom->SetEdgeDetection(false);

		InitiateJumpModule(mushroom, s);

		Mario* supermario = EntityHolder::Get().GetSuperMario();
		CollisionChecker::GetSingleton().Register(
			supermario->GetSelf(),
			mushroom->GetSelf(),
			[=](Sprite* s1, Sprite* s2) {
				Mario* supermario = EntityHolder::Get().GetSuperMario();
				mushroom->Die();
				supermario->Grow(supermario->looking);
			}
		);

		EntityHolder::Get().Add(mushroom);
		return mushroom;
	}
	SpriteEntity* CreateLifeMushroom(int x, int y) {

	}
	SpriteEntity* CreateStar(int x, int y, Sprite* s) {
		MovingEntity* star = new MovingEntity(x, y, AnimationFilmHolder::Get().GetFilm("star"), "star", myGrid, s);
		FrameRangeAnimation walk = FrameRangeAnimation("star", 0, 1, 0, 0, 0, 300000);
		FrameRangeAnimation* death = new FrameRangeAnimation("empty", 0, 1, 1, 0, 0, 400000);
		star->SetTpe("powerup");
		star->SetWalkLeft(walk);
		star->SetWalkRight(walk);
		star->SetOnDeath(
			star->Prepare_DefaultOnDeath(star, death)
		);
		star->StartMoving();
		star->SetSpeed(2);
		star->SetSign(1);
		star->SetEdgeDetection(false);

		InitiateJumpModule(star, s);
		star->StartJump(0);

		Mario* supermario = EntityHolder::Get().GetSuperMario();
		CollisionChecker::GetSingleton().Register(
			supermario->GetSelf(),
			star->GetSelf(),
			[=](Sprite* s1, Sprite* s2) {
				Mario* supermario = EntityHolder::Get().GetSuperMario();
				star->Die();
				supermario->SethasStar(true);
				supermario->starAnimation.StartAnimation(SystemClock::Get().milli_secs());
			}
		);

		EntityHolder::Get().Add(star);
		return star;
	}
	void SetMyGrid(GridLayer *myGrid) {
		this->myGrid = myGrid;
	}
	void SetViewWindow(Rect *viewWindow) {
		this->viewWindow = viewWindow;
	}

	void InitiateJumpModule(MovingEntity* entity, Sprite* s) {
		entity->SetJumpModule(new JumpModule(s));
		entity->GetjumpModule()->SetIsFalling(entity->GetgravityModule().GetIsFallingRef());
		entity->GetjumpModule()->Init();
	}
private:
	GridLayer	*myGrid;
	Rect		*viewWindow;
	
	SpriteEntity *CreateKoopa(int x, int y, std::string color) {
		MovingEntity *koopa = new MovingEntity(x, y - 8, AnimationFilmHolder::Get().GetFilm(color + ".koopa.walk.left"), color + ".koopa", myGrid);
		koopa->SetTpe("enemy");
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

		PrepareEnemyColliders(koopa);

		EntityHolder::Get().Add(koopa);
		return koopa;
	}

	void PrepareEnemyColliders(SpriteEntity *entity) {
		auto enemies = EntityHolder::Get().FindByType("enemy");
		for (auto &it : enemies) {
			auto signChange = PrepareEnemySignChange((MovingEntity  *)entity, (MovingEntity *)it);
			CollisionChecker::GetSingleton().Register(entity->GetSelf(), it->GetSelf(), signChange);
		}

		auto shells = EntityHolder::Get().FindByType("shell");
		for (auto &shell : shells) {
			auto jumpDeath = PrepareDeathFromShell((MovingEntity  *)shell, (MovingEntity  *)entity);
			CollisionChecker::GetSingleton().Register(shell->GetSelf(), entity->GetSelf(), jumpDeath);
		}

		Mario *mario = EntityHolder::Get().GetSuperMario();
		CollisionChecker::GetSingleton().Register(mario->GetSelf(), entity->GetSelf(), PrepareKillBounce(mario, entity));
	}
	CollisionCallback PrepareDeathFromShell(MovingEntity *shell, MovingEntity *entity) {
		return [shell, entity](Sprite *s1, Sprite *s2) {
			DeathJump(entity);
		};
	}
	CollisionCallback PrepareEnemySignChange(MovingEntity *enemy1, MovingEntity *enemy2) {
		return [enemy1, enemy2](Sprite *s1, Sprite *s2) {
			enemy1->SetSign(enemy1->GetSign() * -1);
			enemy2->SetSign(enemy2->GetSign() * -1);
		};
	}
	CollisionCallback PrepareKillBounce(Mario *supermario, SpriteEntity *entity) {
		return [entity](Sprite *s1, Sprite *s2) {
			Mario *supermario = EntityHolder::Get().GetSuperMario();
			Rect marioBox = s1->GetBox();
			Rect entityBox = s2->GetBox();
			if (supermario->GethasStar()) {
				DeathJump((MovingEntity*)entity);
			}	
			else if (marioBox.y + marioBox.h < entityBox.y + 5) {
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
		EntityHolder::Get().GetSuperMario()->AddPoints(100);
		entity->StopMoving();
		entity->GetSelf()->SetVisibility(false);
		EntityHolder::Get().Remove(entity);
		CollisionChecker::GetSingleton().Cancel(entity->GetSelf());
		SpriteManager::GetSingleton().Remove(entity->GetSelf());
	};
}

void DeathJump(MovingEntity* entity) {
	entity->GetSelf()->SetZorder(6);

	if (entity->GetSelf()->GetTypeId() == "goomba") {
		entity->GetSelf()->setCurrFilm(AnimationFilmHolder::Get().GetFilm("goomba.jump.death"));
	}
	else if (entity->GetSelf()->GetTypeId() == "red.koopa") {
		entity->GetSelf()->setCurrFilm(AnimationFilmHolder::Get().GetFilm("red.koopa.jump.death"));
	}
	else if (entity->GetSelf()->GetTypeId() == "green.koopa") {
		entity->GetSelf()->setCurrFilm(AnimationFilmHolder::Get().GetFilm("green.koopa.jump.death"));
	}
	entity->GetSelf()->SetFrame(0);
	entity->GetSelf()->SetFrame(1);

	entity->SetjumpAnimation(MovingAnimation(
		"jump.death",
		8,
		0,
		-2,
		30000
	));
	entity->GetjumpAnimator().SetOnAction(
		[=](Animator* animator, const Animation& anim) {
			entity->GetSelf()->SetHasDirectMotion(true).Move(((const MovingAnimation&)anim).GetDx(), ((const MovingAnimation&)anim).GetDy()).SetHasDirectMotion(true);
		}
	);
	entity->GetjumpAnimator().SetOnStart(
		[=](Animator* animator) {
			CollisionChecker::GetSingleton().Cancel(entity->GetSelf());
		}
	);
	entity->GetjumpAnimator().SetOnFinish(
		[=](Animator* animator) {
			entity->SetjumpAnimation(MovingAnimation(
				"jump.death",
				0,
				0,
				5,
				30000
			));
			((MovingAnimator*)animator)->Start(entity->GetjumpAnimationPtr(), SystemClock::Get().micro_secs());
			entity->GetjumpAnimator().SetOnFinish(
				[=](Animator* animator) {
					entity->GetjumpAnimator().SetOnStart(
						[=](Animator* animator) {
							nullptr;
						}
					);
					entity->StopMoving();
					entity->GetSelf()->SetVisibility(false);
					EntityHolder::Get().Remove(entity);
					SpriteManager::GetSingleton().Remove(entity->GetSelf());
				}
			);
		}
	);
	entity->GetjumpAnimator().Start(entity->GetjumpAnimationPtr(), SystemClock::Get().micro_secs());
}

std::function<void()> MovingEntity::Prepare_DefaultOnDeath(MovingEntity *entity, FrameRangeAnimation *deathAnimation) {
	return [entity, deathAnimation]() {
		entity->StopMoving();
		EntityHolder::Get().Remove(entity);
		EntityHolder::Get().GetSuperMario()->AddPoints(100);
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

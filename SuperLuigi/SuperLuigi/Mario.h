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



extern TileLayer myTile;
extern CameraMover cameraMover;
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
		score = 0;
		lives = 3;
		coins = 0;
		spawnPos.x = 48;
		spawnPos.y = 48;
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
					ALLEGRO_SAMPLE *soundEf = al_load_sample("Sounds/smb_jump_small.wav");
					al_play_sample(soundEf, 0.3, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
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
				else if (al_key_down(&keyState, ALLEGRO_KEY_C)) {
					MoveToCheckPoint();
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

	void MoveToCheckPoint() {
		GetSelf()->SetHasDirectMotion(true);
		selfMover->StopAllAnimators();
		GetSelf()->SetPos(spawnPos.x, spawnPos.y);
		GetSelf()->Move(0, 0);
		cameraMover.SetRightMost(GetSelf()->GetBox().x);
		myTile.viewWin.dimensions.x = GetSelf()->GetBox().x - 48;
		GetSelf()->SetHasDirectMotion(false);
	}

	void Die() {
		if (animationState != DYING) {
			ALLEGRO_SAMPLE *soundEf = al_load_sample("Sounds/smb_mariodie.wav");
			al_play_sample(soundEf, 0.5, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
			if (lives > 0) {
				this->lives--;
			}
		}
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
		return score;
	}
	void SetPoints(int points){
		this->score = points;
	}
	void AddPoints(int amount) {
		this->score += amount;
	}

	void AddCoin(int _coin) {
		this->coins += _coin;
	}
	
	void AddLives(int _lives) {
		this->lives += _lives;
	}

	void CoinsToLives() {
		this->AddLives(1);
		this->coins = 0;
	}

	std::string GetScore() {
		std::string _score = std::to_string(score);
		return _score;
	}

	std::string GetCoins() {
		std::string _coins = std::to_string(coins);
		return _coins;
	}

	std::string GetLives() {
		std::string _lives = std::to_string(lives);
		return _lives;
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
	int					lives;
	int					coins;
	int					score;
	
	struct SpawnPosition {
		int x;
		int y;
	};

	struct SpawnPosition spawnPos;

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

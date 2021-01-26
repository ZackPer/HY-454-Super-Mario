#pragma once

#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>
#include <iostream>
#include "./Engine/Animations/AnimationFilm.h"
#include "./Engine/Animations/Animations.h"
#include "./Engine/Animations/Animators.h"
#include "./Engine/Tiles/Tiles.h"
#include "./Engine/Grid/Grid.h"
#include "Game.h"
#include "./Engine/SystemClock.h"
#include "./Engine/Sprite/Sprite.h"
#include "./Engine/Sprite/Clipper.h"
#include "./Engine/Physics/CollisionHander.h"
#include "./MarioWalkAnimation.h"
#include "./CameraMover.h"
#include "./Mario.h"
#include "./MovingEntity.h"
#include "./EntitySpawner.h"
#include "BlockTile.h"

#define WIDTH	720
#define	HEIGHT	540

const Clipper MakeTileLayerClipper(TileLayer* layer) {
	return Clipper().SetView(
		[layer](void)
	{ return layer->GetViewWindow(); }
	);
}

AnimationFilm littleshit;
TileLayer myTile;
GridLayer myGrid;

CameraMover cameraMover;
Sprite marioSprite;
Sprite luigiSprite;
Clipper clipper;
Rect cameraCoords;
EntitySpawner *entitySpawner;

Mario *supermario;
MovingEntity *goomba;
MysteryTile* mysteryTileMushroom;
BrickTile* bricktile;

namespace mario {
	class App {
	public:
		App() {

		}
		
		void Initialize() {			
			Allegro_Initilization();
		}
	protected:
		Game game;
		//+++
	private:
		ALLEGRO_DISPLAY* display;

		void Allegro_Initilization() {
			if (!al_init())
				std::cout << "There was an error in Allegro initialization.." << std::endl;
			display = al_create_display(WIDTH, HEIGHT);
			al_install_keyboard();
			al_init_image_addon();
			al_init_primitives_addon();
		}
	};
}

TileLayer a;
int startPosX = 0, startPosY = 0;


void InitPrimitiveCallbacks() {
	EntitySpawner::Get().Add(
		196,
		[=](int x, int y) -> SpriteEntity * {
			supermario = new Mario(x, y - 1, &myGrid, &(myTile.viewWin.dimensions));
			supermario->GetSelf()->SetVisibility(true);
			return supermario;
		}
	);

	EntitySpawner::Get().Add(
		200,
		[=](int x, int y) -> SpriteEntity * {
			//Goomba
			MovingEntity *goomba = new MovingEntity(x, y, AnimationFilmHolder::Get().GetFilm("goomba.walk"), "goomba", &myGrid);
			FrameRangeAnimation walk = FrameRangeAnimation("goomba.walk", 0, 2, 0, 0, 0, 300000);
			FrameRangeAnimation *death = new FrameRangeAnimation("goomba.death", 0, 1, 1, 0, 0, 400000);
			goomba->SetWalkLeft(walk);
			goomba->SetWalkRight(walk);
			goomba->SetOnDeath(
				Prepare_MovingEntityOnDeath(goomba, death)
			);
			goomba->StartMoving();
			goomba->SetEdgeDetection(false);

			std::function<void(Sprite *s1, Sprite *s2)> f = [goomba](Sprite *s1, Sprite *s2) {
				Rect marioBox = s1->GetBox();
				Rect goombaBox = s2->GetBox();
				if (marioBox.y + marioBox.h < goombaBox.y + 5) {
					goomba->Die();
					supermario->Bounce();
				}
				else {
					supermario->Die();
				}
			};
			CollisionChecker::GetSingleton().Register(supermario->GetSelf(), goomba->GetSelf(), f);

			return goomba;
		}
	);
	EntitySpawner::Get().Add(
		201,
		[=](int x, int y) -> SpriteEntity * {
			//Green Koopa
			FrameRangeAnimation walkLeft = FrameRangeAnimation("green.koopa.walk.left", 0, 2, 0, 0, 0, 300000);
			FrameRangeAnimation walkRight = FrameRangeAnimation("green.koopa.walk.right", 0, 2, 0, 0, 0, 300000);
			//Todo add animation  for death
			MovingEntity *greenKoopa = new MovingEntity(x, y - 8, AnimationFilmHolder::Get().GetFilm("green.koopa.walk.left"), "greek.koopa", &myGrid);
			greenKoopa->SetWalkLeft(walkLeft);
			greenKoopa->SetWalkRight(walkRight);
			greenKoopa->StartMoving();
			greenKoopa->SetEdgeDetection(false);
			greenKoopa->GetSelf()->SetVisibility(true);
			return greenKoopa;
		}
	);
	EntitySpawner::Get().Add(
		220,
		[=](int x, int y) -> SpriteEntity * {
		//Green Koopa
		FrameRangeAnimation walkLeft = FrameRangeAnimation("red.koopa.walk.left", 0, 2, 0, 0, 0, 300000);
		FrameRangeAnimation walkRight = FrameRangeAnimation("red.koopa.walk.right", 0, 2, 0, 0, 0, 300000);
		//Todo add animation  for death
		MovingEntity *redKoopa = new MovingEntity(x, y - 8, AnimationFilmHolder::Get().GetFilm("red.koopa.walk.left"), "red.koopa", &myGrid);
		redKoopa->SetWalkLeft(walkLeft);
		redKoopa->SetWalkRight(walkRight);
		redKoopa->StartMoving();
		redKoopa->SetEdgeDetection(true);
		redKoopa->GetSelf()->SetVisibility(true);
		return redKoopa;
	}
	);
	EntitySpawner::Get().Add(
		216,
		[=](int x, int y) -> SpriteEntity * {
			//Mystery Mushroom tile.
			MysteryTile *mysteryTile = new MysteryTile(x, y, &myGrid, "mushroom");
			mysteryTile->GetSprite()->SetVisibility(true);

			//Collider
			CollisionChecker::GetSingleton().Register(
				supermario->GetSelf(),
				mysteryTile->GetSprite(),
				mysteryTile->GetOnCollison()
			);

			return nullptr;
		}
	);
}

void initializeAnimationsAndSprites() {
	//Initialize AnimationFilms
	AnimationFilmHolder::Get() = InitAnimationFilmHolder();
	
	// Parses and spawns all entities.
	InitPrimitiveCallbacks();
	EntitySpawner::Get().ParseAndReplaceSpawnPoints(myTile, myGrid);

	//Clipper
	clipper = Clipper();
	clipper = MakeTileLayerClipper(&myTile);

}

void CoreLoop(ALLEGRO_DISPLAY *display, TileMap mapTileIndexes) {
	ALLEGRO_EVENT_QUEUE *event_queue = nullptr;
	ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60);
	event_queue = al_create_event_queue();
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	al_register_event_source(event_queue, al_get_timer_event_source(timer));

	int dx = 0, dy = 0;
	bool draw = true;

	ALLEGRO_BITMAP* beforeScaleBitmap = al_create_bitmap(WIDTH, HEIGHT);
	ALLEGRO_KEYBOARD_STATE	keyboardState;
	al_start_timer(timer);

	EntitySpawner::Get().CheckForSpawn(myTile.viewWin.displayArea);
	//EntitySpawner::Get().SpawnSprites();
	assert(supermario);
	cameraMover = CameraMover(&myTile, supermario->GetSelf(), supermario->GetSelf()->GetBox().x);

	while (1) {
		ALLEGRO_EVENT ev;
		al_wait_for_event(event_queue, &ev);
		if (ev.type == ALLEGRO_EVENT_TIMER) {
			//calculate starting position
			startPosX = supermario->GetSelf()->GetBox().x;
			startPosY = supermario->GetSelf()->GetBox().y;	
			//get Input
			supermario->InputPoll();
			//movements
			supermario->GetselfMover()->Move(supermario->direction, supermario->isRunning, supermario->isSuper, supermario->looking);
			//reset variables
			supermario->isRunning = false;
			supermario->direction = NO;
			draw = true;
		}
		if (draw == true) {		
			draw = false;
			al_flip_display();
			AnimatorManager::GetSingleton().Progress(SystemClock::Get().micro_secs());

			//calculate ending position
			dx = supermario->GetSelf()->GetBox().x - startPosX;
			dy = supermario->GetSelf()->GetBox().y - startPosY;
			//move camera
			cameraMover.ScrollAccordingToCharacter(dx, dy);

			myTile.TileTerrainDisplay(mapTileIndexes, myTile.viewWin.bitmap, myTile.viewWin.dimensions, myTile.viewWin.displayArea);

			BitmapBlit(myTile.viewWin.bitmap, Rect(0, 0, myTile.viewWin.dimensions.w, myTile.viewWin.dimensions.h), beforeScaleBitmap, Point(0, 0));

			EntitySpawner::Get().CheckForSpawn(myTile.viewWin.dimensions);
			//Display all sprites that are visible:
			auto list = SpriteManager::GetSingleton().GetDisplayList();
			for (auto &it : list)
				if (it->IsVisible()) {
					it->Display(beforeScaleBitmap, cameraCoords, clipper);
				}

			al_set_target_bitmap(al_get_backbuffer(display));
			al_draw_scaled_bitmap(beforeScaleBitmap, 0, 0, WIDTH / 3, HEIGHT / 3, 0, 0, WIDTH, HEIGHT, 0);
			dx = dy = 0;
			CollisionChecker::GetSingleton().Check();
		}
	}

	//destroyAllegroComponents --> make that a delegate
	[=]() {
		al_destroy_event_queue(event_queue);
	};
}

int main() {
	ALLEGRO_DISPLAY *display;
	myTile = TileLayer("repo/SuperLuigi/SuperLuigi/CSVMaps/map1.csv");
	myGrid = GridLayer(myTile.TileMapIndexes);

	int mapColumns, mapRows;
	mapRows = myTile.TileMapIndexes.size();
	mapColumns = myTile.TileMapIndexes[0].size();

	if(!al_init())
		return -1;
	display = al_create_display(WIDTH, HEIGHT);

	//allegro addons
	al_install_keyboard();
	al_init_image_addon();
	al_init_primitives_addon();

	myTile.tileset = al_load_bitmap("Tiles/super_mario_tiles_with_sprites.png");
	myTile.TileLayerBitmap = al_create_bitmap(mapRows*TILE_HEIGHT, mapColumns*TILE_WIDTH);

	//initializing view window
	myTile.SetViewWin(ViewWindow(
		WIDTH / 3, HEIGHT / 3, 0, 0, 
		0, 0, 0, 0)
	);

	cameraCoords = Rect(
		0,
		0,
		myTile.viewWin.dimensions.w,
		myTile.viewWin.dimensions.h
	);

	initializeAnimationsAndSprites();	
	
	//mapping map indexes to tilesetIndexes
	TileMap mapTileIndexes;
	myTile.getMapIndexes(mapTileIndexes, myTile.TileMapIndexes);


	//The Original Super Mario Bros Game Loop (but we call it CoreLoop.)
	CoreLoop(display, mapTileIndexes);
	
	//destroyAllegroComponents --> make that a delegate
	[=](){
		al_destroy_bitmap(myTile.tileset);
		al_destroy_display(display);
	};
	
	return 0;
}
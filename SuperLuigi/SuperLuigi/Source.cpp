#pragma once

#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_font.h>
#include <iostream>
#include "./Engine/Animations/AnimationFilm.h"
#include "./Engine/Animations/Animations.h"
#include "./Engine/Animations/Animators.h"
#include "./Engine/Tiles/Tiles.h"
#include "./Engine/Grid/Grid.h"
#include "Game.h"
#include "./Engine/SystemClock.h"
#include "./Engine/Sprite/Pipe.h"
#include "./Engine/Sprite/Sprite.h"
#include "./Engine/Sprite/Clipper.h"
#include "./Engine/Physics/CollisionHander.h"
#include "./MarioWalkAnimation.h"
#include "./CameraMover.h"
#include "./Mario.h"
#include "./MovingEntity.h"
#include "./EntitySpawner.h"
#include "BlockTile.h"
#include "./PrimitiveHolder.h"

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
	PrimitiveHolder::Get().SetMyGrid(&myGrid);
	PrimitiveHolder::Get().SetViewWindow(&myTile.viewWin.dimensions);

	EntitySpawner::Get().Add(
		196,
		[=](int x, int y) -> SpriteEntity * {
			supermario = (Mario *)PrimitiveHolder::Get().CreateMario(x, y);
			EntityHolder::Get().Add(supermario);
			return supermario;
		}
	);
	EntitySpawner::Get().Add(
		200,
		[=](int x, int y) -> SpriteEntity * {
			return PrimitiveHolder::Get().CreateGoomba(x, y);
		}
	);
	EntitySpawner::Get().Add(
		201,
		[=](int x, int y) -> SpriteEntity * {
			return PrimitiveHolder::Get().CreateGreenKoopa(x, y);
		}
	);
	EntitySpawner::Get().Add(
		220,
		[=](int x, int y) -> SpriteEntity * {
			return PrimitiveHolder::Get().CreateRedKoopa(x, y);
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
	EntitySpawner::Get().Add(
		117,
		[=](int x, int y) -> SpriteEntity * {
			return PrimitiveHolder::Get().CreatePiranhaPlant(x, y);
		}
	);

	EntitySpawner::Get().Add(
		41,
		[=](int x, int y) -> SpriteEntity * {
			return PrimitiveHolder::Get().CreatePipe(x, y);
		}
	);

	EntitySpawner::Get().Add(
		197,
		[=](int x, int y) -> SpriteEntity* {
			//Mystery Mushroom tile.
			MysteryTile* mysteryTile = new MysteryTile(x, y, &myGrid, "lifemushroom");
			mysteryTile->GetSprite()->SetVisibility(true);

			//Collider
			CollisionChecker::GetSingleton().Register(
				supermario->GetSelf(),
				mysteryTile->GetSprite(),
				mysteryTile->GetOnCollison()
			);
			//if not returned memory leak
			return nullptr;
		}
	);
	EntitySpawner::Get().Add(
		199,
		[=](int x, int y) -> SpriteEntity* {
			//Mystery Mushroom tile.
			MysteryTile* mysteryTile = new MysteryTile(x, y, &myGrid, "star");
			mysteryTile->GetSprite()->SetVisibility(true);

			//Collider
			CollisionChecker::GetSingleton().Register(
				supermario->GetSelf(),
				mysteryTile->GetSprite(),
				mysteryTile->GetOnCollison()
			);
			//if not returned memory leak
			return nullptr;
		}
	);
	EntitySpawner::Get().Add(
		198,
		[=](int x, int y) -> SpriteEntity* {
			//Mystery Mushroom tile.
			BrickTile* brick = new BrickTile(x, y, &myGrid);
			brick->GetSprite()->SetVisibility(true);

			//Collider
			CollisionChecker::GetSingleton().Register(
				supermario->GetSelf(),
				brick->GetSprite(),
				brick->GetOnCollison()
			);
			//if not returned memory leak
			return nullptr;
		}
	);
	EntitySpawner::Get().Add(
		79,
		[=](int x, int y) -> SpriteEntity *{
			return PrimitiveHolder::Get().CreateTransportPipe1(x, y);
		}
	);
	EntitySpawner::Get().Add(
		103,
		[=](int x, int y) -> SpriteEntity *{
			return PrimitiveHolder::Get().CreateTransportPoint(x, y);
		}
	);
	EntitySpawner::Get().Add(
		81,
		[=](int x, int y) -> SpriteEntity *{
			return PrimitiveHolder::Get().CreateTransportPipe2(x, y);
		}
	);
	EntitySpawner::Get().Add(
		179,
		[=](int x, int y) -> SpriteEntity * {
			return PrimitiveHolder::Get().CreateCoin(x, y);
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

	EntitySpawner::Get().CheckForSpawn(myTile.viewWin.displayArea); //Everything on the first frame is spawned here including mario.
	assert(supermario);
	cameraMover = CameraMover(&myTile, supermario->GetSelf(), supermario->GetSelf()->GetBox().x);
	EntityHolder::Get().SetSuperMario(supermario);
	ALLEGRO_FONT* font = al_load_ttf_font("SuperPlumberBrothers.ttf", 32, NULL);

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
			if(supermario->animationState != GROWING && supermario->animationState != DYING)
				supermario->GetselfMover()->Move(supermario->direction, supermario->isRunning, supermario->isSuper, supermario->looking, supermario->animationState);

			al_draw_text(font, al_map_rgb(255, 255, 255), 50, 30, ALLEGRO_ALIGN_CENTER, "SCORE");
			al_draw_text(font, al_map_rgb(255, 255, 255), 50, 70, ALLEGRO_ALIGN_CENTER, "O");
			al_draw_text(font, al_map_rgb(255, 255, 255), 200, 30, ALLEGRO_ALIGN_CENTER, "COINS");
			al_draw_text(font, al_map_rgb(255, 255, 255), 200, 70, ALLEGRO_ALIGN_CENTER, "O");
			al_draw_text(font, al_map_rgb(255, 255, 255), 350, 30, ALLEGRO_ALIGN_CENTER, "LIVES");
			al_draw_text(font, al_map_rgb(255, 255, 255), 350, 70, ALLEGRO_ALIGN_CENTER, "O");
			al_flip_display();

			AnimatorManager::GetSingleton().Progress(SystemClock::Get().micro_secs());

			//calculate ending position
			dx = supermario->GetSelf()->GetBox().x - startPosX;
			dy = supermario->GetSelf()->GetBox().y - startPosY;	

			//move camera
			cameraMover.ScrollAccordingToCharacter(dx, dy);
			//calculate and blit view 
			myTile.TileTerrainDisplay(mapTileIndexes, myTile.viewWin.bitmap, myTile.viewWin.dimensions, myTile.viewWin.displayArea);
			BitmapBlit(myTile.viewWin.bitmap, Rect(0, 0, myTile.viewWin.dimensions.w, myTile.viewWin.dimensions.h), beforeScaleBitmap, Point(0, 0));

			EntitySpawner::Get().CheckForSpawn(myTile.viewWin.dimensions);
			//Display all sprites that are visible:
			auto list = SpriteManager::GetSingleton().GetDisplayList();
			for (auto &it : list)
				if (it->IsVisible()) {
					if (it->GetTypeId() == "mario") {
						it->DisplayTinted(beforeScaleBitmap, cameraCoords, clipper);
						continue;
					}				
					it->Display(beforeScaleBitmap, cameraCoords, clipper);
				}

			al_set_target_bitmap(al_get_backbuffer(display));
			al_draw_scaled_bitmap(beforeScaleBitmap, 0, 0, WIDTH / 3, HEIGHT / 3, 0, 0, WIDTH, HEIGHT, 0);

			//reset variables
			supermario->isRunning = false;
			supermario->direction = NO;
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
	myTile = TileLayer("CSVMaps/map1.csv");
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
	al_init_ttf_addon();
	al_init_font_addon();

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
#pragma once

#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>
#include <iostream>
#include "Animations/AnimationFilm.h"
#include "Animations/Animations.h"
#include "Animations/Animators.h"
#include "Tiles/Tiles.h"
#include "Grid/Grid.h"
#include "Game.h"
#include "SystemClock.h"
#include "Sprite/Sprite.h"
#include "Sprite/Clipper.h"
#include "Physics/CollisionHander.h"
#include "../MarioWalkAnimation.h"
#include "../CameraMover.h"

#define WIDTH	720
#define	HEIGHT	540

const Clipper MakeTileLayerClipper(TileLayer* layer) {
	return Clipper().SetView(
		[layer](void)
	{ return layer->GetViewWindow(); }
	);
}

AnimationFilm littleshit;
GridLayer myGrid;
TileLayer myTile;
CameraMover cameraMover;
Sprite marioSprite;
Sprite luigiSprite;
Clipper clipper;
Rect cameraCoords;
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
int startPosX = 16, startPosY = 0;
void initializeAnimationsAndSprites() {
	//Initialize AnimationFilms
	AnimationFilmHolder::Get() = InitAnimationFilmHolder();
	
	//Clipper
	clipper = Clipper();
	clipper = MakeTileLayerClipper(&myTile);
	
	//Mario Sprite
	marioSprite = Sprite(startPosX, startPosY, AnimationFilmHolder::Get().GetFilm("littlemario.walk.right"), "mario");
	std::function<void(Rect&, int* dx, int* dy)> gridMover = marioSprite.MakeSpriteGridLayerMover(&myGrid, &marioSprite);
	marioSprite.SetMover(gridMover);
	marioSprite.SetRange(1, 1);
	PrepareSpriteGravityHandler(&myGrid, &marioSprite);

	//Luigi Sprite
	luigiSprite = Sprite(250, 32, AnimationFilmHolder::Get().GetFilm("littlemario.walk.right"), "luigi");
	std::function<void(Sprite* s1, Sprite* s2)> f = [](Sprite* s1, Sprite* s2) {
		//s1->Move(1, 1);
	};
	
	//Collisions
	CollisionChecker::GetSingleton().Register(&marioSprite, &luigiSprite, f);
	marioSprite.SetBoundingArea();
	luigiSprite.SetBoundingArea();

	cameraMover = CameraMover(&myTile, &marioSprite, startPosX);
}


void CoreLoop(ALLEGRO_DISPLAY *display, TileMap mapTileIndexes, MarioMover* marioHandler) {
	ALLEGRO_EVENT_QUEUE *event_queue = nullptr;

	ALLEGRO_TIMER* timer = al_create_timer(1.0 / 30);

	event_queue = al_create_event_queue();
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	al_register_event_source(event_queue, al_get_timer_event_source(timer));

	int dx = 0, dy = 0;
	bool draw = true, isRunning = false, super = false;
	Direction direction = NO;

	ALLEGRO_BITMAP* beforeScaleBitmap = al_create_bitmap(WIDTH, HEIGHT);
	ALLEGRO_KEYBOARD_STATE	keyboardState;
	al_start_timer(timer);
	while (1) {
		ALLEGRO_EVENT ev;
		al_wait_for_event(event_queue, &ev);
		if (ev.type == ALLEGRO_EVENT_TIMER) {
			al_get_keyboard_state(&keyboardState);
			//actions on keys
			if (al_key_down(&keyboardState, ALLEGRO_KEY_DOWN)) {
				direction = DOWN;
			}
			else if (al_key_down(&keyboardState, ALLEGRO_KEY_UP)) {
				direction = UP;
			}
			if (al_key_down(&keyboardState, ALLEGRO_KEY_RIGHT)) {
				direction = RIGHT;	
			}
			else if (al_key_down(&keyboardState, ALLEGRO_KEY_LEFT)) {
				direction = LEFT;
			}
			if (al_key_down(&keyboardState, ALLEGRO_KEY_SPACE)) {
				isRunning = true;
			}
			if (al_key_down(&keyboardState, ALLEGRO_KEY_1)) {
				super = true;
			}
			if (al_key_down(&keyboardState, ALLEGRO_KEY_2)) {
				super = false;
			}

			//before all the movements
			startPosX = marioSprite.GetBox().x;
			startPosY = marioSprite.GetBox().y;

			//movements
			marioHandler->Move(direction, isRunning, super);

			//reset variables
			isRunning = false;
			direction = NO;
			draw = true;
		}		
		if (draw == true) {		
			draw = false;
			al_flip_display();
			AnimatorManager::GetSingleton().Progress(SystemClock::Get().micro_secs());
			myTile.TileTerrainDisplay(mapTileIndexes, myTile.viewWin.bitmap, myTile.viewWin.dimensions, myTile.viewWin.displayArea);

			//after all the movements
			dx = marioSprite.GetBox().x - startPosX;
			dy = marioSprite.GetBox().y - startPosY;
	
			cameraMover.ScrollAccordingToCharacter(dx, dy);
			BitmapBlit(myTile.viewWin.bitmap, Rect( 0, 0, myTile.viewWin.dimensions.w, myTile.viewWin.dimensions.h), beforeScaleBitmap, Point(0, 0));			
			marioSprite.Display(beforeScaleBitmap, cameraCoords, clipper);
			luigiSprite.Display(beforeScaleBitmap, cameraCoords, clipper);
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

	myTile = TileLayer("CSVMaps/mario1.csv");
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

	myTile.tileset = al_load_bitmap("Tiles/super_mario_tiles.png");
	myTile.TileLayerBitmap = al_create_bitmap(mapRows*TILE_HEIGHT, mapColumns*TILE_WIDTH);

	//mapping map indexes to tilesetIndexes
	TileMap mapTileIndexes;
	myTile.getMapIndexes(mapTileIndexes,myTile.TileMapIndexes);

	//initializing view window
	myTile.viewWin = ViewWindow(WIDTH/3, HEIGHT/3, 0, 0,
									0, 0, 0, 0);
	cameraCoords = Rect(
		0,
		0,
		myTile.viewWin.dimensions.w,
		myTile.viewWin.dimensions.h
	);

	initializeAnimationsAndSprites();
	MarioMover marioHandler(&marioSprite);

	//The Original Super Mario Bros Game Loop (but we call it CoreLoop.)
	CoreLoop(display, mapTileIndexes, &marioHandler);
	
	//destroyAllegroComponents --> make that a delegate
	[=](){
		al_destroy_bitmap(myTile.tileset);
		al_destroy_display(display);
	};
	
	return 0;
}
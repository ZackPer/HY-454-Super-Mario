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

Sprite marioSprite;
Sprite luigiSprite;
Clipper clipper;

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
void CoreLoop(ALLEGRO_DISPLAY *display, TileMap mapTileIndexes) {
	ALLEGRO_EVENT_QUEUE *event_queue = nullptr;
	ALLEGRO_EVENT ev;// only allocates memory - does not initialize contents
	event_queue = al_create_event_queue();
	al_register_event_source(event_queue, al_get_keyboard_event_source());

	int dx = 0 , dy = 0, i = 0, j = 0;
	int zachSteps = 4;

	ALLEGRO_BITMAP* beforeScaleBitmap = al_create_bitmap(WIDTH, HEIGHT);


	while (1) {
		bool received_event = false;
		received_event = al_get_next_event(event_queue, &ev);

		if (!received_event) {	
		}
		else {	
			if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
				break;
			}
			else if (ev.keyboard.keycode == ALLEGRO_KEY_DOWN) {
				dy += zachSteps;
			}
			else if (ev.keyboard.keycode == ALLEGRO_KEY_UP) {

				dy -= zachSteps;
			}
			else if (ev.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
				dx += zachSteps;
			}
			else if (ev.keyboard.keycode == ALLEGRO_KEY_LEFT) {
				dx -= zachSteps;
			}
			marioSprite.Move(dx, dy);
			dx = dy = 0;
		}
		al_flush_event_queue(event_queue);
		

		myTile.TileTerrainDisplay(mapTileIndexes, myTile.viewWin.bitmap, myTile.viewWin.dimensions, myTile.viewWin.displayArea);
		
		al_flip_display();
		AnimatorManager::GetSingleton().Progress(SystemClock::Get().micro_secs());
		BitmapBlit(myTile.viewWin.bitmap, myTile.viewWin.dimensions, beforeScaleBitmap, Point(0,0));
		marioSprite.Display(beforeScaleBitmap, myTile.viewWin.dimensions, clipper);
		luigiSprite.Display(beforeScaleBitmap, myTile.viewWin.dimensions, clipper);
		al_set_target_bitmap(al_get_backbuffer(display));
		al_draw_scaled_bitmap(beforeScaleBitmap, 0, 0, WIDTH/3, HEIGHT/3, 0, 0, WIDTH, HEIGHT, 0);
		CollisionChecker::GetSingleton().Check();

		//Looping mario animation.
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
	//singletons
	AnimationFilmHolder::Get() = InitAnimationFilmHolder();


	//Animation
	AnimationFilm *walkRightFilm = AnimationFilmHolder::Get().GetFilm("littlemario.walk.right");
	FrameRangeAnimation walkRightAnimation = FrameRangeAnimation(walkRightFilm->GetId(), 1, walkRightFilm->GetTotalFrames(), 0, 0, 0, 190000);
	
	//Our first Sprite :D
	marioSprite = Sprite(0, 0, AnimationFilmHolder::Get().GetFilm("littlemario.walk.right"), "mario");
	std::function<void(Rect&, int* dx, int* dy)> gridMover = marioSprite.MakeSpriteGridLayerMover(&myGrid, &marioSprite);
	
	marioSprite.SetMover(gridMover);
	marioSprite.SetRange(1, 1);
	PrepareSpriteGravityHandler(&myGrid, &marioSprite);
	//Clipper
	clipper = Clipper();
	clipper = MakeTileLayerClipper(&myTile);

	//Animator
	FrameRangeAnimator walkRightAnimator;
	walkRightAnimator.Start(&marioSprite, &walkRightAnimation, SystemClock::Get().micro_secs());
	
	//Sprite 2
	luigiSprite = Sprite(32, 32, AnimationFilmHolder::Get().GetFilm("littlemario.walk.right"), "luigi");
	std::function<void(Sprite* s1, Sprite* s2)> f = [](Sprite* s1, Sprite* s2) {
		s1->Move(1, 1);
	};

	CollisionChecker::GetSingleton().Register(&marioSprite, &luigiSprite, f);
	marioSprite.SetBoundingArea();
	luigiSprite.SetBoundingArea();
	//The Original Super Mario Bros Game Loop (but we call it CoreLoop.)
	CoreLoop(display, mapTileIndexes);
	
	//destroyAllegroComponents --> make that a delegate
	[=](){
		al_destroy_bitmap(myTile.tileset);
		al_destroy_display(display);
	};
	
	return 0;
}
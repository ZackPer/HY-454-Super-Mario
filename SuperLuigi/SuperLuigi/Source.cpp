#pragma once

#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>
#include <iostream>
#include "AnimationFilm.h"
#include "Animations.h"
#include "Animators.h"
#include "Tiles.h"
#include "Grid.h"
#include "Game.h"
#include "SystemClock.h"

#define WIDTH	720
#define	HEIGHT	540

AnimationFilm littleshit;
GridLayer myGrid;
TileLayer myTile;

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
void CoreLoop(ALLEGRO_DISPLAY *display, TileMap mapTileIndexes, ViewWindow win1) {
	ALLEGRO_EVENT_QUEUE *event_queue = nullptr;
	ALLEGRO_EVENT ev;// only allocates memory - does not initialize contents
	event_queue = al_create_event_queue();
	al_register_event_source(event_queue, al_get_keyboard_event_source());

	int dx = 0 , dy = 0, i = 0, j = 0, displayXKIVOS = 0, displayYKIVOS = 0;
	int zachSteps = 4;
	Rect KIVOS = Rect(0,0,16,16);
	a =  TileLayer("CSVMaps/mario1.csv");
	a.tileset = al_load_bitmap("Tiles/super_mario_tiles.png");
	a.TileLayerBitmap = al_create_bitmap(TILE_HEIGHT, TILE_WIDTH);
	AnimationFilmHolder::Get() = InitAnimationFilmHolder();

	MovingAnimation* test = new MovingAnimation("littlemario.walk.right", 0, 0, 0, 100000);
	MovingAnimator* kounaw = new MovingAnimator();
	kounaw->Start(test, SystemClock::Get().micro_secs());
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
				AnimationFilmHolder::Get().GetFilm("littlemario.walk.left")->DisplayFrame(a.TileLayerBitmap, Point(0, 0), j);
				dy += zachSteps;
			}
			else if (ev.keyboard.keycode == ALLEGRO_KEY_UP) {

				dy -= zachSteps;
			}
			else if (ev.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
				dx += zachSteps;
			}
			else if (ev.keyboard.keycode == ALLEGRO_KEY_LEFT) {
				//j++;
				//j = j % 4;
				dx -= zachSteps;
			}

			myGrid.FilterGridMotion(myGrid.gridTileStatus, KIVOS, dx, dy);;
			myTile.ScrollWithBoundCheck(win1.dimensions, dx, dy, myTile.TileMapIndexes);

			displayXKIVOS = KIVOS.x - win1.dimensions.x;
			displayYKIVOS = KIVOS.y - win1.dimensions.y;

			dx = dy = 0;
		}
		al_flush_event_queue(event_queue);
		
		myTile.TileTerrainDisplay(mapTileIndexes, win1.camera, win1.dimensions, win1.displayArea);
		al_flip_display();
		AnimatorManager::GetSingleton().Progress(SystemClock::Get().micro_secs());
		al_set_target_bitmap(al_get_backbuffer(display));
		al_draw_scaled_bitmap(win1.camera, 0, 0, WIDTH/3, HEIGHT/3, 0, 0, WIDTH, HEIGHT, 0);
		al_draw_scaled_bitmap(a.TileLayerBitmap, 0, 0, WIDTH / 3, HEIGHT / 3, displayXKIVOS *3, displayYKIVOS *3, WIDTH, HEIGHT, 0);
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
	ViewWindow win1 = ViewWindow(WIDTH/3, HEIGHT/3, 0, 0,
									0, 0, 0, 0);
	//The Original Super Mario Bros Game Loop (but we call it CoreLoop.)
	CoreLoop(display, mapTileIndexes, win1);
	

	//destroyAllegroComponents --> make that a delegate
	[=](){
		al_destroy_bitmap(myTile.tileset);
		al_destroy_display(display);
	};
	
	return 0;
}
#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>
#include <iostream>
#include "Tiles.h"

#define WIDTH	640
#define	HEIGHT	480



void initViewWindow(
	int dimHeight, int dimWidth, int dimX, int dimY,
	int dispHeight, int dispWidth, int dispX, int dispY,
	ViewWindow& win
){
	win = ViewWindow(
		Rect( dimX, dimY, dimHeight, dimWidth),
		Rect(dispX, dispY, dispHeight, dispWidth)
	);
}


void CoreLoop(ALLEGRO_DISPLAY *display, TileMap mapTileIndexes, ViewWindow win1) {
	ALLEGRO_EVENT_QUEUE *event_queue = nullptr;
	event_queue = al_create_event_queue();
	al_register_event_source(event_queue, al_get_keyboard_event_source());

	int dx = 0 , dy = 0;
	int zachSteps = 2;
	while (1) {
		ALLEGRO_EVENT ev;
		al_wait_for_event(event_queue, &ev);
		if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
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

			ScrollWithBoundCheck(win1.dimensions, dx, dy);

			dx = dy = 0;
		}
		TileTerrainDisplay(mapTileIndexes, win1.camera, win1.dimensions, win1.displayArea);
		al_set_target_bitmap(al_get_backbuffer(display));
		al_draw_scaled_bitmap(win1.camera, 0, 0, 160, 160, 0, 0, 160 * 3, 160 * 3, 0);
		al_flip_display();
	}

	//destroyAllegroComponents --> make that a delegate
	[=]() {
		al_destroy_event_queue(event_queue);
	};
}


int main() {
	ALLEGRO_DISPLAY *display;
	TileMapIndexes = getTileMapIDs("CSVMaps/mario1.csv");
	int mapColumns, mapRows;
	mapRows = TileMapIndexes.size();
	mapColumns = TileMapIndexes[0].size();

	if(!al_init())
		return -1;
	display = al_create_display(WIDTH, HEIGHT);

	//allegro addons
	al_install_keyboard();
	al_init_image_addon();

	tileset = al_load_bitmap("Tiles/super_mario_tiles.png");
	map = al_create_bitmap(mapRows*TILE_HEIGHT, mapColumns*TILE_WIDTH);

	//mapping map indexes to tilesetIndexes
	TileMap mapTileIndexes;
	getMapIndexes(mapTileIndexes, TileMapIndexes);

	//initializing view window
	ViewWindow win1;
	initViewWindow(
		160, 160, 0, 0,
		0, 0, 0, 0,
		win1
	);

	//The Original Super Mario Bros Game Loop (but we call it CoreLoop.)
	CoreLoop(display, mapTileIndexes, win1);
	

	//destroyAllegroComponents --> make that a delegate
	[=](){
		al_destroy_bitmap(tileset);
		al_destroy_display(display);
	};
	
	return 0;
}
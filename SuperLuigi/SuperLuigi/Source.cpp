#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>
#include <iostream>
#include "Tiles.h"
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

#define WIDTH	1024
#define	HEIGHT	768

std::vector<std::vector<int>> getTileMapIDs(std::string filename) {
	std::string line;
	std::vector<std::vector<int>> TileIDs;
	std::ifstream input(filename);

	int id;
	if (input.is_open())
		while (std::getline(input, line)) {
			std::vector<int> row;
			std::stringstream input_stringstream(line);
			while (getline(input_stringstream, line, ',')) {
				row.push_back(std::stoi(line));
			}
			TileIDs.push_back(row);
		}
	return TileIDs;
}

int getColFromID(int ID, int TileSetWidth) {
	return ID % TileSetWidth;
}

int getRowFromID(int ID, int TileSetWidth) {
	return ID / TileSetWidth;
}


int main() {
	ALLEGRO_DISPLAY *display;
	ALLEGRO_BITMAP* image = nullptr;
	ALLEGRO_EVENT_QUEUE *event_queue = nullptr;

	ALLEGRO_BITMAP* tileset = nullptr;
	ALLEGRO_BITMAP* screenBitmap = nullptr;

	std::vector<std::vector<int>> TileMapIndexes = getTileMapIDs("CSVMaps/mario.csv");

	if(!al_init())
		return -1;

	display = al_create_display(WIDTH, HEIGHT);

	if (!display)
		return 1;

	//allegro addons
	al_install_keyboard();
	al_init_image_addon();

	image = al_load_bitmap("Tiles/super_mario_tiles.png");
	screenBitmap = al_create_bitmap(WIDTH, HEIGHT);

	if (!image)
		return -1;

	event_queue = al_create_event_queue();
	al_register_event_source(event_queue, al_get_keyboard_event_source());

	while (1) {
		ALLEGRO_EVENT ev;
		al_wait_for_event(event_queue,&ev);
		if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
			if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
				break;
			}
		}
		for (int i = 0; i < TileMapIndexes.size(); i++) {
			for (int j = 0; j<TileMapIndexes[i].size(); j++) {
				PutTile(screenBitmap, i*16, j*16, image, MakeIndex2(getRowFromID(TileMapIndexes[j][i], 19), getColFromID(TileMapIndexes[j][i], 19)));
			}
		}

		al_set_target_bitmap(al_get_backbuffer(display));
		al_draw_scaled_bitmap(screenBitmap, 0, 0, WIDTH, HEIGHT, 0, 0, WIDTH * TILE_SCALE, HEIGHT * TILE_SCALE,0);
		al_flip_display();
	}

	al_destroy_bitmap(image);
	al_destroy_event_queue(event_queue);
	al_destroy_display(display);

	return 0;
}
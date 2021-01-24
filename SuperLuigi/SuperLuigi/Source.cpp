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
GridLayer myGrid;
TileLayer myTile;
CameraMover cameraMover;
Sprite marioSprite;
Sprite luigiSprite;
Clipper clipper;
Rect cameraCoords;

Mario *supermario;
MovingEntity *goomba;
MysteryTile* mtile1;

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

void initializeAnimationsAndSprites() {
	//Initialize AnimationFilms
	AnimationFilmHolder::Get() = InitAnimationFilmHolder();
	
	//Clipper
	clipper = Clipper();
	clipper = MakeTileLayerClipper(&myTile);
	
	//Mario
	supermario = new Mario(&myGrid, &(myTile.viewWin.dimensions));

	//Goomba
	goomba = new MovingEntity(50, 50, AnimationFilmHolder::Get().GetFilm("goomba.walk"), "goomba", &myGrid);

	//MysteryTile
	mtile1 = new MysteryTile(&myGrid);

	std::function<void(Sprite* s1, Sprite* s2)> f = [](Sprite* s1, Sprite* s2) {
		std::cout << "Bumped";
	};
	
	//Collisions
	CollisionChecker::GetSingleton().Register(
		supermario->GetSelf(),
		&(mtile1->GetSprite()),
		mtile1->GetOnCollison()
	);

	cameraMover = CameraMover(&myTile, supermario->GetSelf(), startPosX);
}



void CoreLoop(ALLEGRO_DISPLAY *display, TileMap mapTileIndexes) {
	ALLEGRO_EVENT_QUEUE *event_queue = nullptr;
	
	ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60);

	event_queue = al_create_event_queue();
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	al_register_event_source(event_queue, al_get_timer_event_source(timer));

	int dx = 0, dy = 0;
	bool draw = true;
	Direction direction = NO;

	ALLEGRO_BITMAP* beforeScaleBitmap = al_create_bitmap(WIDTH, HEIGHT);
	ALLEGRO_KEYBOARD_STATE	keyboardState;
	al_start_timer(timer);
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
			myTile.TileTerrainDisplay(mapTileIndexes, myTile.viewWin.bitmap, myTile.viewWin.dimensions, myTile.viewWin.displayArea);
			//calculate ending position
			dx = supermario->GetSelf()->GetBox().x - startPosX;
			dy = supermario->GetSelf()->GetBox().y - startPosY;
			
			//move camera
			cameraMover.ScrollAccordingToCharacter(dx, dy);

			BitmapBlit(myTile.viewWin.bitmap, Rect( 0, 0, myTile.viewWin.dimensions.w, myTile.viewWin.dimensions.h), beforeScaleBitmap, Point(0, 0));			
			
			//display sprites
			mtile1->GetSprite().Display(beforeScaleBitmap, cameraCoords, clipper);
			supermario->GetSelf()->Display(beforeScaleBitmap, cameraCoords, clipper);
			goomba->GetSelf()->Display(beforeScaleBitmap, cameraCoords, clipper);

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
	myTile = TileLayer("CSVMaps/mario2.csv");
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
	myTile.getMapIndexes(mapTileIndexes, myTile.TileMapIndexes);

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
	//The Original Super Mario Bros Game Loop (but we call it CoreLoop.)
	CoreLoop(display, mapTileIndexes);
	
	//destroyAllegroComponents --> make that a delegate
	[=](){
		al_destroy_bitmap(myTile.tileset);
		al_destroy_display(display);
	};
	
	return 0;
}
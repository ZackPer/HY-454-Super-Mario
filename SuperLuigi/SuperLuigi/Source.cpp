#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>
#include <iostream>

#define WIDTH	1024
#define	HEIGHT	768

int main() {
	ALLEGRO_DISPLAY *display;
	ALLEGRO_BITMAP	*image = NULL;
	ALLEGRO_EVENT_QUEUE *event_queue = NULL;

	if(!al_init())
		return -1;

	display = al_create_display(WIDTH, HEIGHT);

	if (!display)
		return 1;

	//allegro addons
	al_install_keyboard();
	al_init_image_addon();

	image = al_load_bitmap("god_help_us.png");

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

		al_draw_bitmap(image, al_get_bitmap_width(image)/2, al_get_bitmap_height(image) / 2,0);
		al_flip_display();
	}

	al_destroy_bitmap(image);
	al_destroy_event_queue(event_queue);
	al_destroy_display(display);

	return 0;
}
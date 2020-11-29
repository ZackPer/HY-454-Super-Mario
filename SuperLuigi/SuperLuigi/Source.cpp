#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <iostream>

#define WIDTH	1024
#define	HEIGHT	768

int main() {
	ALLEGRO_DISPLAY *display;

	display = al_create_display(WIDTH, HEIGHT);

	system("pause");

	return 0;
}
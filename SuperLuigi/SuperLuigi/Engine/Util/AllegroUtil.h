#pragma once
#include "../Types/Types.h"
void BitmapBlit(ALLEGRO_BITMAP* sourceBitmap, Rect sourceRect, ALLEGRO_BITMAP* destinationBitmap, Point destPoint) {
	al_set_target_bitmap(destinationBitmap); //dbitmap
	al_draw_bitmap_region(
		sourceBitmap,	//sbitmap
		sourceRect.x,	//sx
		sourceRect.y,	//sy
		sourceRect.w,	//sw
		sourceRect.h,	//sh
		destPoint.x,	//dx
		destPoint.y,	//dy
		0				//flags
	);
}

void MaskedBlit(ALLEGRO_BITMAP* sourceBitmap, Rect sourceRect, ALLEGRO_BITMAP* destinationBitmap, Point destPoint) {
	al_set_target_bitmap(destinationBitmap);
	//al_clear_to_color(al_map_rgba(0, 0, 0, 0));
	BitmapBlit(sourceBitmap, sourceRect, destinationBitmap, destPoint);
}

void TintedBlit(ALLEGRO_BITMAP* sourceBitmap, Rect sourceRect, ALLEGRO_BITMAP* destinationBitmap, Point destPoint, ALLEGRO_COLOR color) {
	al_set_target_bitmap(destinationBitmap);
	al_draw_tinted_bitmap_region(
		sourceBitmap,
		color,
		sourceRect.x, sourceRect.y, sourceRect.w, sourceRect.h,
		destPoint.x, destPoint.y,
		0
	);
}
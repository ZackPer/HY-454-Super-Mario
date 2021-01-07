#pragma once
#include "Types.h"
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
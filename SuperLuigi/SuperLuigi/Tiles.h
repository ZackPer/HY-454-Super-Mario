#pragma once

#include <allegro5/allegro.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

#include "Types.h"

enum BitDepth { bits8 = 1, bits16, bits24, bits32 };

#define ROW_MASK 0x0F
#define COL_MASK 0xF0
#define COL_SHIFT 4


byte MakeIndex(byte row, byte col)
{
	return (col << COL_SHIFT) | row;
}
byte GetCol(byte index)
{
	return index >> COL_SHIFT;
}
byte GetRow(byte index)
{
	return index & ROW_MASK;
}

#define MUL_TILE_WIDTH(i) ((i)<<4)
#define MUL_TILE_HEIGHT(i) ((i)<<4)
#define DIV_TILE_WIDTH(i) ((i)>>4)
#define DIV_TILE_HEIGHT(i) ((i)>>4)
#define MOD_TILE_WIDTH(i) ((i)&15)
#define MOD_TILE_HEIGHT(i) ((i)&15)

#define TILEX_MASK 0xFF00
#define TILEX_SHIFT 8
#define TILEY_MASK 0x00FF

Dim TileX3(Index index) { return index >> TILEX_SHIFT; }
Dim TileY3(Index index) { return index & TILEY_MASK; }
	

Index MakeIndex2(byte row, byte col)
{
	return (MUL_TILE_WIDTH(col) << TILEX_SHIFT) | MUL_TILE_HEIGHT(row);
}

#define TILE_SCALE 2


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


void PutTile(ALLEGRO_BITMAP* dest, Dim x, Dim y, ALLEGRO_BITMAP* tiles, Index tile) {
	Rect sourceRect = { TileX3(tile), TileY3(tile), TILE_WIDTH, TILE_HEIGHT };
	Point destPoint = {x, y};
	BitmapBlit(tiles, sourceRect, dest, destPoint);
}



Index GetTile(TileMap TileMapIndexes, Dim col, Dim row)
{
	return TileMapIndexes[row][col];
}

ALLEGRO_BITMAP* map = nullptr;
TileMap TileMapIndexes;
ALLEGRO_BITMAP* dpyBuffer = nullptr;
ALLEGRO_BITMAP* tileset = nullptr;
bool dpyChanged = true;
Dim dpyX = 0, dpyY = 0;

void TileTerrainDisplay(TileMap tileMapIndexes, ALLEGRO_BITMAP* dest, const Rect& viewWin, const Rect& displayArea) {
	if (dpyChanged) {
		auto startCol = DIV_TILE_WIDTH(viewWin.x);
		auto startRow = DIV_TILE_HEIGHT(viewWin.y);
		auto endCol = DIV_TILE_WIDTH(viewWin.x + viewWin.w - 1);
		auto endRow = DIV_TILE_HEIGHT(viewWin.y + viewWin.h - 1);
		dpyX = MOD_TILE_WIDTH(viewWin.x);
		dpyY = MOD_TILE_WIDTH(viewWin.y);
		dpyChanged = false;
		dpyBuffer = al_create_bitmap(((endCol - startCol)+1) * 16, ((endRow - startRow) + 1)* 16);
		for (Dim row = startRow; row <= endRow; ++row)
			for (Dim col = startCol; col <= endCol; ++col)
				PutTile(
					dpyBuffer,
					MUL_TILE_WIDTH(col - startCol),
					MUL_TILE_HEIGHT(row - startRow),
					tileset,
					GetTile(tileMapIndexes, col, row)
				);
	}
	BitmapBlit(
		dpyBuffer,
		{ dpyX, dpyY , viewWin.w, viewWin.h },
		dest,
		{ displayArea.x, displayArea.y }
	);

}

void SetTile(TileMap* m, Dim col, Dim row, Index index)
{
	(*m)[row][col] = index;
}

void WriteTextMap(const TileMap, FILE fp) {
	/// custom write in text format /
}


int getColFromID(int ID, int TileSetWidth) {
	return ID % TileSetWidth;
}

int getRowFromID(int ID, int TileSetWidth) {
	return ID / TileSetWidth;
}

TileMap getTileMapIDs(std::string filename) {
	std::string line;
	std::vector<std::vector<int>> TileIDs;
	TileMap tileIndexes;
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
	for (int i = 0; i < TileIDs.size(); i++) {
		std::vector<Index> a;
		tileIndexes.push_back(a);
		for (int j = 0; j < TileIDs[i].size(); j++) {
			tileIndexes[i].push_back(MakeIndex2(getRowFromID(TileIDs[i][j], 19), getColFromID(TileIDs[i][j], 19)));
		}
	}
	return tileIndexes;
}


void getMapIndexes(TileMap& mapTileIndexes, TileMap tileMapIndexes) {
	for (int i = 0; i < tileMapIndexes.size(); i++) {
		std::vector<Index> a;
		mapTileIndexes.push_back(a);
		for (int j = 0; j < tileMapIndexes[i].size(); j++) {
			PutTile(map, j * 16, i * 16, tileset, tileMapIndexes[i][j]);
			mapTileIndexes[i].push_back(tileMapIndexes[i][j]);
		}
	}
}

int GetMapPixelWidth(void) {
	return TileMapIndexes[0].size() * TILE_WIDTH;
}

int GetMapPixelHeight(void) {
	return TileMapIndexes.size() * TILE_HEIGHT;
}


void Scroll(Rect& viewWin, int dx, int dy) {
	viewWin.x += dx;
	viewWin.y += dy;
}

bool CanScrollHoriz(const Rect& viewWin, int dx) {
	return viewWin.x >= -dx && (viewWin.x + viewWin.w + dx) <= GetMapPixelWidth();
}

bool CanScrollVert(const Rect& viewWin, int dy) {
	return viewWin.y >= -dy && (viewWin.y + viewWin.h + dy) <= GetMapPixelHeight();
}

static void FilterScrollDistance(
	int		viewStartCoord,  // x or y
	int		viewSize,		// w or h
	int&	d,			   // dx or dy
	int		maxMapSize    // w or h
) {
	auto val = d + viewStartCoord;
	if (val < 0)
		d = -viewStartCoord;
	else
		if ((val + viewSize) >= maxMapSize)
			d = maxMapSize - (viewStartCoord + viewSize);
}

void FilterScroll(const Rect& viewWin, int& dx, int& dy) {
	FilterScrollDistance(viewWin.x, viewWin.w, dx, GetMapPixelWidth());
	FilterScrollDistance(viewWin.y, viewWin.h, dy, GetMapPixelHeight());
}

void ScrollWithBoundCheck(
	Rect& viewWin,
	int&   dx,
	int&   dy
) {
	FilterScroll(viewWin, dx, dy);
	Scroll(viewWin, dx, dy);
	//we might change the dpychanged var for cahcing
	dpyChanged = true;
}
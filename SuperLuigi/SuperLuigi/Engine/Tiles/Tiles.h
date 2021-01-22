#pragma once

#include <allegro5/allegro.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

#include "../Types/Types.h"
#include "../Util/AllegroUtil.h"


enum BitDepth { bits8 = 1, bits16, bits24, bits32 };

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


class TileLayer {
public:
	TileMap TileMapIndexes;
	ALLEGRO_BITMAP* TileLayerBitmap;
	ALLEGRO_BITMAP* tileset;
	ALLEGRO_BITMAP* dpyBuffer = nullptr;
	ViewWindow viewWin;
	bool dpyChanged = true;
	Dim dpyX = 0, dpyY = 0;

	TileLayer() {
		TileLayerBitmap = nullptr;
		tileset = nullptr;
	}

	int GetMaxHeight() {
		return maxHeight;
	}

	int GetMaxWidth() {
		return maxWidth;
	}

	TileLayer(std::string filename) {
		this->TileMapIndexes = this->getTileMapIDs(filename);
		TileLayerBitmap = nullptr;
		tileset = nullptr;
		maxHeight = TileMapIndexes.size()*16;
		maxWidth = TileMapIndexes[0].size()*16;
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

	void ScrollWithBoundCheck(
		Rect& viewWin,
		int&   dx,
		int&   dy,
		TileMap tileMapIndexes
	) {
		FilterScroll(viewWin, dx, dy, tileMapIndexes);
		Scroll(viewWin, dx, dy);
		//we might change the dpychanged var for cahcing
		dpyChanged = true;
	}


	bool CanScrollHoriz(const Rect& viewWin, int dx, TileMap tileMapIndexes) {
		return viewWin.x >= -dx && (viewWin.x + viewWin.w + dx) <= GetMapPixelWidth(tileMapIndexes);
	}

	bool CanScrollVert(const Rect& viewWin, int dy, TileMap tileMapIndexes) {
		return viewWin.y >= -dy && (viewWin.y + viewWin.h + dy) <= GetMapPixelHeight(tileMapIndexes);
	}

	void PutTile(ALLEGRO_BITMAP* dest, Dim x, Dim y, ALLEGRO_BITMAP* tiles, Index tile) {
		Rect sourceRect = { TileX3(tile), TileY3(tile), TILE_WIDTH, TILE_HEIGHT };
		Point destPoint = { x, y };
		BitmapBlit(tiles, sourceRect, dest, destPoint);
	}

	void getMapIndexes(TileMap& mapTileIndexes, TileMap tileMapIndexes) {
		for (int i = 0; i < tileMapIndexes.size(); i++) {
			std::vector<Index> a;
			mapTileIndexes.push_back(a);
			for (int j = 0; j < tileMapIndexes[i].size(); j++) {
				PutTile(this->TileLayerBitmap, j * 16, i * 16, this->tileset, tileMapIndexes[i][j]);
				mapTileIndexes[i].push_back(tileMapIndexes[i][j]);
			}
		}
	}

	void TileTerrainDisplay(TileMap tileMapIndexes, ALLEGRO_BITMAP* dest, const Rect& viewWin, const Rect& displayArea) {
		if (dpyChanged) {
			auto startCol = DIV_TILE_WIDTH(viewWin.x);
			auto startRow = DIV_TILE_HEIGHT(viewWin.y);
			auto endCol = DIV_TILE_WIDTH(viewWin.x + viewWin.w - 1);
			auto endRow = DIV_TILE_HEIGHT(viewWin.y + viewWin.h - 1);
			dpyX = MOD_TILE_WIDTH(viewWin.x);
			dpyY = MOD_TILE_WIDTH(viewWin.y);
			dpyChanged = false;
			dpyBuffer = al_create_bitmap(((endCol - startCol) + 1) * 16, ((endRow - startRow) + 1) * 16);
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

	Rect GetViewWindow() {
		return viewWin.dimensions;
	}

	void ScrollAccordingToCharacter(Rect& viewWin, int xCharacter, int yCharacter) {
		dpyChanged = true;
		viewWin.x += xCharacter;
		viewWin.y += yCharacter;
		if (viewWin.x >= 100)
			viewWin.x = 100;
		if (viewWin.y >= viewWin.h)
			viewWin.y = viewWin.h;
	}

private:
	int maxHeight, maxWidth;

	Index MakeIndex2(byte row, byte col)
	{
		return (MUL_TILE_WIDTH(col) << TILEX_SHIFT) | MUL_TILE_HEIGHT(row);
	}

	int getColFromID(int ID, int TileSetWidth) {
		return ID % TileSetWidth;
	}

	int getRowFromID(int ID, int TileSetWidth) {
		return ID / TileSetWidth;
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

	void FilterScroll(const Rect& viewWin, int& dx, int& dy, TileMap tileMapIndexes) {
		FilterScrollDistance(viewWin.x, viewWin.w, dx, GetMapPixelWidth(tileMapIndexes));
		FilterScrollDistance(viewWin.y, viewWin.h, dy, GetMapPixelHeight(tileMapIndexes));
	}

	void Scroll(Rect& viewWin, int dx, int dy) {
		viewWin.x += dx;
		viewWin.y += dy;
	}

	int GetMapPixelWidth(TileMap tileMapIndexes) {
		return tileMapIndexes[0].size() * TILE_WIDTH;
	}

	int GetMapPixelHeight(TileMap tileMapIndexes) {
		return tileMapIndexes.size() * TILE_HEIGHT;
	}



	Index GetTile(TileMap TileMapIndexes, Dim col, Dim row)
	{
		return TileMapIndexes[row][col];
	}

	
};

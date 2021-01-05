#pragma once

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

#include "Types.h"

extern TileLayer myTile;

#if TILE_WIDTH % GRID_ELEMENT_WIDTH != 0
#error "TILE_WIDTH % GRID_ELEMENT_WIDTH must be zero!"
#endif
#if TILE_HEIGHT % GRID_ELEMENT_HEIGHT != 0
#error "TILE_HEIGHT % GRID_ELEMENT_HEIGHT must be zero!"
#endif
//==============================================================================
#define GRID_BLOCK_COLUMNS (TILE_WIDTH / GRID_ELEMENT_WIDTH)
#define GRID_BLOCK_ROWS (TILE_HEIGHT / GRID_ELEMENT_HEIGHT)
#define GRID_ELEMENTS_PER_TILE (GRID_BLOCK_ROWS * GRID_BLOCK_COLUMNS)
//==============================================================================
#define GRID_THIN_AIR_MASK 0x0000 // element is ignored
#define GRID_LEFT_SOLID_MASK 0x0001 // bit 0
#define GRID_RIGHT_SOLID_MASK 0x0002 // bit 1
#define GRID_TOP_SOLID_MASK 0x0004 // bit 2
#define GRID_BOTTOM_SOLID_MASK 0x0008 // bit 3
#define GRID_GROUND_MASK 0x0010 // bit 4, keep objects top / bottom (gravity)
#define GRID_FLOATING_MASK 0x0020 // bit 5, keep objects anywhere inside (gravity)
#define GRID_EMPTY_TILE GRID_THIN_AIR_MASK
#define GRID_SOLID_TILE \
(GRID_LEFT_SOLID_MASK | GRID_RIGHT_SOLID_MASK | GRID_TOP_SOLID_MASK | GRID_BOTTOM_SOLID_MASK)
//==============================================================================
#define MAX_PIXEL_WIDTH MUL_TILE_WIDTH(myTile.TileMapIndexes[0].size())
#define MAX_PIXEL_HEIGHT MUL_TILE_HEIGHT(myTile.TileMapIndexes.size())
#define DIV_GRID_ELEMENT_WIDTH(i) ((i)>>4)
#define DIV_GRID_ELEMENT_HEIGHT(i) ((i)>>4)
#define MUL_GRID_ELEMENT_WIDTH(i) ((i)<<4)
#define MUL_GRID_ELEMENT_HEIGHT(i) ((i)<<4)

std::vector<int> emptyMarioTiles = {
	4, 5, 6, 7, 19, 20, 21, 24, 25, 26, 27, 28, 38, 39, 40, 43, 44, 45, 46, 47, 57, 58, 
	59, 30, 32, 34, 49, 51, 53, 59, 62, 63, 64, 65, 66, 103, 104, 105, 106, 107, 108, 109,
	122, 123, 128, 129, 131, 143, 144, 147, 149, 160, 161, 162, 163, 164, 165, 167, 168, 169, 170, 179,
	180, 181, 182, 183, 184, 185, 197, 198, 199, 200, 201, 215, 216, 217, 218, 219, 220 
};

class GridLayer {
private:
	GridIndex GetGridTile(const GridMap m, Dim col, Dim row)
	{
		return (m)[row][col];
	}

	bool CanPassGridTile(GridMap m, Dim col, Dim row, GridIndex flags)
	{
		return (GetGridTile(m, col, row) == GRID_EMPTY_TILE); // & flags != 0;
	}

	bool ComputeIsGridIndexEmpty(Index tile) {
		int row = TileY3(tile) / 16;
		int col = TileX3(tile) / 16;
		int index = (row * 19) + col;
		for (auto& it : emptyTiles) {
			if (it == index)
				return true;
		}
		return false;
	}

	void ComputeTileGridBlocks(const TileMap map) {
		for (auto row = 0; row < map.size(); ++row) {
			std::vector<int> rower;
			gridTileStatus.push_back(rower);
			for (auto col = 0; col < map[0].size(); ++col) {
				gridTileStatus[row].push_back(
					ComputeIsGridIndexEmpty(myTile.TileMapIndexes[row][col]) ?
					GRID_EMPTY_TILE : GRID_SOLID_TILE
				);
			}
		}
	}

	void InitEmptyTiles() {
		this->emptyTiles = emptyMarioTiles;
	}

	void FilterGridMotionRight(GridMap m, Rect& r, int& dx) {
		int x2 = r.x + r.w - 1;
		int x2_next = x2 + dx;
		int t = MAX_PIXEL_WIDTH;
		if (x2_next >= t)
			dx = MAX_PIXEL_WIDTH - x2;
		else {
			auto newCol = DIV_GRID_ELEMENT_WIDTH(x2_next);
			auto currCol = DIV_GRID_ELEMENT_WIDTH(x2);
			
			if (newCol != currCol) {
				assert(newCol - 1 == currCol); // we really move right
				auto startRow = DIV_GRID_ELEMENT_HEIGHT(r.y);
				auto endRow = DIV_GRID_ELEMENT_HEIGHT(r.y + r.h - 1);
				for (auto row = startRow; row <= endRow; ++row)
					if (!CanPassGridTile(m, newCol, row, GRID_LEFT_SOLID_MASK)) {		
						dx = MUL_GRID_ELEMENT_WIDTH(newCol) - (x2 + 1);
						break;
					}
			}
		}
	}

	void FilterGridMotionDown(GridMap m, Rect& r, int& dy) {
		int y2 = r.y + r.h - 1;
		int y2_next = y2 + dy;
		int t = MAX_PIXEL_WIDTH;
		if (y2_next >= t)
			dy = MAX_PIXEL_WIDTH - y2;
		else {
			auto newRow = DIV_GRID_ELEMENT_HEIGHT(y2_next);
			auto currRow = DIV_GRID_ELEMENT_HEIGHT(y2);
			if (newRow != currRow) {
				assert(newRow - 1 == currRow); // we really move right
				auto startCol = DIV_GRID_ELEMENT_WIDTH(r.x);
				auto endCol = DIV_GRID_ELEMENT_WIDTH(r.x + r.w - 1);
				for (auto col = startCol; col <= endCol; ++col)
					if (!CanPassGridTile(m, col, newRow, GRID_TOP_SOLID_MASK)) {
						dy = MUL_GRID_ELEMENT_HEIGHT(newRow) - (y2 + 1);
						break;
					}
			}
		}
	}

	void FilterGridMotionUp(GridMap m, Rect& r, int& dy) {
		auto y1_next = r.y + dy;
		if (y1_next < 0)
			dy = -r.y;
		else {
			auto newRow = DIV_GRID_ELEMENT_HEIGHT(y1_next);
			auto currRow = DIV_GRID_ELEMENT_HEIGHT(r.y);
			if (newRow != currRow) {
				assert(newRow + 1 == currRow); // we really move up
				auto startCol = DIV_GRID_ELEMENT_WIDTH(r.x);
				auto endCol = DIV_GRID_ELEMENT_WIDTH(r.x + r.w - 1);
				for (auto col = startCol; col <= endCol; ++col)
					if (!CanPassGridTile(m, col, newRow, GRID_BOTTOM_SOLID_MASK)) {
						dy = MUL_GRID_ELEMENT_HEIGHT(currRow) - r.y;
						break;
					}
			}
		}
	}

	void FilterGridMotionLeft(GridMap m, Rect& r, int& dx) {
		auto x1_next = r.x + dx;
		if (x1_next < 0)
			dx = -r.x;
		else {
			auto newCol = DIV_GRID_ELEMENT_WIDTH(x1_next);
			auto currCol = DIV_GRID_ELEMENT_WIDTH(r.x);
			if (newCol != currCol) {
				assert(newCol + 1 == currCol); // we really move left
				auto startRow = DIV_GRID_ELEMENT_HEIGHT(r.y);
				auto endRow = DIV_GRID_ELEMENT_HEIGHT(r.y + r.h - 1);
				for (auto row = startRow; row <= endRow; ++row)
					if (!CanPassGridTile(m, newCol, row, GRID_RIGHT_SOLID_MASK)) {
						dx = MUL_GRID_ELEMENT_WIDTH(currCol) - r.x;
						break;
					}
			}
		}
	}


public:
	std::vector<int> emptyTiles;
	GridMap  gridTileStatus;

	GridLayer() {
		InitEmptyTiles();
	}

	GridLayer(TileMap tileMap) {
		InitEmptyTiles();
		ComputeTileGridBlocks(tileMap);
	}

	//for gravity TODO later
	bool IsOnSolidGround(GridMap m, Rect& r) {
		int dy = 1; // down 1 pixel
		FilterGridMotionDown( m, r, dy);
		return dy == 0; // if true IS attached to solid ground
	}

	GridMap GetBuffer() {
		return gridTileStatus;
	}

	void FilterGridMotion(GridMap m, Rect& r, int& dx, int& dy) {
		assert(
			abs(dx) <= GRID_ELEMENT_WIDTH && abs(dy) <= GRID_ELEMENT_HEIGHT
		);
		// try horizontal move
		if (dx < 0)
			FilterGridMotionLeft(m, r, dx);
		else
			if (dx > 0)
				FilterGridMotionRight(m, r, dx);

		// try vertical move
		if (dy < 0)
			FilterGridMotionUp(m, r, dy);
		else
			if (dy > 0)
				FilterGridMotionDown(m, r, dy);
		r.x += dx;
		r.y += dy;
	}

	void DrawGrid(int rectangleScale) {
		for (auto row = 0; row < gridTileStatus.size(); row++) {
			for (auto col = 0; col < gridTileStatus[0].size(); col++) {
				float x1 = row * 16 * rectangleScale;
				float y1 = col * 16 * rectangleScale;
				float x2 = ((row * 16) + 16) *rectangleScale;
				float y2 = ((col * 16) + 16) *rectangleScale;
				if (gridTileStatus[row][col] == GRID_SOLID_TILE) {
					ALLEGRO_COLOR t = al_map_rgb(255, 0, 0);
					al_draw_rectangle(y1, x1, y2, x2, t, 3);
				}
				else {
					ALLEGRO_COLOR t = al_map_rgb(0, 0, 255);
					al_draw_rectangle(y1, x1, y2, x2, t, 3);
				}
			}
		}
	}

};
#pragma once

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

#include "Types.h"

#if TILE_WIDTH % GRID_ELEMENT_WIDTH != 0
#error "TILE_WIDTH % GRID_ELEMENT_WIDTH must be zero!"
#endif
#if TILE_HEIGHT % GRID_ELEMENT_HEIGHT != 0
#error "TILE_HEIGHT % GRID_ELEMENT_HEIGHT must be zero!"
#endif

#define GRID_BLOCK_COLUMNS (TILE_WIDTH / GRID_ELEMENT_WIDTH)
#define GRID_BLOCK_ROWS (TILE_HEIGHT / GRID_ELEMENT_HEIGHT)
#define GRID_ELEMENTS_PER_TILE (GRID_BLOCK_ROWS * GRID_BLOCK_COLUMNS)

void SetGridTile(GridMap* m, Dim col, Dim row, GridIndex index)
{
	(*m)[row][col] = index;
}
GridIndex GetGridTile(const GridMap m, Dim col, Dim row)
{
	return (m)[row][col];
}

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

void SetSolidGridTile(GridMap* m, Dim col, Dim row)
{
	SetGridTile(m, col, row, GRID_SOLID_TILE);
}
void SetEmptyGridTile(GridMap* m, Dim col, Dim row)
{
	SetGridTile(m, col, row, GRID_EMPTY_TILE);
}
void SetGridTileFlags(GridMap* m, Dim col, Dim row, GridIndex flags)
{
	SetGridTile(m, col, row, flags);
}
void SetGridTileTopSolidOnly(GridMap* m, Dim col, Dim row)
{
	SetGridTileFlags(m, row, col, GRID_TOP_SOLID_MASK);
}
bool CanPassGridTile(GridMap m, Dim col, Dim row, GridIndex flags)
{
	return (GetGridTile(m, col, row) == GRID_EMPTY_TILE); // & flags != 0;
}

std::vector<int> emptyTiles;
void InitEmptyTiles() {
	emptyTiles.push_back(4);
	emptyTiles.push_back(5);
	emptyTiles.push_back(6);
	emptyTiles.push_back(7);
	emptyTiles.push_back(19);
	emptyTiles.push_back(20);
	emptyTiles.push_back(21);
	emptyTiles.push_back(24);
	emptyTiles.push_back(25);
	emptyTiles.push_back(26);
	emptyTiles.push_back(27);
	emptyTiles.push_back(28);
	emptyTiles.push_back(38);
	emptyTiles.push_back(39);
	emptyTiles.push_back(40);
	emptyTiles.push_back(43);
	emptyTiles.push_back(44);
	emptyTiles.push_back(45);
	emptyTiles.push_back(46);
	emptyTiles.push_back(47);
	emptyTiles.push_back(57);
	emptyTiles.push_back(58);
	emptyTiles.push_back(59);
	emptyTiles.push_back(30);
	emptyTiles.push_back(32);
	emptyTiles.push_back(34);
	emptyTiles.push_back(49);
	emptyTiles.push_back(51);
	emptyTiles.push_back(53);
	emptyTiles.push_back(59);
	emptyTiles.push_back(62);
	emptyTiles.push_back(63);
	emptyTiles.push_back(64);
	emptyTiles.push_back(65);
	emptyTiles.push_back(66);
	emptyTiles.push_back(103);
	emptyTiles.push_back(104);
	emptyTiles.push_back(105);
	emptyTiles.push_back(106);
	emptyTiles.push_back(107);
	emptyTiles.push_back(108);
	emptyTiles.push_back(109);
	emptyTiles.push_back(122);
	emptyTiles.push_back(123);
	emptyTiles.push_back(128);
	emptyTiles.push_back(129);
	emptyTiles.push_back(131);
	emptyTiles.push_back(143);
	emptyTiles.push_back(144);
	emptyTiles.push_back(147);
	emptyTiles.push_back(149);
	emptyTiles.push_back(160);
	emptyTiles.push_back(161);
	emptyTiles.push_back(162);
	emptyTiles.push_back(163);
	emptyTiles.push_back(164);
	emptyTiles.push_back(165);
	emptyTiles.push_back(167);
	emptyTiles.push_back(168);
	emptyTiles.push_back(169);
	emptyTiles.push_back(170);
	emptyTiles.push_back(179);
	emptyTiles.push_back(180);
	emptyTiles.push_back(181);
	emptyTiles.push_back(182);
	emptyTiles.push_back(183);
	emptyTiles.push_back(184);
	emptyTiles.push_back(185);
	emptyTiles.push_back(197);
	emptyTiles.push_back(198);
	emptyTiles.push_back(199);
	emptyTiles.push_back(200);
	emptyTiles.push_back(201);
	emptyTiles.push_back(215);
	emptyTiles.push_back(216);
	emptyTiles.push_back(217);
	emptyTiles.push_back(218);
	emptyTiles.push_back(219);
	emptyTiles.push_back(220);
}

std::vector<std::vector<int>>  gridTileStatus;

Index GetTile(TileMap TileMapIndexes, Dim col, Dim row);

bool ComputeIsGridIndexEmpty(Index tile) {
	int row = TileY3(tile)/16;
	int col = TileX3(tile)/16;
	int index = (row * 19) + col;
	for (auto& it : emptyTiles) {
		if (it == index)
			return true;
	}

	return false;
}

void ComputeTileGridBlocks(const TileMap map) {
	InitEmptyTiles();
	for (auto row = 0; row < map.size(); ++row) {
		std::vector<int> rower;
		gridTileStatus.push_back(rower);
		for (auto col = 0; col < map[0].size(); ++col) {
			gridTileStatus[row].push_back( 
				ComputeIsGridIndexEmpty(TileMapIndexes[row][col]) ? 
				GRID_EMPTY_TILE : GRID_SOLID_TILE
			);
		}
	}
}

#define epi 3
void DrawGrid() {
	for (auto row = 0; row < gridTileStatus.size(); row++) {
		for (auto col = 0; col < gridTileStatus[0].size(); col++) {
			float x1 = row * 16 * epi;
			float y1 = col * 16 * epi;
			float x2 = ((row * 16) + 16) *epi;
			float y2 = ((col * 16) + 16) *epi;
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


#define MAX_PIXEL_WIDTH MUL_TILE_WIDTH(TileMapIndexes.size())
#define MAX_PIXEL_HEIGHT MUL_TILE_HEIGHT(TileMapIndexes[0].size())
#define DIV_GRID_ELEMENT_WIDTH(i) ((i)>>4)
#define DIV_GRID_ELEMENT_HEIGHT(i) ((i)>>4)
#define MUL_GRID_ELEMENT_WIDTH(i) ((i)<<4)
#define MUL_GRID_ELEMENT_HEIGHT(i) ((i)<<4)


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
}void FilterGridMotionRight(GridMap m, Rect& r, int& dx) {
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
}void FilterGridMotionDown(GridMap m, Rect& r, int& dy) {	int y2 = r.y + r.h - 1;
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
	}}void FilterGridMotionUp(GridMap m, Rect& r, int& dy) {	auto y1_next = r.y + dy;
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
	}}void FilterGridMotion(GridMap m, Rect& r, int& dx, int& dy) {
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
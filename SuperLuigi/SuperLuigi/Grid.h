#pragma once

#include <allegro5/allegro.h>
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
GridIndex GetGridTile(const GridMap* m, Dim col, Dim row)
{
	return (*m)[row][col];
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
bool CanPassGridTile(GridMap* m, Dim col, Dim row, GridIndex flags)
{
	return GetGridTile(m, row, col) & flags != 0;
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


Index GetTile(TileMap TileMapIndexes, Dim col, Dim row);

bool IsTileIndexAssumedEmpty(int row, int col) {
	int index = row * 19 + col;
	
	for (auto& it : emptyTiles) {
		if (it == index)
			return true;
	}

	return false;
}

void ComputeTileGridBlocks1(const TileMap* map, GridIndex* grid) {
	for (auto row = 0; row < (*map).size(); ++row)
		for (auto col = 0; col < (*map)[0].size(); ++col) {
			memset(
				grid,
				IsTileIndexAssumedEmpty(row, col) ?
				GRID_EMPTY_TILE :
				GRID_SOLID_TILE,
				GRID_ELEMENTS_PER_TILE
			);
			grid += GRID_ELEMENTS_PER_TILE;
		}
}
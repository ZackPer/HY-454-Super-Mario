#pragma once

#include <allegro5/allegro.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

//our header files
#include "Rect.h"
#include "Point.h"
#include "ViewWindow.h"

#define TILE_WIDTH	16
#define TILE_HEIGHT 16

#define GRID_ELEMENT_WIDTH 16
#define GRID_ELEMENT_HEIGHT 16

typedef unsigned short Dim;

typedef unsigned short Index;
typedef std::vector<std::vector<Index>> TileMap;
typedef unsigned char byte;

using GridIndex = byte;
typedef std::vector<std::vector<int>> GridMap ;  //GridMap[GRID_MAX_WIDTH][GRID_MAX_HEIGHT];
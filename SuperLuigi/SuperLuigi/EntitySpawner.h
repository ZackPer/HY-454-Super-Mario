#pragma once

#include <iostream>
#include <unordered_set>
#include <map>
#include <functional>

#include "./Engine/Sprite/Sprite.h"

// Singleton responsible of parsing/spawning the positions of the game enitities.
class EntitySpawner {
public:
	using PrimitiveCallback = std::function<SpriteEntity *(int x, int y)>;

	//Singleton Structure
	static EntitySpawner& Get() {
		static EntitySpawner singleton;
		return singleton;
	}
	EntitySpawner() = default;
	EntitySpawner(EntitySpawner const&) = delete;
	void operator=(EntitySpawner const&) = delete;

	void Add(int id, PrimitiveCallback primitive) {
		map.insert({ id, primitive});
	}

	// Prepares a list with all the spawn points of the known entities that exist in the tileLayer map.
	void ParseAndReplaceSpawnPoints(TileLayer &myTile, GridLayer &myGrid) {
		TileMap tileSetIndexMap = myTile.TileMapIndexes;
		for (int i = 0; i < tileSetIndexMap.size(); i++) {
			for (int j = 0; j < tileSetIndexMap[0].size(); j++) {
				Dim col = TileX3(tileSetIndexMap[i][j]) / TILE_WIDTH;
				Dim row = TileY3(tileSetIndexMap[i][j]) / TILE_HEIGHT;
				int index = row * 19 + col;
				tileSetIndexMap[i][j] = index;
			}
		}

		// Add all the sprite entities that have been 
		// registered to an unordered set for O(1) find.
		std::unordered_set<int> keySet;
		for (auto &mapIt : map)
			keySet.insert(mapIt.first);

		// Search in the spriteMap for IDs that exist in the 
		// unordered set and if they do add them to the spawnPointList.
		for (int i = 0; i < tileSetIndexMap.size(); i++) {
			for (int j = 0; j < tileSetIndexMap[i].size(); j++) {
				int tileLayerId = tileSetIndexMap[i][j];
				if (keySet.find(tileLayerId) != keySet.end()) {
					PrimitiveCallback primitiveCallback = map[tileLayerId];											
					spawnPoints.push_back({Point(i, j), primitiveCallback});
					MakeBrickTilesSolid(tileLayerId, i, j, myGrid);
					myTile.TileMapIndexes[i][j] = 20512; //Code for background sky
				}
			}
		}
	}

	void MakeBrickTilesSolid(int tileLayerId, int i, int j, GridLayer& myGrid) {
		if (tileLayerId == 216 || tileLayerId == 197 || tileLayerId == 199 || tileLayerId == 198)
			myGrid.gridTileStatus[i][j] = GRID_SOLID_TILE;
		//also add for brick that need to be kept in a map as well.
	}

	// Instantiates all sprites corresponding to their spawn positions
	void SpawnSprites() {
		for (auto &it : spawnPoints) {
			Point p = it.first;
			PrimitiveCallback callback = it.second;
			callback(p.y * 16, p.x * 16);
		}
	}

private:
	std::map<int, PrimitiveCallback> map;							// Maps primitive SpriteEntities to SpriteTileIDs
	std::vector<std::pair<Point, PrimitiveCallback>> spawnPoints;	// Contains cloned SpriteEntities and their spawning positions
};
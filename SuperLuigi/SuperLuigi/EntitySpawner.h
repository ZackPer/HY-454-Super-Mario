#pragma once

#include <iostream>
#include <unordered_set>
#include <map>

#include "./Engine/Sprite/Sprite.h"

// Singleton responsible of parsing/spawning the positions of the game enitities.
class EntitySpawner {
public:
	//Singleton Structure
	static EntitySpawner& Get() {
		static EntitySpawner singleton;
		return singleton;
	}
	EntitySpawner() = default;
	EntitySpawner(EntitySpawner const&) = delete;
	void operator=(EntitySpawner const&) = delete;

	enum SpriteIDs {
		MARIO = 196,
		LIFE_MUSHROOM = 197,
		BRICK = 198,
		STARMAN = 199,
		GOOMBA = 200,
		GREEN_KOOPA = 201,
		SUPER_MUSHROOM = 216,
		MYSTERY_BOX = 217,
		COIN = 218,
		PIRANHA = 219,
		RED_KOOPA = 220
	};
	std::map<SpriteIDs, SpriteEntity*> map;
	std::list<std::pair<Point, SpriteEntity*>> spawnPoints;

	void Add(int id, SpriteEntity *sprite) {
		map.insert({ SpriteIDs(id), sprite });
	}

	// Prepares a list with all the spawn points of the known entities that exist in the tileLayer map.
	void ParseSpawnPoints(std::vector<std::vector<Index>> enemyMap) {
		// Add all the sprite entities that have been 
		// registered to an unordered set for O(1) find.
		std::unordered_set<int> keySet;
		for (auto &mapIt : map)
			keySet.insert(mapIt.first);

		// Search in the enemyMap for IDs that exist in the 
		// unordered set and if they do add them to the spawnPointList.
		for (int i = 0; i < enemyMap.size(); i++) {
			for (int j = 0; j < enemyMap[i].size(); j++) {
				int tileLayerId = enemyMap[i][j];
				if (keySet.find(tileLayerId) != keySet.end()) {
					SpriteEntity *sprite = map[SpriteIDs(tileLayerId)];
					spawnPoints.push_back({Point(i, j), sprite});
				}
			}
		}
	}

	// Instantiates all sprites corresponding to their spawn positions
	void SpawnSprites() {
		for (auto &it : spawnPoints) {
			Point point = it.first;
			SpriteEntity *clone = it.second->Clone(point.y * 16, point.x * 16);
			clone->GetSelf()->SetVisibility(true);
		}
	}

private:
	

};
#pragma once
#include <iostream>
#include <assert.h>
#include <functional>
#include "Sprite.h"

class CollisionChecker final {
public:
	using Action = std::function<void(Sprite* s1, Sprite* s2)>;
	static CollisionChecker singleton;
protected:
	using Entry = std::tuple<Sprite*, Sprite*, Action>;
	std::list<Entry> entries;

	auto Find(Sprite* s1, Sprite* s2) -> std::list<Entry>::iterator {
		return std::find_if(
			entries.begin(),
			entries.end(),
			[s1, s2](const Entry& e) {
				return std::get<0>(e) == s1 && std::get<1>(e) == s2 ||
					std::get<0>(e) == s2 && std::get<1>(e) == s1;
			}
		);
	}

	bool IsInEntries(Sprite* s1, Sprite* s2){
		return !(entries.end() == Find(s1, s2));
	}

public:

	void Register(Sprite* s1, Sprite* s2, const Action& f)
	{
		assert(!IsInEntries(s1, s2));
		entries.push_back(std::make_tuple(s1, s2, f));
	}
	void Cancel(Sprite* s1, Sprite* s2) {
		entries.erase(Find(s1, s2));
	}
	void Check(void) const {
		for (auto& e : entries)
			if (std::get<0>(e)->CollisionCheck(std::get<1>(e)))
				std::get<2>(e)(std::get<0>(e), std::get<1>(e));
	}
	static auto GetSingleton(void) -> CollisionChecker&
	{
		return singleton;
	}
	static auto GetSingletonConst(void) -> const CollisionChecker&
	{
		return singleton;
	}
};

CollisionChecker CollisionChecker::singleton;
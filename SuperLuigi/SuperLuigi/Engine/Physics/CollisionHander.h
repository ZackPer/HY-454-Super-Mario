#pragma once
#include <iostream>
#include <assert.h>
#include <functional>
#include "../Sprite/Sprite.h"
#include <list>

class CollisionChecker final {
public:
	using Action = std::function<void(Sprite* s1, Sprite* s2)>;
	static CollisionChecker singleton;
protected:
	using Entry = std::tuple<Sprite*, Sprite*, Action>;
	std::list<Entry> entries;
	std::map<Sprite *, std::list<Sprite *>> spriteRelations;

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

	bool HasRelationList(Sprite *s) {
		auto it = spriteRelations.find(s);
		if (it == spriteRelations.end())
			return false;
		else
			return true;
	}
	void AddRelation(Sprite *s1, Sprite *s2) {
		if (!HasRelationList(s1))
			spriteRelations.insert({ s1, std::list<Sprite *>() });
		if (!HasRelationList(s2))
			spriteRelations.insert({ s2, std::list<Sprite *>() });
		
		spriteRelations[s1].push_back(s2);
		spriteRelations[s2].push_back(s1);
	}
	void RemoveRelation(Sprite *s1, Sprite *s2) {
		spriteRelations[s1].remove(s2);
		spriteRelations[s2].remove(s1);
	}

public:
	void Register(Sprite* s1, Sprite* s2, const Action& f)
	{
		assert(!IsInEntries(s1, s2));
		entries.push_back(std::make_tuple(s1, s2, f));
		AddRelation(s1, s2);
	}
	void Cancel(Sprite* s) {
		auto &relationList = spriteRelations[s];
		while (!relationList.empty()) {
			Cancel(s, relationList.front());
		}
	}
	void Cancel(Sprite* s1, Sprite* s2) {
		entries.erase(Find(s1, s2));
		RemoveRelation(s1, s2);
	}
	void Check(void) const {
		// Old good trustworthy way
		for (int i = entries.size(); i >= 0; i--) {
			if (entries.empty())
				return;

			auto &e = entries.back();
			if (std::get<0>(e)->CollisionCheck(std::get<1>(e)))
				std::get<2>(e)(std::get<0>(e), std::get<1>(e));
		}
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
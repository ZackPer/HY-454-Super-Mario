#pragma once

#include <iostream>
#include <functional>
#include "./Engine/Sprite/Sprite.h";
#include "./Engine/Physics/CollisionHander.h";
#include "./Engine/Animations/Animations.h";
#include "./Engine/Animations/Animators.h"
#include "./Engine/Animations/AnimationFilm.h"

#include "SpriteEntity.h"
#include "MovingEntity.h"
#include "Mario.h"

class EntityHolder {
public:
	using List = std::list<SpriteEntity*>;

	//Singleton Structure
	static EntityHolder& Get() {
		static EntityHolder singleton;
		return singleton;
	}
	EntityHolder() = default;
	EntityHolder(EntityHolder const&) = delete;
	void operator=(EntityHolder const&) = delete;
	
	List GetEntityList() {
		return entityList;
	}
	void Add(SpriteEntity *entity) {
		entityList.push_back(entity);
	}
	void Remove(SpriteEntity *entity) {
		entityList.remove(entity);
	}
	void ClearAll() {
		for (auto &it : entityList) {
			if (it != supermario)
				it->GetSelf()->SetVisibility(false);
		}
		entityList.clear();
	}
	SpriteEntity *GetSpriteEntity(Sprite *sprite) {
		for (auto &it : entityList)
			if (it->GetSelf() == sprite)
				return it;

		return NULL;
	}
	std::vector<SpriteEntity *> FindByType(std::string typeID) {
		std::vector<SpriteEntity *> results;
		for (auto &it : entityList) {	
			if (it->GetType() == typeID)
				results.push_back(it);
		}
		return results;
	}
	std::vector<SpriteEntity *> FindBySpriteTypeId(std::string typeID) {
		std::vector<SpriteEntity *> results;
		for (auto &it : entityList) {
			if (it->GetSelf()->GetTypeId() == typeID)
				results.push_back(it);
		}
		return results;
	}

	void SetSuperMario(Mario *supermario) {
		this->supermario = supermario;
	}
	Mario *GetSuperMario() {
		return supermario;
	}

private:
	List	entityList;
	Mario	*supermario;
};


DeathModule::DeathModule(Sprite* s) {
	hasFinished = false;
	dead = false;
	sprite = s;
}

void DeathModule::Die() {
	film = AnimationFilmHolder::Get().GetFilm("mario.death");
	animation = MovingAnimation(
		"mario.death",
		8,
		0,
		-5,
		30000
	);

	animator.SetOnAction(
		[=](Animator* animator, const Animation& anim) {
		sprite->SetHasDirectMotion(true).Move(((const MovingAnimation&)anim).GetDx(), ((const MovingAnimation&)anim).GetDy()).SetHasDirectMotion(true);
	}
	);

	animator.SetOnFinish(
		[=](Animator* animator) {
		animation = MovingAnimation(
			"mario.death",
			150,
			0,
			5,
			30000
		);

		((MovingAnimator*)animator)->Start(&animation, SystemClock::Get().micro_secs());
		animator->SetOnFinish(
			[=](Animator* animator) {
				dead = true;
				//hasFinished = true;
				EntityHolder::Get().GetSuperMario()->MoveToCheckPoint();
				EntityHolder::Get().GetSuperMario()->GetSelf()->Move(1, 1);
			}
		);

	}
	);


	sprite->SetCurrFilm(film);
	sprite->SetFrame(0);
	animator.Start(&animation, SystemClock::Get().micro_secs());
	
}
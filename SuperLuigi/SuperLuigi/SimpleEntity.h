#include <iostream>
#include "SpriteEntity.h"
#include "Engine/Sprite/Sprite.h"
#include "Engine/Animations/AnimationFilm.h"

class SimpleEntity : public SpriteEntity {
public:
	SimpleEntity(int x, int y, AnimationFilm *film, std::string typeId)
		: SpriteEntity(x, y, film, typeId)
	{
		
	}

private:

	void OnStartFalling() {

	}

	void OnStopFalling() {

	}
};
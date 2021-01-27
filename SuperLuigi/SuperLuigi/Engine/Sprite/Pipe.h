#pragma once

#include "Sprite.h"
#include "../Physics/CollisionHander.h"

class Pipe {
public:

	Pipe() = default;

	Pipe(Sprite* p) {
		pipe = p;
	}

	Pipe(int x, int y, AnimationFilm *film, const std::string& _typeId = "") {
		pipe = new Sprite(x, y, film, _typeId);
		pipe->SetVisibility(true);
		pipe->SetBoundingArea();
	}

	Sprite* GetSprite() {
		return pipe;
	}

private:
	Sprite* pipe;
};
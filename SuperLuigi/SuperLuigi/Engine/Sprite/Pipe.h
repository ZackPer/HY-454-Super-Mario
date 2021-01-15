#pragma once

#include "Sprite.h"

class Pipe {
public:

	Pipe() = default;

	Pipe(Sprite p) {
		pipe = p;
	}

	Pipe(int x, int y, AnimationFilm *film, const std::string& _typeId = "") {
		pipe = Sprite(x, y, film, _typeId);
	}
	
	int GetPositionX() {
		return pipe.GetBox().x;
	}

	int GetPositionY() {
		return pipe.GetBox().y;
	}

	int GetPositionWidth() {
		return pipe.GetBox().w;
	}

	int GetPositionHeight() {
		return pipe.GetBox().h;
	}

	Sprite GetSprite() {
		return pipe;
	}

	Sprite pipe;
private:
	
};
#pragma once
#include "Engine/Sprite/Sprite.h"
#include "Engine/Tiles/Tiles.h"
#include "Engine/Types/Types.h"
class CameraMover {
private:
	Sprite* sprite;
	TileLayer* tileLayer;
	int rightmost;

public:

	CameraMover(){}

	CameraMover(TileLayer* tl, Sprite* s, int startPos) {
		sprite = s;
		tileLayer = tl;
		rightmost = startPos + 32;
	}
	
	void XfilterMovement(int& dx){
		tileLayer->dpyChanged = true;
		int viewWindowEnd = tileLayer->viewWin.dimensions.x + tileLayer->viewWin.dimensions.w;
		if (sprite->GetBox().x > rightmost) {
			if (viewWindowEnd + dx >= tileLayer->GetMaxWidth()) {
				dx = tileLayer->GetMaxWidth() - viewWindowEnd;
				rightmost = viewWindowEnd;
			}	
			rightmost += dx;
		}
		else
			dx = 0;
	}
	void YfilterMovement(int& dy) const {
		int viewWindowEnd = tileLayer->viewWin.dimensions.y + tileLayer->viewWin.dimensions.h;
		tileLayer->dpyChanged = true;
		if (viewWindowEnd + dy > tileLayer->GetMaxHeight()) {
			tileLayer->dpyChanged = true;
			dy = tileLayer->GetMaxHeight() - viewWindowEnd;
		}	
	}

	void ScrollAccordingToCharacter(int& dx, int& dy) {
		XfilterMovement(dx);
		YfilterMovement(dy);
		tileLayer->viewWin.dimensions.x += dx;
		tileLayer->viewWin.dimensions.y += dy;
	}

	
};
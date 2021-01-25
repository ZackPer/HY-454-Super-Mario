#pragma once

#include <iostream>
#include <vector>
#include "Engine/Animations/Animations.h"
#include "Engine/Animations/Animators.h"
#include "Engine/Sprite/Sprite.h"
#include "Engine/SystemClock.h"

class MovementAI {
public:
	using OnSignChange = std::function<void(int)>;

	MovementAI() = default;
	MovementAI(Sprite *self, int speed) {
		this->self = self;
		this->speed = speed;
		movingAnimation = new MovingAnimation("goomba.walk", 0, 0, 0, 20000);
		movingAnimator = new MovingAnimator();
	}

	// Initialize mover and animator
	void Init(GridLayer *myGrid) {
		PrepareEdgeDetectionMover(myGrid);
		self->SetRange(1, 1);
		movingAnimator->SetOnAction(
			[=](Animator *animator, const Animation& animation) {
				int dx = speed * sign;
				int dy = 0;
				
				lastPos = self->GetBox();
				self->Move(speed * sign, 0);
				if (lastPos == self->GetBox()) { //That means it hit a wall
					sign *= -1;
					NotifyChange(sign);
				}
			}
		);
	}

	void Start() {
		movingAnimator->Start(movingAnimation, SystemClock::Get().micro_secs());
	}
	
	void Stop() {
		movingAnimator->Stop();
	}

	void PrepareEdgeDetectionMover(GridLayer *myGrid) {
		// For edge detection we need the reverse of the grid tile status map.
		// We cache it in reverseGridTileStatus so it is calculated once only.
		for (int i = 0; i < myGrid->gridTileStatus.size(); i++) {
			reverseGridTileStatus.push_back(std::vector<int>());
			for (int j = 0; j < myGrid->gridTileStatus[i].size(); j++)
				reverseGridTileStatus[i].push_back(myGrid->gridTileStatus[i][j] ? 0 : 1);
		}

		auto defaultMover = self->MakeSpriteGridLayerMover(myGrid, self);
		auto edgeDetectionMover = [=](Rect& box, int* dx, int *dy) {
			if (edgeDetection == true) {
				Rect edgeDetectionBox = box;
				int dummyY = 0;
				edgeDetectionBox.y += box.h;
				myGrid->FilterGridMotion(reverseGridTileStatus, edgeDetectionBox, *dx, dummyY);
			}

			defaultMover(box, dx, dy);
		};
		self->SetMover(edgeDetectionMover);
	}

	void SetEdgeDetection(bool edgeDetection) {
		this->edgeDetection = edgeDetection;
	}
	
	void SetOnSignChange(OnSignChange callback) {
		notifyChange = callback;
	}

private:
	Sprite				*self;
	MovingAnimation		*movingAnimation;
	MovingAnimator		*movingAnimator;
	OnSignChange		notifyChange;
	Rect				lastPos;
	bool				edgeDetection = false;
	int					sign = 1;
	int					speed = 1;
	std::vector<std::vector<int>> reverseGridTileStatus;


	void NotifyChange(int sign) {
		if (notifyChange)
			notifyChange(sign);
	}
};
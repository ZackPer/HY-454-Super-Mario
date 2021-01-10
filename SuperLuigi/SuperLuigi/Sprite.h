#pragma once
#include <iostream>
#include <assert.h>
#include <functional>
#include "Types.h"
#include "AnimationFilm.h"
#include "Clipper.h"
#include "Grid.h"
#include "GravityHandler.h"
#include "MotionQuantizer.h"

class BoundingArea {
};

class Sprite {
public:
	using Mover = std::function<void(Rect&, int* dx, int* dy)>;
protected:
	byte frameNo = 0;
	Rect frameBox; // inside the film
	int x = 0, y = 0;
	bool isVisible = false;
	AnimationFilm* currFilm = nullptr;
	BoundingArea* boundingArea = nullptr;
	unsigned zorder = 0;
	std::string typeId, stateId;
	Mover mover;
	MotionQuantizer quantizer;

public:
	bool directMotion = false;
	GravityHandler gravity;
	GravityHandler& GetGravityHandler(void)
	{
		return gravity;
	}
	Sprite& SetHasDirectMotion(bool v) { directMotion = true; return *this; }
	bool GetHasDirectMotion(void) const { return directMotion; }
	Sprite& Move(int dx, int dy) {
		if (directMotion) // apply unconditionally offsets!
			x += dx, y += dy;
		else {
			Rect box = GetBox();
			quantizer.Move(box, &dx, &dy);
			gravity.Check(box);
			x = box.x;
			y = box.y;
		}
		return *this;
	}
	template <typename Tfunc>
	void SetMover(const Tfunc& f)
	{
		quantizer.SetMover(mover = f);
	}
	Rect& GetBox(void) const
	{
		Rect box(x, y, frameBox.w, frameBox.h);
		return box;
	}
	void SetPos(int _x, int _y) { x = _x; y = _y; }
	void SetZorder(unsigned z) { zorder = z; }
	unsigned GetZorder(void) { return zorder; }	void SetFrame(byte i) {
		if (i != frameNo) {
			assert(i < currFilm->GetTotalFrames());
			frameBox = currFilm->GetFrameBox(frameNo = i);
		}
	}
	byte GetFrame(void) const { return frameNo; }
	void SetBoundingArea(const BoundingArea& area)
	{
		//assert(!boundingArea); boundingArea = area.Clone();
	}
	void SetBoundingArea(BoundingArea* area)
	{
		assert(!boundingArea); boundingArea = area;
	}
	auto GetBoundingArea(void) const -> const BoundingArea*
	{
		return boundingArea;
	}
	auto GetTypeId(void) -> const std::string& { return typeId; }
	void SetVisibility(bool v) { isVisible = v; }
	bool IsVisible(void) const { return isVisible; }
	bool CollisionCheck(const Sprite* s) const;
	void Display(ALLEGRO_BITMAP* dest, const Rect& dpyArea, const Clipper& clipper) const {
		Rect clippedBox;
		Point dpyPos;
		if (clipper.Clip(GetBox(), dpyArea, &dpyPos, &clippedBox)) {
			std::cout << "AA\n";
			Rect clippedFrame = Rect(
			frameBox.x + clippedBox.x,
			frameBox.y + clippedBox.y,
			clippedBox.w,
			clippedBox.h
			);
			MaskedBlit(
				currFilm->GetBitmap(),
				clippedFrame,
				dest,
				dpyPos
			);
		}
		else {
			MaskedBlit(
				currFilm->GetBitmap(),
				GetBox(),
				dest,
				dpyPos
			);
		}
	}

	const Mover MakeSpriteGridLayerMover(GridLayer* gridLayer, Sprite* sprite) {
		return [gridLayer, sprite](Rect& r, int* dx, int* dy) {
			// the r is actually awlays the sprite->GetBox():
			assert(r == sprite->GetBox());
			gridLayer->FilterGridMotion(gridLayer->gridTileStatus, r, *dx, *dy);
			if (*dx || *dy)
				sprite->SetHasDirectMotion(true).Move(*dx, *dy).SetHasDirectMotion(false);
		};
	};


	Sprite(int _x, int _y, AnimationFilm* film, const std::string& _typeId = "") :
		x(_x), y(_y), currFilm(film), typeId(_typeId)
	{		frameNo = currFilm->GetTotalFrames(); SetFrame(0);	}	Sprite() = default;};void PrepareSpriteGravityHandler(GridLayer* gridLayer, Sprite* sprite) {
	std::function<bool(Rect&)> function = [gridLayer](Rect& r){ return gridLayer->IsOnSolidGround(gridLayer->gridTileStatus, r); };
	sprite->GetGravityHandler().SetOnSolidGround(
		function
	);
}
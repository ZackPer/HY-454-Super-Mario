#pragma once
#include <iostream>
#include <assert.h>
#include <functional>
#include "../Types/Types.h"
#include "../Animations/AnimationFilm.h"
#include "Clipper.h"
#include "../Grid/Grid.h"
#include "../Physics/GravityHandler.h"
#include "../Physics/MotionQuantizer.h"
#include "../Physics/BoundingArea.h"

class SpriteManager;

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
	void setCurrFilm(AnimationFilm* film) {
		currFilm = film;
	}
	bool directMotion = false;
	GravityHandler gravity;
	GravityHandler& GetGravityHandler(void)
	{
		return gravity;
	}
	Sprite& SetHasDirectMotion(bool v) { directMotion = v; return *this; }
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
			((BoundingBox*)(boundingArea))->area->x = x;
			((BoundingBox*)(boundingArea))->area->y = y;
		}
		return *this;
	}

	template <typename Tfunc>
	void SetMover(const Tfunc& f)
	{
		quantizer.SetMover(mover = f);
	}
	void SetRange(int x, int y) {
		quantizer = quantizer.SetRange(x, y);
	}
	const Rect& GetBox(void) const
	{
		return Rect(x, y, frameBox.w, frameBox.h);
	}
	void SetPos(int _x, int _y) { x = _x; y = _y; }
	void SetZorder(unsigned z) { zorder = z; }
	unsigned GetZorder(void) { return zorder; }

	void SetFrame(byte i) {
		if (i != frameNo) {
			assert(i < currFilm->GetTotalFrames());
			frameBox = currFilm->GetFrameBox(frameNo = i);
		}
	}
	byte GetFrame(void) const { return frameNo; }
	void SetBoundingArea(const BoundingArea& area){
		assert(!boundingArea); 
		*boundingArea = area;
	}
	void SetBoundingArea(/*BoundingArea* area*/){
		/*assert(!boundingArea); boundingArea = area;*/
		boundingArea = (BoundingArea*)new BoundingBox();
		Rect rect = GetBox();
		*(((BoundingBox*) (boundingArea))->area) = rect;
	}
	auto GetBoundingArea(void) const -> const BoundingArea*{
		return boundingArea;
	}
	auto GetTypeId(void) -> const std::string& { 
		return typeId; 
	}
	void SetVisibility(bool v) { 
		isVisible = v; 
	}
	bool IsVisible(void) const { 
		return isVisible; 
	}
	bool CollisionCheck(const Sprite* s) const {
		return boundingArea->Intersects(s->GetBoundingArea());
	};
	void Display(ALLEGRO_BITMAP* dest, const Rect& dpyArea, const Clipper& clipper) const {
		Rect clippedBox;
		Point dpyPos;
		Rect r = GetBox();
		if (clipper.Clip(r, dpyArea, &dpyPos, &clippedBox)) {
			Rect clippedFrame = Rect(
			frameBox.x - clippedBox.x,
			frameBox.y - clippedBox.y,
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
	Sprite(int _x, int _y, AnimationFilm* film, const std::string& _typeId);

	
	Sprite() = default;
};


void PrepareSpriteGravityHandler(GridLayer* gridLayer, Sprite* sprite) {
	sprite->gravity.SetGravityAddicted(true);
	std::function<bool(Rect&)> function = [gridLayer](Rect& r){ 
		return gridLayer->IsOnSolidGround(gridLayer->gridTileStatus, r); 
	};
	sprite->GetGravityHandler().SetOnSolidGround(
		function
	);
}

class SpriteManager final {
public:
	using SpriteList = std::list<Sprite*>;
	using TypeLists = std::map<std::string, SpriteList>;
private:
	SpriteList dpyList;
	TypeLists types;
	static SpriteManager singleton;
public:
	void Add(Sprite* s) {
		auto found = types.find(s->GetTypeId());

		if (found == types.end())
			types.insert(std::pair<std::string, SpriteList>(s->GetTypeId(), SpriteList()));
		types.find(s->GetTypeId())->second.push_back(s);

		if(dpyList.empty())
			dpyList.push_back(s);
		else
			for (auto it = dpyList.begin(); it != dpyList.end(); ++it) {
				if ((*it)->GetZorder() == s->GetZorder()) {
					dpyList.insert(it, s);
				}
			}
	}
	void Remove(Sprite* s) {
		types.find(s->GetTypeId())->second.remove(s);
		dpyList.remove(s);
	}
	auto GetDisplayList(void) -> const SpriteList&
	{
		return dpyList;
	}
	auto GetTypeList(const std::string& typeId) -> const SpriteList&
	{
		return types[typeId];
	}
	static auto GetSingleton(void) -> SpriteManager&
	{
		return singleton;
	}
	static auto GetSingletonConst(void) -> const SpriteManager&
	{
		return singleton;
	}
};

SpriteManager SpriteManager::singleton;

Sprite::Sprite(int _x, int _y, AnimationFilm* film, const std::string& _typeId = "") :
	x(_x), y(_y), currFilm(film), typeId(_typeId)
	{
		frameNo = currFilm->GetTotalFrames();
		SetFrame(0);
		SpriteManager::GetSingleton().Add(this);
}
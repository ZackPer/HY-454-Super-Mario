#pragma once

#include <iostream>
#include <functional>
#include "Types.h"

namespace clip_util {
	template <class T> bool clip_rect(
		T x, T y, T w, T h,
		T wx, T wy, T ww, T wh,
		T* cx, T* cy, T* cw, T* ch
	) {
		*cw = T(std::min(wx + ww, x + w)) - (*cx = T(std::max(wx, x)));
		*ch = T(std::min(wy + wh, y + h)) - (*cy = T(std::max(wy, y)));
		return *cw > 0 && *ch > 0;
	}
	bool clip_rect(const Rect& r, const Rect& area, Rect* result) {
		return clip_rect(
			r.x,
			r.y,
			r.w,
			r.h,
			area.x,
			area.y,
			area.w,
			area.h,
			&result->x,
			&result->y,
			&result->w,
			&result->h
		);
	}
}

class Clipper {
public:
	using View = std::function<const Rect&(void)>;
private:
	View view;

public:
	Clipper& SetView(const View & f)
	{
		view = f; return *this;
	}

	bool Clip(const Rect& r, const Rect& dpyArea, Point* dpyPos, Rect* clippedBox) const {
		Rect visibleArea;
		if (!clip_util::clip_rect(r, view(), &visibleArea))
		{
			clippedBox->w = clippedBox->h = 0; return false;
		}
		else {
			// clippedBox is in ‘r’ coordinates, sub-rectangle of the input rectangle
			clippedBox->x = r.x - visibleArea.x;
			clippedBox->y = r.y - visibleArea.y;
			clippedBox->w = visibleArea.w;
			clippedBox->h = visibleArea.h;
			dpyPos->x = dpyArea.x + (visibleArea.x - view().x);
			dpyPos->y = dpyArea.y + (visibleArea.y - view().y);
			return true;
		}
	}

	Clipper(void) = default;
	Clipper(const Clipper&) = default;
};
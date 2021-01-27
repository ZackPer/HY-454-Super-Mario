#pragma once
#include "../Types/Types.h"
#include "../Util/AllegroUtil.h"
#include "../../ThirdParty/json.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <set>

#include "../Tiles/Tiles.h"

using json = nlohmann::json;
json parsedFilms;

std::string jsonToString(std::string filename) {
	std::string jsonString;
	std::string line;
	std::ifstream input(filename);
	if (input.is_open())
		while (std::getline(input, line)) {
			jsonString += line;
		}
	return jsonString;
}

class AnimationFilm {
	std::vector<Rect> boxes;
	ALLEGRO_BITMAP* bitmap = nullptr;
	std::string id;
public:
	byte GetTotalFrames(void) const {
		auto asd = boxes.size();
		return boxes.size();
	}
	ALLEGRO_BITMAP* GetBitmap(void) const { 
		return bitmap; 
	}
	auto GetId(void) const -> const std::string& {
		return id; 
	}
	const Rect& GetFrameBox(byte frameNo) const {
		assert(boxes.size() > frameNo); 
		return boxes[frameNo];
	}
	void DisplayFrame(ALLEGRO_BITMAP* dest, const Point& at, byte frameNo) const {
		MaskedBlit(bitmap, GetFrameBox(frameNo), dest, at);
	}
	void SetBitmap(ALLEGRO_BITMAP* b){
		assert(!bitmap); bitmap = b;
	}
	void Append(const Rect& r) { 
		boxes.push_back(r);
	}
	//AnimationFilm(const std::string& _id) : id(_id) {}
	AnimationFilm(){}
	AnimationFilm(ALLEGRO_BITMAP* b, const std::vector<Rect>& r, const std::string& id) {
		this->id = id;
		this->bitmap = b;
		this->boxes = r;
	}
};

class BitmapLoader {
private:
	using Bitmaps = std::map<std::string, ALLEGRO_BITMAP*>;
	Bitmaps bitmaps;
	ALLEGRO_BITMAP* GetBitmap(const std::string& path) const {
		auto i = bitmaps.find(path);
		return i != bitmaps.end() ? i->second : nullptr;
	}
public:
	ALLEGRO_BITMAP* Load(const std::string& path) {
		auto b = GetBitmap(path);
		if (!b) {
			bitmaps[path] = b = al_load_bitmap(path.c_str());
			assert(b);
		}
		return b;
	}
	// prefer to massively clear bitmaps at the end than
	// to destroy individual bitmaps during gameplay

	/*void CleanUp(void) {
		for (auto& i : bitmaps)
			BitmapDestroy(i.second);
		bitmaps.clear();
	}
	BitmapLoader(void) {}
	~BitmapLoader() { CleanUp(); }*/
};

class AnimationFilmHolder {
private:
	using Films = std::map<std::string, AnimationFilm*>;
	Films films;
	BitmapLoader bitmaps; // only for loading of film bitmaps
	static AnimationFilmHolder holder; // singleton
	AnimationFilmHolder(void) {}
	//~AnimationFilmHolder() { CleanUp(); }
public:
	static auto Get(void) -> AnimationFilmHolder& { return holder; }
	static auto GetConst(void) -> const AnimationFilmHolder& { return holder; }

	static int ParseEntry( //1 = terminated gracefully, 0 = parsed last entry
		int startPos,
		std::string& id,
		std::string& path,
		std::vector<Rect>& rects
	) {	
		id = parsedFilms["films"][startPos]["id"];
		std::cout << id << std::endl;
		path = id + ".png";
		for (int i = 0; i < parsedFilms["films"][startPos]["frames"].size(); i++) {
			byte x = parsedFilms["films"][startPos]["frames"][i]["x"];
			byte y = parsedFilms["films"][startPos]["frames"][i]["y"];
			byte w = parsedFilms["films"][startPos]["frames"][i]["w"];
			byte h = parsedFilms["films"][startPos]["frames"][i]["h"];
			rects.push_back(Rect(x, y, w, h));
		}
		if (startPos == parsedFilms["films"].size() - 1)
			return 0;
		else if (startPos < parsedFilms["films"].size() - 1)
			return 1;
		assert(false);
	}

	void LoadAll() {
		int pos = 0;
		int flag = 1;
		while (flag) {
			std::string id, path;
			std::vector<Rect> rects;			
			flag = ParseEntry(pos, id, path, rects);
			pos += 1;
			assert(!GetFilm(id));
			films[id] = new AnimationFilm(bitmaps.Load(path), rects, id);
		}
	}

	void CleanUp(void) {
		for (auto& i : films)
			delete(i.second);
		films.clear();
	}
	auto GetFilm(const std::string& id) -> AnimationFilm* const {
		auto i = films.find(id);
		return i != films.end() ? i->second : nullptr;
	}
};

AnimationFilmHolder AnimationFilmHolder::holder;
AnimationFilmHolder InitAnimationFilmHolder() {
	std::string text = jsonToString("films.json");
	parsedFilms = json::parse(text);
	AnimationFilmHolder filmHolder = filmHolder.Get();
	filmHolder.LoadAll();
	return filmHolder;
}

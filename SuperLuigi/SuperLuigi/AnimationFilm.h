#pragma once
#include "Types.h"
#include "AllegroUtil.h"
#include "json.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <set>

#include "Tiles.h"

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
		al_set_target_bitmap(dest);
		al_clear_to_color(al_map_rgba(0, 0, 0, 0));
		BitmapBlit(bitmap, GetFrameBox(frameNo), dest, at);
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
	using Films = std::map<std::string, AnimationFilm*>;
	Films films;
	BitmapLoader bitmaps; // only for loading of film bitmaps
	static AnimationFilmHolder holder; // singleton
	AnimationFilmHolder(void) {}
	//~AnimationFilmHolder() { CleanUp(); }
public:
	static auto Get(void) -> const AnimationFilmHolder& { return holder; }
	static auto Get2(void) -> AnimationFilmHolder& { return holder; }
	// TODO(4u): set a parsing functor implemented externally to the class
	static int ParseEntry( //1 = terminated gracefully, 0 = parsed last entry
		int startPos,
		std::string& id,
		std::string& path,
		std::vector<Rect>& rects
	) {	
		id = parsedFilms["films"][startPos]["id"];
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
	auto GetFilm(const std::string& id) -> const AnimationFilm* const {
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



class Animation {
protected:
	std::string id;
public:
	const std::string&	GetId(void) { return id; }
	void				SetId(const std::string& _id) { id = _id; };
	virtual Animation*	Clone(void) const = 0;
	Animation(const std::string& _id) : id(_id){}
	virtual ~Animation(){}
};

class MovingAnimation : public Animation {
protected:
	unsigned	reps = 1; // 0 = inf
	int			dx = 0, dy = 0;
	unsigned	delay = 0;
public:
	using Me = MovingAnimation;
	int GetDx(void) const { return dx; }
	Me& SetDx(int v) { dx = v; return *this; }
	int GetDy(void) const { return dy; }
	Me& SetDy(int v) { dy = v; return *this; }
	unsigned GetDelay(void) const { return delay; }
	Me& SetDelay(unsigned v) { delay = v; return *this; }
	unsigned GetReps(void) const { return reps; }
	Me& SetReps(unsigned n) { reps = n; return *this; }
	bool IsForever(void) const { return !reps; }
	Me& SetForever(void) { reps = 0; return *this; }
	Animation* Clone(void) const override
	{
		return new MovingAnimation(id, reps, dx, dy, delay);
	}

	MovingAnimation(
		const std::string& _id, unsigned _reps, int _dx, int _dy, unsigned _delay
	) : Animation(_id), reps(_reps), dx(_dx), dy(_dy), delay(_delay) {}
};


typedef uint64_t timestamp_t;
enum animatorstate_t {
	ANIMATOR_FINISHED = 0, ANIMATOR_RUNNING = 1, ANIMATOR_STOPPED = 2
};
class Animator {
public:
	using OnFinish = std::function<void(Animator*)>;
	using OnStart = std::function<void(Animator*)>;
	using OnAction = std::function<void(Animator*, const Animation&)>;
protected:
	timestamp_t lastTime = 0;
	animatorstate_t state = ANIMATOR_FINISHED;
	OnFinish onFinish;
	OnStart onStart;
	OnAction onAction;
	void NotifyStopped(void);
	void NotifyStarted(void);
	void NotifyAction(const Animation& anim){
		if (onAction)
			(onAction)(this, anim);
	}
	void Finish(bool isForced = false) {
		if (!HasFinished()) {
			state = isForced ? ANIMATOR_STOPPED : ANIMATOR_FINISHED;
			NotifyStopped();
		}
	}

public:
	Animator(void);
	Animator(const Animator&) = delete;
	Animator(Animator&&) = delete;
	virtual ~Animator();

	void Stop(void){ 		Finish(true); 	}
	bool HasFinished(void) const { return state != ANIMATOR_RUNNING; }
	virtual void TimeShift(timestamp_t offset) { 
		lastTime += offset; 
	}
	virtual void Progress(timestamp_t currTime) = 0;
	template <typename Tfunc> void SetOnFinish(const Tfunc& f) { onFinish = f; }
	template <typename Tfunc> void SetOnStart(const Tfunc& f) { onStart = f; }
	template <typename Tfunc> void SetOnAction(const Tfunc& f) { onAction = f; }
};extern TileLayer a;class MovingAnimator : public Animator {
protected:
	MovingAnimation* anim = nullptr;
	unsigned currRep = 0; // animation state
public:
	void Progress(timestamp_t currTime){
		int i = 0;
		while (currTime > lastTime && (currTime - lastTime) >= anim->GetDelay()) {
			lastTime += anim->GetDelay();
			NotifyAction(*anim);
			AnimationFilmHolder::Get2().GetFilm("littlemario.walk.right")->DisplayFrame(a.TileLayerBitmap, Point(0, 0), i);
			if (!anim->IsForever() && ++currRep == anim->GetReps()) {
				state = ANIMATOR_FINISHED;
				NotifyStopped();
			}
			i++;
			i = i % 4;
		}
	}
	auto GetAnim(void) const -> const MovingAnimation&
	{
		return *anim;
	}
	void Start(MovingAnimation* a, timestamp_t t) {
		anim = a;
		lastTime = t;
		state = ANIMATOR_RUNNING;
		currRep = 0;
		NotifyStarted();
	}
	MovingAnimator(void) = default;

	//void Sprite_MoveAction(Sprite* sprite, const MovingAnimation& anim) {
	//	sprite->Move(anim.GetDx(), anim.GetDy());
	//}

};class AnimatorManager {
private:
	std::set<Animator*> running, suspended;
	static AnimatorManager singleton;
	AnimatorManager(void) = default;
	AnimatorManager(const AnimatorManager&) = delete;
	AnimatorManager(AnimatorManager&&) = delete;
public:
	void Register(Animator* a)
	{
		assert(a->HasFinished()); suspended.insert(a);
	}
	void Cancel(Animator* a)
	{
		assert(a->HasFinished()); suspended.erase(a);
	}
	void MarkAsRunning(Animator* a)
	{
		assert(!a->HasFinished()); suspended.erase(a); running.insert(a);
	}
	void MarkAsSuspended(Animator* a)
	{
		assert(a->HasFinished()); running.erase(a); suspended.insert(a);
	}
	void Progress(timestamp_t currTime) {
		auto copied(running);
		for (auto* a : copied)
			a->Progress(currTime);
	}
	static auto GetSingleton(void) -> AnimatorManager& { return singleton; }
	static auto GetSingletonConst(void) -> const AnimatorManager& { return singleton; }
};AnimatorManager AnimatorManager::singleton;
void Animator::NotifyStarted(void) {
	AnimatorManager::GetSingleton().MarkAsRunning(this);
	if (onStart)
		(onStart)(this);
}

void Animator::NotifyStopped(void) {
	AnimatorManager::GetSingleton().MarkAsSuspended(this);
	if (onFinish)
		(onFinish)(this);
	std::cout << "Telos :)" << std::endl;
}

Animator::Animator(void){
	AnimatorManager::GetSingleton().Register(this);
}
Animator::~Animator(void){	AnimatorManager::GetSingleton().Cancel(this);
}
#pragma once

#include "Animations.h"
#include "AnimationFilm.h"
#include <iostream>
#include <functional>
#include "../Sprite/Sprite.h"

extern TileLayer a;

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
	Sprite* sprite;
	timestamp_t lastTime = 0;
	animatorstate_t state = ANIMATOR_FINISHED;
	OnFinish onFinish;
	OnStart onStart;
	OnAction onAction;
	void NotifyStopped(void);

	void NotifyStarted(void);

	void NotifyAction(const Animation& anim) {
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

	const Sprite& GetSprite() {
		return *sprite;
	}

	void Stop(void) {
		Finish(true);
	}
	bool HasFinished(void) const { return state != ANIMATOR_RUNNING; }
	virtual void TimeShift(timestamp_t offset) {
		lastTime += offset;
	}
	virtual void Progress(timestamp_t currTime) = 0;
	template <typename Tfunc> void SetOnFinish(const Tfunc& f) { onFinish = f; }
	template <typename Tfunc> void SetOnStart(const Tfunc& f) { onStart = f; }
	template <typename Tfunc> void SetOnAction(const Tfunc& f) { onAction = f; }
};

class MovingAnimator : public Animator {
protected:
	MovingAnimation* anim = nullptr;
	unsigned currRep = 0; // animation state
public:
	void Progress(timestamp_t currTime) {
		while (currTime > lastTime && (currTime - lastTime) >= anim->GetDelay()) {
			lastTime += anim->GetDelay();
			NotifyAction(*anim);
			if (!anim->IsForever() && ++currRep == anim->GetReps()) {
				state = ANIMATOR_FINISHED;
				NotifyStopped();
			}
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
};

class FrameRangeAnimator : public Animator {
protected:
	FrameRangeAnimation* anim = nullptr;
	unsigned currFrame = 0; // animation state
	unsigned currRep = 0; // animation state
public:
	void Progress(timestamp_t currTime) {
		while (currTime > lastTime && (currTime - lastTime) >= anim->GetDelay()) {
			if (currFrame == anim->GetEndFrame()) {
				assert(anim->IsForever() || currRep < anim->GetReps());
				currFrame = anim->GetStartFrame(); // flip to start
			}
			else {
				currFrame = (currFrame + 1) % anim->GetEndFrame();
			}
				
			lastTime += anim->GetDelay();
			NotifyAction(*anim);
			if (currFrame == anim->GetEndFrame()-1)
				if (!anim->IsForever() && ++currRep == anim->GetReps()) {
					state = ANIMATOR_FINISHED;
					NotifyStopped();
					return;
				}
		}
	}
	unsigned GetCurrFrame(void) const { return currFrame; }
	unsigned GetCurrRep(void) const { return currRep; }
	void Start(Sprite* sprite, FrameRangeAnimation* anim, timestamp_t t) {
		this->sprite = sprite;
		this->anim = anim;
		lastTime = t;
		state = ANIMATOR_RUNNING;
		currFrame = anim->GetStartFrame();
		currRep = 0;
		NotifyStarted();
		NotifyAction(*anim);
	}
	FrameRangeAnimator(void) {
		this->SetOnAction(
			[=](Animator* animator, const Animation& anim) {
				FrameRange_Action(sprite, animator, (const FrameRangeAnimation&)anim);
			}
		);
	}
	void FrameRange_Action(Sprite* sprite, Animator* animator, const FrameRangeAnimation& anim) {
		auto* frameRangeAnimator = (FrameRangeAnimator*)animator;
		if (frameRangeAnimator->GetCurrFrame() != anim.GetStartFrame() ||
			frameRangeAnimator->GetCurrRep())
			sprite->Move(anim.GetDx(), anim.GetDy());
		sprite->SetFrame(frameRangeAnimator->GetCurrFrame());
	}
};

class AnimatorManager {
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
};

void Animator::NotifyStarted(void) {
	AnimatorManager::GetSingleton().MarkAsRunning(this);
	if (onStart)
		(onStart)(this);
}

void Animator::NotifyStopped(void) {
	AnimatorManager::GetSingleton().MarkAsSuspended(this);
	if (onFinish)
		(onFinish)(this);
}

Animator::Animator(void) {
	AnimatorManager::GetSingleton().Register(this);
}
Animator::~Animator(void) {
	AnimatorManager::GetSingleton().Cancel(this);
}

AnimatorManager AnimatorManager::singleton;
#include <iostream>
#include <chrono>

class SystemClock final {
private:
	std::chrono::high_resolution_clock clock;
	static SystemClock singleton;
public:
	static auto Get(void) -> SystemClock&
	{
		return singleton;
	}
	uint64_t milli_secs(void) const {
		return std::chrono::duration_cast<std::chrono::milliseconds>
			(clock.now().time_since_epoch()).count();
	}
	uint64_t micro_secs(void) const {
		return std::chrono::duration_cast<std::chrono::microseconds>
			(clock.now().time_since_epoch()).count();
	}
	uint64_t nano_secs(void) const {
		return std::chrono::duration_cast<std::chrono::nanoseconds>
			(clock.now().time_since_epoch()).count();
	}
};SystemClock SystemClock::singleton;uint64_t GetSystemTime(void) {
	return SystemClock::Get().micro_secs();
}
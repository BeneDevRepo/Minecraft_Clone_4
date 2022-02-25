#pragma once

#include <cstdint>
#include <algorithm>
#include <Windows.h>

inline uint64_t timeStamp() {
	LARGE_INTEGER time;
	QueryPerformanceFrequency(&time);
	return time.QuadPart;
}

struct ProfileSegment {
	const char *name;
	uint64_t start, end;
};

struct ProfileResult {
	uint32_t numSampled;
	ProfileSegment segments[2048];
	bool segmentRunning;

	inline ProfileResult() { reset(); }
	inline void reset() { numSampled = 0; segmentRunning = false; }
	inline void startSegment(const char *const name) {
		if(segmentRunning) stopSegment();
		segments[numSampled].name = name;
		segments[numSampled].start = timeStamp();
		segmentRunning = true;
	}
	inline void stopSegment() {
		if(!segmentRunning) return;
		segments[numSampled++].end = timeStamp();
		segmentRunning = false;
	}
};

class Profiler {
private:
	ProfileResult buffer1, buffer2;
	ProfileResult *active, *inactive;

private:
	inline Profiler(): active(&buffer1), inactive(&buffer2) { }

public:
	static inline Profiler &get() {
		static Profiler instance;
		return instance;
	}
	inline void startFrame() {
		active->stopSegment();
		std::swap(active, inactive);
		active->reset();
		active->startSegment("unknown");
	}
	inline void startSegment(const char *const segmentName) {
		active->startSegment(segmentName);
	}
	inline void endSegment() {
		active->stopSegment();
	}
};
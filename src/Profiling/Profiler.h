#pragma once

#include <cstdint>
#include <algorithm>
#include <Windows.h>

#include <iostream>

inline uint64_t timeStamp() {
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	return time.QuadPart;
}
inline uint64_t timerFrequency() {
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	return freq.QuadPart;
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
		thread_local static Profiler instance;
		return instance;
	}
	inline void startFrame(const uint64_t frame) {
		active->stopSegment();

		{
			static int frame = 0; frame++;
			if(frame % 120 == 0) {
				const static uint64_t ticksPerSec = timerFrequency();
				const uint64_t frameTime = active->segments[active->numSampled-1].end - active->segments[0].start;
				printf("\nFrame [%lld] total: %.2fms\n", frame, frameTime * 1. / (ticksPerSec / 1000));

				for(int i = 0; i < active->numSampled; i++) {
					const ProfileSegment &seg = active->segments[i];
					const uint64_t segTime = seg.end - seg.start;
					printf("	[<%s> %.2fms (%.2f%%)]\n", seg.name, segTime * 1. / (ticksPerSec / 1000), segTime * 100. / frameTime);
				}
			}
		}

		std::swap(active, inactive);
		active->reset();
		active->startSegment("unknown");
	}
	inline void startSegment(const char *const segmentName) {
		active->startSegment(segmentName);
	}
};
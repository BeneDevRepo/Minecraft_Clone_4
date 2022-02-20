#pragma once

#include <windows.h>
#include <cstdint>

class COM {
private:
	static thread_local uint32_t numClients;

public:
	// static inline bool isInitialized() { return numClients > 0; };
	static void initialize();
	static void createInstance(const IID &clsid, const IID &iid, LPVOID *ppv);
	static void uninitialize();
};
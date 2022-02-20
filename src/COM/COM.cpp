#include "COM.h"

#include <stdexcept>

thread_local uint32_t COM::numClients = 0;

void COM::initialize() {
	if(numClients++ > 0)
		return;
	if(CoInitialize(NULL) != S_OK)
		throw std::runtime_error("COM Error: in initialize: Initializing COM failed.");
}

void COM::uninitialize() {
	if(numClients == 0)
		throw std::runtime_error("COM Error: in uninitialize: COM not initialized.");
	if(--numClients > 0)
		return;
	CoUninitialize();
}

void COM::createInstance(const IID &clsid, const IID &iid, LPVOID *ppv) {
	if(numClients == 0)
		throw std::runtime_error("COM Error: in createInstance: COM not initialized.");

	HRESULT hr = CoCreateInstance(
			clsid,
			NULL,
			CLSCTX_ALL,
			iid,
			ppv);

	if(hr != S_OK)
		throw std::runtime_error("COM Error: in createInstance: CoCreateInstance Failed.");
}
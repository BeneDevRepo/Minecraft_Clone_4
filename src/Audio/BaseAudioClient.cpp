#include <initguid.h>

#include "BaseAudioClient.h"

#include "COM/COM.h"
#include <stdexcept>
// #include <iostream>

// REFERENCE_TIME unit: 100ns = 0.1us =  0.0001ms =   0.0000001s
// REFERENCE_TIMEs per time:    10/us ;  10000/ms ;   10000000/s
// REFERENCE_TIMEs per time:    10/us ; 10.000/ms ; 10.000.000/s
static constexpr uint64_t REFTIMES_PER_SEC 		= 10000000; // 10.000.000
static constexpr uint64_t REFTIMES_PER_MILLISEC = 	 10000; //     10.000
static constexpr uint64_t REFTIMES_PER_MICROSEC =		10; //         10

BaseAudioClient::BaseAudioClient(EDataFlow dataFlow, DWORD streamFlags) {
	COM::initialize();
	{
		IMMDevice* pDevice;
		{
			IMMDeviceEnumerator* pEnumerator;
			COM::createInstance(CLSID_MMDeviceEnumerator, IID_IMMDeviceEnumerator, (void**)&pEnumerator);
			HRESULT hr3 = pEnumerator->GetDefaultAudioEndpoint(dataFlow, ERole::eConsole, &pDevice);
			pEnumerator->Release();
		}
		HRESULT hr3 = pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pAudioClient);
		pDevice->Release();
	}

	WAVEFORMATEX* pwfx = NULL;
	HRESULT hr4 = pAudioClient->GetMixFormat(&pwfx);
	sampleRate = pwfx->nSamplesPerSec;
	// std::cout << "Channels: " << pwfx->nChannels << "\n";
	// std::cout << "Bits per Sample: " << pwfx->wBitsPerSample << "\n";
	// std::cout << "Samples Per Second: " << pwfx->nSamplesPerSec << "\n";

	// Initialize Audio Client
	{
		REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC * 3;
		HRESULT hr5 = pAudioClient->Initialize(
				AUDCLNT_SHAREMODE::AUDCLNT_SHAREMODE_SHARED,
				0 | AUDCLNT_STREAMFLAGS_RATEADJUST | streamFlags /**/, // for IAudioClockAdjustment
				hnsRequestedDuration,
				0,
				pwfx,
				NULL);
	}

	HRESULT hr6 = pAudioClient->GetBufferSize(&bufferFrameCount); // retreive Buffersize
}

BaseAudioClient::~BaseAudioClient() {
	pAudioClient->Release();
	COM::uninitialize();
}

void BaseAudioClient::start() const {
	HRESULT hr = pAudioClient->Start();
}

void BaseAudioClient::stop() const {
	HRESULT hr = pAudioClient->Stop();
}

void BaseAudioClient::setSampleRate(float sampleRate) {
	IAudioClockAdjustment* pAudioClockAdjustment = (IAudioClockAdjustment*)getService(IID_IAudioClockAdjustment);
	pAudioClockAdjustment->SetSampleRate(sampleRate);
	pAudioClockAdjustment->Release();
	this->sampleRate = sampleRate;
}

void* BaseAudioClient::getService(const IID &riid) {
	void* service = nullptr;
	HRESULT hr = pAudioClient->GetService(riid, &service);
	if(hr != S_OK)
		throw std::runtime_error("BaseAudioClient::getService Failed.");
	return service;
}

IAudioClient* BaseAudioClient::getRawClient() { // ------------------------------------
	return pAudioClient;
}
#pragma once

#include <audioclient.h>
#include <mmdeviceapi.h>

class BaseAudioClient {
private:
	IAudioClient* pAudioClient;
	UINT32 bufferFrameCount;
	float sampleRate;

public:
	BaseAudioClient(EDataFlow dataFlow, DWORD streamFlags=0);
	~BaseAudioClient();
	void start() const;
	void stop() const;
	void setSampleRate(float sampleRate);
	inline float getSampleRate() const { return sampleRate; }
	inline UINT32 getBufferFrameCount() const { return bufferFrameCount; }
	void* getService(const IID &riid);
	IAudioClient* getRawClient();
};
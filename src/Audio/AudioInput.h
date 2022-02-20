#pragma once

#include "BaseAudioClient.h"

class AudioInput : public BaseAudioClient {
private:
	IAudioCaptureClient* pCaptureClient;

public:
	AudioInput();
	~AudioInput();
	UINT32 nextPacketSize();
	void setEventHandle(HANDLE eventHandle);
	BYTE* getBuffer(UINT32* pNumFramesAvailable);
	void releaseBuffer(UINT32 numFrames);
};
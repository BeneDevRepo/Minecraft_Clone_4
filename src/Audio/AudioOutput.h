#pragma once

#include "BaseAudioClient.h"

class AudioOutput : public BaseAudioClient {
private:
	IAudioRenderClient* pRenderClient;

public:
	AudioOutput();
	~AudioOutput();
	UINT32 numFramesPadding();
	BYTE* getBuffer(UINT32 numFrames);
	void releaseBuffer(UINT32 numFrames);

	void setMasterVolume(float volume);
	void setChannelVolumes(UINT32 numChannels, const float *volumes);

// INLINE
	inline UINT32 numFramesAvailable() { return getBufferFrameCount() - numFramesPadding(); }
};
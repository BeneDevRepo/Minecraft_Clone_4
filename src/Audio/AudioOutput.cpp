#include "AudioOutput.h"

// #include <iostream>

AudioOutput::AudioOutput(): BaseAudioClient(EDataFlow::eRender) {
	pRenderClient = (IAudioRenderClient*)getService(IID_IAudioRenderClient); // Get Renderclient
}

AudioOutput::~AudioOutput() {
	pRenderClient->Release();
}

void AudioOutput::setMasterVolume(float volume) {
	ISimpleAudioVolume* pSimpleAudioVolume = (ISimpleAudioVolume*)getService(IID_ISimpleAudioVolume);
	pSimpleAudioVolume->SetMasterVolume(volume, 0);
	pSimpleAudioVolume->Release();
}

void AudioOutput::setChannelVolumes(UINT32 numChannels, const float *volumes) {
	IAudioStreamVolume* pStreamVolume = (IAudioStreamVolume*)getService(IID_IAudioStreamVolume);
	pStreamVolume->SetAllVolumes(numChannels, volumes);
	pStreamVolume->Release();
}

UINT32 AudioOutput::numFramesPadding() {
	UINT32 numFramesPadding; // Specifies the number of audio frames that are queued up to play in the endpoint buffer.
	HRESULT hr = getRawClient()->GetCurrentPadding(&numFramesPadding);
	return numFramesPadding;
}

BYTE* AudioOutput::getBuffer(UINT32 numFrames) {
	BYTE* pData;
	HRESULT hr = pRenderClient->GetBuffer(numFrames, &pData);
	return pData;
}

void AudioOutput::releaseBuffer(UINT32 numFrames) {
	HRESULT hr = pRenderClient->ReleaseBuffer(numFrames, 0);
}
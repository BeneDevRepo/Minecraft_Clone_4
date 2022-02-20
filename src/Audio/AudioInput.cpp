#include "AudioInput.h"

// #include <iostream>

AudioInput::AudioInput(): BaseAudioClient(EDataFlow::eCapture, AUDCLNT_STREAMFLAGS_EVENTCALLBACK) {
	pCaptureClient = (IAudioCaptureClient*)getService(IID_IAudioCaptureClient);
}

AudioInput::~AudioInput() {
	pCaptureClient->Release();
}

UINT32 AudioInput::nextPacketSize() {
	UINT32 numFramesNextPacket = 0;
	HRESULT hr = pCaptureClient->GetNextPacketSize(&numFramesNextPacket);
	return numFramesNextPacket;
}

void AudioInput::setEventHandle(HANDLE eventHandle) {
	HRESULT hr = getRawClient()->SetEventHandle(eventHandle);
}

BYTE* AudioInput::getBuffer(UINT32* pNumFramesAvailable) {
	DWORD flags; // filled by GetBuffer
	BYTE *pData;
	HRESULT hr = pCaptureClient->GetBuffer(&pData, pNumFramesAvailable, &flags, NULL, NULL);// Get the available data
	return pData;
}

void AudioInput::releaseBuffer(UINT32 numFrames) {
	HRESULT hr = pCaptureClient->ReleaseBuffer(numFrames);
}
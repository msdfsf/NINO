#pragma once

#include "IAudioIO.h"
#include "SampleFormat.h"

#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm")

class AudioIOLegacy : public IAudioIO {

public:

	WAVEFORMATEX* waveFormat;
	SampleFormat::SampleFormat sampleFormat;

	HWAVEIN* waveIn;
	HWAVEOUT* waveOut;

	WAVEHDR** waveHeaders;
	BYTE** waveBuffers;

	double* buffer;

	int frameSize;
	int framesInBuffer;

	// naming...
	int bufferLen;
	int bufferCount;
	int bufferIdx;

	int leftChannelIn;
	int rightChannelIn;
	int leftChannelOut;
	int rightChannelOut;

	int reset;

	AudioIOLegacy();

	virtual int init(AudioDriver::DriverInfo* info);

	virtual int start();
	virtual int stop();
	virtual int exit();

	virtual void setChannels(AudioDriver::DriverInfo* info);
	virtual void openExternalConfig();

	virtual AudioDriver::Device** getDevices(int* deviceCount);

	virtual ~AudioIOLegacy();

};

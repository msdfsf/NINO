#pragma once

#include "IAudioIO.h"
#include "SampleFormat.h"

//#include <Audioclient.h>
//#include <Audiopolicy.h>
//#include <mmdeviceapi.h>
#include <windows.h>
//#include <mmeapi.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm")
//#pragma comment(lib,"libwinmm")

class AudioIOLegacy : public IAudioIO {

public:

	WAVEFORMATEX* waveFormat;
	SampleFormat::SampleFormat sampleFormat;

	HWAVEIN* waveIn;
	HWAVEOUT* waveOut;

	WAVEHDR** waveHeaders;
	BYTE** waveBuffers;

	double* buffer;

	int bufferLen;
	int bufferCount;
	int bufferIdx;

	int leftChannelIn;
	int rightChannelIn;
	int leftChannelOut;
	int rightChannelOut;

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
#pragma once

#include "IAudioIO.h"

class AudioIOFile : IAudioIO {

public:

	AudioIOFile();

	template <typename InData, typename OutData>
	void iProcessWAVE(
		InData inData,
		OutData outData,
		double* const inBuff,
		double* const outBuff,
		const int sgn = 1
	);

	template <typename InData, typename OutData>
	void fProcessWAVE(
		InData inData,
		OutData outData,
		double* const inBuff,
		double* const outBuff
	);

	virtual int init(AudioDriver::DriverInfo* info);

	virtual int start();
	virtual int stop();
	virtual int exit();

	virtual void setChannels(AudioDriver::DriverInfo* info);
	virtual void openExternalConfig();

	virtual AudioDriver::Device** getDevices(int* deviceCount);

	virtual ~AudioIOFile();

	AudioDriver::DriverInfo* driverInfo;

	int leftChannelIn = 0;
	int leftChannelOut = 0;
	int rightChannelIn = 0;
	int rightChannelOut = 0;

	int audioFormat;
	int fileType;
	int sampleRate;
	int buffSize;

	// WAVE only
	int bitsPerSample;
	int bytesPerFrame;
	int channelCount;

	int inDataLen;
	void* inData;

};

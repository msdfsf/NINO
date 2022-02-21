#pragma once

#include "AudioDriver.h"

class IAudioIO {

public:

	IAudioIO() {};

	virtual int init(AudioDriver::DriverInfo* info) = 0;

	virtual int start() = 0;
	virtual int stop() = 0;
	virtual int exit() = 0;

	virtual void setChannels(AudioDriver::DriverInfo* info) = 0;
	virtual void openExternalConfig() = 0;

	virtual AudioDriver::Device** getDevices(int* deviceCount) = 0;

	void (*processInput) (void* inBuffer, void* outBuffer, int bufferLength);

	virtual ~IAudioIO() {};

private:

};
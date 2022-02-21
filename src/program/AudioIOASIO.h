#pragma once

#include "IAudioIO.h"

class AudioIOASIO : IAudioIO {

public:

	AudioIOASIO();

	virtual int init(AudioDriver::DriverInfo* info);

	virtual int start();
	virtual int stop();
	virtual int exit();

	virtual void setChannels(AudioDriver::DriverInfo* info);
	virtual void openExternalConfig();

	virtual AudioDriver::Device** getDevices(int* deviceCount);

	virtual ~AudioIOASIO();

private:

};
#pragma once

#include <string>

#include "AudioDriver.h"

namespace Config {

	const std::string fileName = "./config.cfg";

	extern int windowWidth;
	extern int windowHeight;

	extern int renderWidth;
	extern int renderHeight;

	extern int windowResize;
	extern int windowMaximize;
	extern int fullScreen;

	extern AudioDriver::Driver audioDriver;

	extern int audioDeviceId;
	extern int leftChannelIn;
	extern int rightChannelIn;
	extern int leftChannelOut;
	extern int rightChannelOut;
	extern int sampleRate;

	extern char* outFileName;
	extern char* inFileName;

	int load();

	void setWindowWidth(int width);
	void setWindowHeight(int height);

	void setRenderWidth(int width);
	void setRenderHeight(int height);

	void setWindowResize(int resize);
	void setWindowMaximize(int maximize);
	void setWindowFullScreen(int fullScreen);

	void setAudioDriver(AudioDriver::Driver driver);
	void setAudioDevice(int deviceId);
	void setLeftChannelIn(int on);
	void setRightChannelIn(int on);
	void setLeftChannelOut(int on);
	void setRightChannelOut(int on);
	void setSampleRate(int smplrt);

	void setOutFileName(std::string name);
	void setInFileName(std::string name);

};

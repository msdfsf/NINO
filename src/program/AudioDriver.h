#pragma once

#include "IPlugin.h"

namespace AudioDriver {

	// alphabetical order for now
	enum Driver {

		AD_ASIO,
		AD_FILE,
		AD_LEGACY,
		AD_WASAPI,
		AD_COUNT

	};

	// for now only 2 channels are supportes as for input as output
	enum Channel {
		
		CHANNEL_1 = 1,
		CHANNEL_2 = 2,
		CHANNEL_3 = 4,
		CHANNEL_4 = 8,
		CHANNEL_5 = 16,
		CHANNEL_6 = 32,
		// .. etc .. //
	
	};

	struct Device {
		
		char* name; // null terminated
		int id;
	
	};

	struct DriverInfo {
		
		Device* device;

		// each bit represent cast of each channel, most left bit represents first channel, 
		// use enum Channel to fill these properties
		int channelIn;
		int channelOut;

		int sampleRate;
		int maxBufferLength;

	};

	// handler that will contain all info about driver initialization
	extern DriverInfo info;

	const int INIT_MAX_PLUGINS = 10;
	extern int maxPluginCount;

	IPlugin** getSamplePlugins();
	IPlugin** getPlugins();
	IPlugin** getPlugins(int* count);
	int getPluginCount();

	int addPlugin(IPlugin* plugin); // return index in inner array, used to remove it
	void removePlugin(int idx);
	void removeAll();

	void openExternalConfig();

	void select(Driver driver);

	void init(DriverInfo* info);

	void setChannels(AudioDriver::DriverInfo* info);

	Device** getDevices(int* deviceCount);

	void* getInstance();

	void start();
	void stop();
	void exit();

}

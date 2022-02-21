#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include "PluginState.h"
#include "AudioDriver.h"
#include "Plugin.h"

#include <thread>
#include <Windows.h>

#include "AudioIOASIO.h"
#include "AudioIOWASAPI.h"
#include "AudioIOLegacy.h"
#include "AudioIOFile.h"

#include "Utils.h"

namespace AudioDriver {

	void processSignals(void* inBuffer, void* outBuffer, int bufferLength);
	void processSignalsFile(void* inBuffer, void* outBuffer, int bufferLength);

	DriverInfo info;

	int running = 0;

	int maxPluginCount = 0;
	int pluginCount = 0;
	IPlugin** plugins;
	IPlugin** samplePlugins;

	IAudioIO* audioIO = NULL;

	void select(Driver driver) {
	
		switch (driver) {

			case AD_ASIO: {

				if (audioIO != NULL) delete audioIO;

				AudioIOASIO* asio = new AudioIOASIO();
				audioIO = (IAudioIO*) asio;
				audioIO->processInput = &processSignals;

				break;

			};

			case AD_WASAPI: {

				if (audioIO != NULL) delete audioIO;

				AudioIOWASAPI* wasapi = new AudioIOWASAPI();
				audioIO = (IAudioIO*) wasapi;
				audioIO->processInput = &processSignals;

				break;

			};

			case AD_LEGACY: {

				if (audioIO != NULL) delete audioIO;

				AudioIOLegacy* legacy = new AudioIOLegacy();
				audioIO = (IAudioIO*) legacy;
				audioIO->processInput = &processSignals;

				break;

			};

			case AD_FILE: {
				
				if (audioIO != NULL) delete audioIO;

				AudioIOFile* file = new AudioIOFile();
				audioIO = (IAudioIO*) file;
				audioIO->processInput = &processSignals;

				break;
			
			};

		}

	}

	void init(AudioDriver::DriverInfo* info) {

		audioIO->init(info);

	}

	void setChannels(AudioDriver::DriverInfo* info) {
		
		audioIO->setChannels(info);

	}

	void openExternalConfig() {

		audioIO->openExternalConfig();

	}

	Device** getDevices(int* deviceCount) {

		return audioIO->getDevices(deviceCount);

	}

	void* getInstance() {
		return audioIO;
	}

	void start() {

		audioIO->start();
		running = 1;

	}

	void stop() {

		audioIO->stop();
		running = 0;
	
	}

	void exit() {

		audioIO->exit();
	
	}

	IPlugin** getSamplePlugins() {

		return samplePlugins;

	}

	IPlugin** getPlugins() {

		return plugins;

	}

	IPlugin** getPlugins(int* count) {
		
		*count = pluginCount;
		return plugins;
	
	
	}

	int getPluginCount() {
		
		return pluginCount;
	
	}

	int addPlugin(IPlugin* newPlugin) {

		if (pluginCount >= maxPluginCount) {
		
			IPlugin** const tmp = (IPlugin**) realloc(plugins, sizeof(IPlugin*) * (pluginCount + 1));
			if (tmp == NULL) {
				return -1;
			}

			IPlugin** const tmp2 = (IPlugin**) realloc(samplePlugins, sizeof(IPlugin*) * (pluginCount + 1));
			if (tmp2 == NULL) {
				return -1;
			}
			samplePlugins = tmp2;
			samplePlugins[pluginCount] = newPlugin;

			IPlugin* pluginSpace = (IPlugin*) malloc(sizeof(IPlugin));
			tmp[maxPluginCount] = pluginSpace;

			if (Plugin::copyPlugin(tmp[maxPluginCount], newPlugin)) {
				free(tmp[maxPluginCount]);
				// realoc(tmp)
				return -1;
			}

			plugins = tmp;
			maxPluginCount++;
		
		} else {

			if (Plugin::copyPlugin(plugins[pluginCount], newPlugin)) {
				free(plugins[pluginCount]);
				// realoc(tmp)
				return -1;
			}

			samplePlugins[pluginCount] = newPlugin;
		
		}

		IPluginInfo pluginInfo = { 
			info.sampleRate
			
		};
		plugins[pluginCount]->space = NULL;
		if (plugins[pluginCount]->init(&pluginInfo, &(plugins[pluginCount]->space))) {
			// seems like malloc error
			free(plugins[pluginCount]);
			return -1;
		}

		pluginCount++;
		return pluginCount - 1;
	
	}
	
	/*
	void freePlugin(IPlugin* plugin) {
		while (plugin->state != PluginState::DELETED) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		plugin->free(plugin->space);
	}
	*/

	void removePlugin(int idx) {
		
		if (pluginCount <= 0 || idx < 0 || idx >= maxPluginCount) return;

		// has to be better solution, but lets keep it simple as that for now, as it will work 100%
		// at least I hope so...
		if (plugins[idx]->free && running && plugins[idx]->state == PluginState::ON) {
			// plugins[idx]->state = PluginState::TO_BE_DELETED;
			//std::thread th(freePlugin, plugins[idx]);
			//th.detach();
			plugins[idx]->state = PluginState::TO_BE_DELETED;
			while (plugins[idx]->state != PluginState::DELETED) {
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
			plugins[idx]->free(plugins[idx]->space);
		}

		pluginCount--;

		IPlugin* tmpPlugin = plugins[idx];
		IPlugin* tmpSamplePlugin = samplePlugins[idx];
		for (int i = idx; i < maxPluginCount - 1; i++) {
			plugins[i] = plugins[i + 1];
			samplePlugins[i] = samplePlugins[i + 1];
		}
		plugins[maxPluginCount - 1] = tmpPlugin;
		samplePlugins[maxPluginCount - 1] = tmpSamplePlugin;

	}

	void removeAll() {

		/*
		for (int i = 0; i < pluginCount - 1; i++) {
			plugins[i] = NULL;
		}
		*/
		for (int i = 0; i < pluginCount; i++) {
			if (plugins[i]->free) plugins[i]->free(plugins[i]->space);
		}
		pluginCount = 0;
	
	}

	void processSignals(void* inBuffer, void* outBuffer, int bufferLength) {
		
		const int buffCount = pluginCount;
		const int buffLen = bufferLength;

		// firstly we set out buffer by ourself, so we can apply some basic dsp
		double* const foutBuffer = (double*) outBuffer;
		double* const fintBuffer = (double*) inBuffer;
		for (int i = 0; i < buffLen; i++) {
			foutBuffer[i] = fintBuffer[i];
		}

		for (int i = 0; i < buffCount; i++) {

			IPlugin* const plugin = plugins[i];
			
			const int state = plugin->state;
			if (state == PluginState::ON)
				plugin->process(outBuffer, outBuffer, bufferLength, plugin->space);
			if (state == PluginState::TO_BE_DELETED)
				plugin->state = PluginState::DELETED;
		
		}
	
	}

}
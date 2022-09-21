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

#include "ErrorCodes.h"

#include "Utils.h"

namespace AudioDriver {

	int processSignals(void* inBuffer, void* outBuffer, const int bufferLength);
	int processSignalsLegacy(void* inBuffer, void* outBuffer, const int buffLen);

	DriverInfo info;

	int running = 0;

	int maxPluginCount = 0;
	int pluginCount = 0;
	IPlugin** plugins;
	IPlugin** samplePlugins;

	IAudioIO* audioIO = NULL;

	double pan = 0.5; // 0 - 1, 1 full right, 0 full left

	void select(Driver driver) {
	
		if (audioIO != NULL) delete audioIO;

		switch (driver) {

			case AD_ASIO: {

				AudioIOASIO* asio = new AudioIOASIO();
				audioIO = (IAudioIO*) asio;
				audioIO->processInput = &processSignals;

				break;

			};

			case AD_WASAPI: {

				AudioIOWASAPI* wasapi = new AudioIOWASAPI();
				audioIO = (IAudioIO*) wasapi;
				audioIO->processInput = &processSignals;

				break;

			};

			case AD_LEGACY: {

				AudioIOLegacy* legacy = new AudioIOLegacy();
				audioIO = (IAudioIO*) legacy;
				audioIO->processInput = &processSignalsLegacy;

				break;

			};

			case AD_FILE: {

				AudioIOFile* file = new AudioIOFile();
				audioIO = (IAudioIO*) file;
				audioIO->processInput = &processSignalsLegacy;

				break;
			
			};

		}

	}

	void init(AudioDriver::DriverInfo* info) {

		// maybe default function in ErrorCodes.h that will switch case all errors with default messages
		switch (audioIO->init(info)) {
			
			case ERR_ALLOC :
				Utils::showError("Allocation failed!");
				break;

			case ERR_WHATSOEVER :
				Utils::showError("Something bad happened! The error is unknown, or programmer was lazy to identify it!");
				break;

			case ERR_AD_INVALID_INPUT_DEVICE :
				Utils::showError("Could not initialize input device! Make sure os can do so");
				break;

			case ERR_AD_INVALID_OUTPUT_DEVICE :
				Utils::showError("Could not initialize output device! Make sure os can do so!");
				break;

			case ERR_AD_INVALID_FORMAT :
				Utils::showError("Something wrong with the audio format! Try to play around with the os settings!");
				break;

			default :
				break;
				// Utils::showError("default state......");
		
		}

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

		// has to be better solution, but lets keep it simple as that for now
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

	// for now outBuffer has to have two consequence buffers
	// in put has to be the same packed, but only first buffer considers as inout data (so only mono input)
	// returns 1 if two out buffers has to be mixed
	// returns 0 if there is only first buffer filled and has to be passed to both channels
	int processSignals(void* inBuffer, void* outBuffer, const int buffLen) {

		const int buffCount = pluginCount;

		double* const finBufferL = (double*) inBuffer;
		double* const finBufferR = ((double*) inBuffer) + buffLen;
		double* const foutBufferL = (double*) outBuffer;
		double* const foutBufferR = ((double*) outBuffer) + buffLen;

		for (int i = 0; i < buffLen; i++) {
			foutBufferL[i] = foutBufferR[i] = 0;
		}

		int mix = 0;
		for (int i = 0; i < buffCount; i++) {

			IPlugin* const plugin = plugins[i];
			
			const int state = plugin->state;

			if (state == PluginState::OFF) continue;

			if (!mix && state != PluginState::ON) {
				for (int i = 0; i < buffLen; i++) {
					finBufferR[i] = finBufferL[i];
				}
			}

			switch (state) {

				case PluginState::ON : {
					
					if (mix) {

						const double lRatio = 1 - pan;
						const double rRatio = pan;
						
						for (int i = 0; i < buffLen; i++) {
							finBufferL[i] = lRatio * foutBufferL[i] + rRatio * foutBufferR[i];
						}
					
					}
					mix = 0;

					plugin->process((void*) finBufferL, (void*) foutBufferL, buffLen, plugin->space);

					for (int i = 0; i < buffLen; i++) {
						finBufferL[i] = foutBufferL[i];
					}

					break;

				}

				case PluginState::ON_LEFT: {

					mix = 1;

					plugin->process((void*) finBufferL, foutBufferL, buffLen, plugin->space);

					for (int i = 0; i < buffLen; i++) {
						finBufferL[i] = foutBufferL[i];
					}

					break;

				}

				case PluginState::ON_RIGHT: {
					
					mix = 1;

					plugin->process((void*) finBufferR, (void*) foutBufferR, buffLen, plugin->space);
					
					for (int i = 0; i < buffLen; i++) {
						finBufferR[i] = foutBufferR[i];
					}

					break;
				}
			
			}

			if (state == PluginState::TO_BE_DELETED)
				plugin->state = PluginState::DELETED;

		}
	
		return mix;

	}

	int processSignalsLegacy(void* inBuffer, void* outBuffer, const int buffLen) {

		const int buffCount = pluginCount;

		double* const foutBuffer = (double*) outBuffer;
		double* const fintBuffer = (double*) inBuffer;

		int noActivePlugin = 1; // there has to be better name
		for (int i = 0; i < buffCount; i++) {

			IPlugin* const plugin = plugins[i];
			const int state = plugin->state;
			
			if (state > PluginState::OFF && state <= PluginState::ON_RIGHT) {
				noActivePlugin = 0;
				plugin->process(inBuffer, outBuffer, buffLen, plugin->space);
			}

			// maybe unloop or whatever
			if (i < buffCount - 1)
				memcpy(inBuffer, outBuffer, buffLen * sizeof(double));

			if (state == PluginState::TO_BE_DELETED)
				plugin->state = PluginState::DELETED;

		}

		if (noActivePlugin) {
			memcpy(outBuffer, inBuffer, buffLen * sizeof(double));
		}

		return 0;

	}

}

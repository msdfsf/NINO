#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "Plugin.h"
#include <stdlib.h>
#include <math.h>

typedef struct Space {

	int lenSamples;
	int sampleRate;
	int endIdx;
	double* buffer;
	void* renderBuffer;

	int audioSampleRate;

	int idx;
	int startIdx;
	
	int passedSamples;

} Space;

void pluginFree(void* space) {

	free(((Space*)space)->renderBuffer);
	free(((Space*)space)->buffer);

}

int init(IPluginInfo* info, void** space) {

	if (*space != NULL) {
		
		pluginFree(*space);
	
	} else {

		(*space) = malloc(sizeof(Space));
		if (!(*space)) return 1;

	}

	const int sampleRate = 60 * 3;
	const int seconds = 4;
	((Space*) (*space))->buffer = calloc(sampleRate * seconds, sizeof(double));
	if (!((Space*) (*space))->buffer) {
		return 1;
	}

	((Space*) (*space))->audioSampleRate = info->sampleRate;
	((Space*) (*space))->sampleRate = sampleRate;
	
	((Space*) (*space))->idx = 0;
	((Space*) (*space))->startIdx = 0;
	((Space*) (*space))->endIdx = sampleRate * seconds - 1;

	((Space*) (*space))->lenSamples = sampleRate * seconds;

	((Space*) (*space))->passedSamples = 0;

	return 0;

}

void process(void* inBuffer, void* outBuffer, int bufferLength, void* space) {

	double* const inBuff = inBuffer;
	double* const outBuff = outBuffer;

	Space* const spc = ((Space*) space);

	const int sampleRate = spc->sampleRate;
	const int audioSampleRate = spc->audioSampleRate;
	const int startIdx = spc->startIdx;
	const int idx = spc->idx;
	const int lenSamples = spc->lenSamples;
	const int passedSamples = spc->passedSamples;

	const int step = audioSampleRate / sampleRate;

	// assuming step > bufferLength

	if (passedSamples + bufferLength < step) {
		// nothing realy to do

		double sum = 0;
		for (int i = 0; i < bufferLength; i++) {
			sum += fabs(inBuff[i]);
		}

		spc->buffer[idx] += sum;
		spc->passedSamples = passedSamples + bufferLength;

	} else {
		// have two parts, one is for current idx, 'overflow' for idx + 1

		double sum = 0;
		int i = 0;
		for (; i < step - passedSamples; i++) {
			sum += fabs(inBuff[i]);
		}

		spc->buffer[idx] = (spc->buffer[idx] + sum) / step;

		sum = 0;
		for (; i < bufferLength; i++) {
			sum += fabs(inBuff[i]);
		}
	
		if (idx < lenSamples - 1) {
			// ok

			spc->buffer[idx + 1] = sum;//((Space*)space)->buffer[idx + 1] + sum;
			spc->idx++;
			spc->endIdx = idx;

		} else {
			// not so ok, but ok

			spc->buffer[0] = sum;//+= sum;
			spc->idx = 0;
			spc->endIdx = idx;

		}

		spc->passedSamples = passedSamples + bufferLength - step;

	}

}

IPlugin* getPlugin() {

	IPlugin* plugin = (IPlugin*)malloc(sizeof(IPlugin));
	if (plugin == NULL) return NULL;

	PluginUIHandler* uihnd = buildPluginUIHandler();
	uihnd->controls[0]->backgroundColor = 0xFF000000;
	uihnd->controls[0]->color = 0xFFFFFFFF;
	uihnd->controls[0]->fillType = PFP_SOLID_COLOR;

	PluginControl* viewer = addControl(uihnd, PCT_SIGNAL_VIEWER);
	viewer->color = 0xFFFF0000;
	viewer->backgroundColor = 0xFF000000;
	viewer->label = "Viewer";

	plugin->name = L"Signal Viewer";
	plugin->uihnd = uihnd;
	plugin->process = &process;
	plugin->init = &init;
	plugin->free = &pluginFree;

	return plugin;

}
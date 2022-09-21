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

	free(((Space*) space)->renderBuffer);
	free(((Space*) space)->buffer);

}

int init(IPluginInfo* info, void** space) {

	if (*space != NULL) {

		pluginFree(*space);

	}
	else {

		(*space) = malloc(sizeof(Space));
		if (!(*space)) return 1;

	}

	// magic literal is length of fft that will main program calculate
	const int buffLen = 8192 * 2;
	((Space*) (*space))->buffer = calloc(buffLen, sizeof(double));
	if (!((Space*) (*space))->buffer) {
		return 1;
	}

	((Space*) (*space))->audioSampleRate = info->sampleRate;
	((Space*) (*space))->sampleRate = info->sampleRate;

	((Space*) (*space))->idx = 0;
	((Space*) (*space))->startIdx = 0;
	((Space*) (*space))->endIdx = buffLen - 1;

	((Space*) (*space))->lenSamples = buffLen;

	((Space*) (*space))->passedSamples = 0;

	return 0;

}

void process(void* inBuffer, void* outBuffer, int bufferLength, void* space) {

	double* const inBuff = inBuffer;
	double* const outBuff = outBuffer;

	Space* const spc = ((Space*) space);

	const int startIdx = spc->startIdx;
	const int idx = spc->idx;
	const int lenSamples = spc->lenSamples;
	const int passedSamples = spc->passedSamples;

	for (int i = 0; i < bufferLength; i++) {
		spc->buffer[(idx + i >= lenSamples) ? idx + i - lenSamples : idx + i] = outBuff[i] = inBuff[i];
	}

	spc->idx = (idx + bufferLength >= lenSamples) ? idx + bufferLength - lenSamples : idx + bufferLength;
	spc->endIdx = (spc->idx - 1 < 0) ? lenSamples - 1 : spc->idx - 1;

}

IPlugin* getPlugin() {

	IPlugin* plugin = (IPlugin*)malloc(sizeof(IPlugin));
	if (plugin == NULL) return NULL;

	PluginUIHandler* uihnd = buildPluginUIHandler();
	uihnd->controls[0]->backgroundColor = 0xFF000000;
	uihnd->controls[0]->color = 0xFFFFFFFF;
	uihnd->controls[0]->fillType = PFP_SOLID_COLOR;

	PluginControl* viewer = addControl(uihnd, PCT_FREQUENCY_VIEWER);
	viewer->color = 0xFFFF0000;
	viewer->backgroundColor = 0xFF000000;
	viewer->label = "Frequency";

	plugin->name = L"Frequency Viewer";
	plugin->uihnd = uihnd;
	plugin->process = &process;
	plugin->init = &init;
	plugin->free = &pluginFree;

	return plugin;

}
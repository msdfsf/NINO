#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "Plugin.h"
#include <math.h>
#include <stdlib.h>

#define M_PI 3.14159265358979323846

#define TEXT_COLOR 0xFF000000

#define MIN_RATE 0
#define MAX_RATE 4
#define DF_RATE 1

#define MIN_DELAY 0.001
#define MAX_DELAY 0.01
#define DF_DELAY 0.005

#define NEW_PHASE(phase) (((phase) > 2 * M_PI) ? (phase) - 2 * M_PI : (phase) + 2 * M_PI * rate / sampleRate);

typedef struct Space {

	int sampleRate;

	double phase;
	double rate;
	double delay;

	int startIdx;
	int length;
	double* pastBuffer;

} Space;

int freePlugin(void* space) {

	free(((Space*)space)->pastBuffer);

}


int init(IPluginInfo* info, void** space) {

	if (*space) {
		freePlugin(*space);
	} else {
		*space = malloc(sizeof(Space));
		if (!*space) return 1;
	}

	Space* const spc = (Space*) *space;

	spc->phase = 0;
	spc->rate = DF_RATE;
	spc->delay = DF_DELAY;

	spc->sampleRate = info->sampleRate;

	spc->startIdx = 0;
	spc->length = MAX_DELAY * spc->sampleRate + info->maxBufferLength;
	spc->pastBuffer = (double*) calloc(spc->length, sizeof(double));
	if (!spc->pastBuffer) return 1;

	return 0;

}

void process(void* inBuffer, void* outBuffer, int bufferLength, void* space) {

	double* const inBuff = inBuffer;
	double* const outBuff = outBuffer;

	Space* const spc = (Space*)(space);

	const double sampleRate = spc->sampleRate;

	double phase = spc->phase;
	const double rate = spc->rate;
	const double delay = spc->delay;

	const int pastBuffStartIdx = spc->startIdx;
	const int pastBuffLen = spc->length;
	double* const pastBuff = spc->pastBuffer;

	if (pastBuffStartIdx + bufferLength > pastBuffLen) {

		for (int i = pastBuffStartIdx; i < pastBuffLen; i++) {
			pastBuff[i] = inBuff[i - pastBuffStartIdx];
		}

		int i = 0;
		for (; i < pastBuffStartIdx + bufferLength - pastBuffLen; i++) {
			pastBuff[i] = inBuff[pastBuffLen - pastBuffStartIdx + i];
		}

		spc->startIdx = i;

	} else {

		for (int i = 0; i < bufferLength; i++) {
			pastBuff[pastBuffStartIdx + i] = inBuff[i];
		}

		spc->startIdx += bufferLength;

	}

	// update start idx

	const int delaySamples = delay * sampleRate;
	int baseIdx = pastBuffStartIdx - 1;
	
	for (int i = 0; i < bufferLength; i++) {
	
		const int idx = baseIdx - floor(delaySamples * (0.5 + 0.5 * sin(phase)));
		if (idx > 470) {
			int x = 0;
			int y = 2 + x;
		}
		outBuff[i] = pastBuff[(idx < 0) ? pastBuffLen + idx : idx];

		baseIdx = (baseIdx + 1 >= pastBuffLen) ? 0 : baseIdx + 1;
		phase = NEW_PHASE(phase);

	}

	spc->phase = phase;

}

void rateChange(PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	Space* const spc = (Space*) source->plugin->space;

	spc->rate = source->value;

}

void delayChange(PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	Space* const spc = (Space*) source->plugin->space;

	spc->delay = source->value;

}

IPlugin* getPlugin() {

	IPlugin* plugin = (IPlugin*)malloc(sizeof(IPlugin));
	if (plugin == NULL) return NULL;

	// ui
	PluginUIHandler* uihnd = buildPluginUIHandler();
	uihnd->controls[0]->backgroundColor = 0xFFEAC863;
	uihnd->controls[0]->fillType = PFP_DOTS;

	PluginControl* freqKnob = addControl(uihnd, PCT_KNOB);
	freqKnob->color = TEXT_COLOR;
	freqKnob->MIN_VALUE = MIN_RATE;
	freqKnob->MAX_VALUE = MAX_RATE;
	freqKnob->value = DF_RATE;
	freqKnob->label = "Rate";
	freqKnob->eChange = &rateChange;

	PluginControl* delayKnob = addControl(uihnd, PCT_KNOB);
	delayKnob->color = TEXT_COLOR;
	delayKnob->MIN_VALUE = MIN_DELAY;
	delayKnob->MAX_VALUE = MAX_DELAY;
	delayKnob->value = DF_DELAY;
	delayKnob->label = "Delay";
	delayKnob->eChange = &delayChange;

	// plugin itself
	plugin->name = L"Vibrato";
	plugin->uihnd = uihnd;
	plugin->init = &init;
	plugin->free = &freePlugin;
	plugin->process = &process;

	return plugin;

}
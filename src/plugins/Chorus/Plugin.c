#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "Plugin.h"
#include <math.h>
#include <stdlib.h>

#define M_PI 3.14159265358979323846

#define TEXT_COLOR 0xFF000000

#define MIN_RATIO 0.0
#define MAX_RATIO 1.0
#define DF_RATIO 0.5

#define MIN_VOICE_COUNT 1
#define MAX_VOICE_COUNT 6
#define DF_VOICE_COUNT 3

#define NEW_PHASE(phase) (((phase) > 2 * M_PI) ? (phase) - 2 * M_PI : (phase) + 2 * M_PI * rate / sampleRate);

typedef struct Voice {

	double phase;
	double rate;
	double ratio;
	int delay; // in samples
	int delayOffset; // in samples

} Voice;

typedef struct Space {

	int sampleRate;
	
	double ratio;
	double feedbackRatio;

	int voiceCount;
	Voice* voices;

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
	}
	else {
		*space = malloc(sizeof(Space));
		if (!*space) return 1;
	}

	Space* const spc = (Space*)*space;

	spc->voices = malloc(MAX_VOICE_COUNT * sizeof(Voice));
	if (!spc->voices) return 1;

	const double delayTime = 0.002;
	const double delaySamples = info->sampleRate * delayTime;

	double delayTmp = delaySamples;
	for (int i = 0; i < MAX_VOICE_COUNT; i++) {
		(spc->voices + i)->phase = 0;
		(spc->voices + i)->delayOffset = delayTmp;
		delayTmp += delayTmp;
	}

	(spc->voices + 0)->delay = 0.01;
	(spc->voices + 1)->delay = 0.012;
	(spc->voices + 2)->delay = 0.009;
	(spc->voices + 3)->delay = 0.0;
	(spc->voices + 4)->delay = 0.07;
	(spc->voices + 5)->delay = 0.02;

	(spc->voices + 0)->rate = 0.01;
	(spc->voices + 1)->rate = 0.04;
	(spc->voices + 2)->rate = 0.08;
	(spc->voices + 3)->rate = 0.04;
	(spc->voices + 4)->rate = 0.07;
	(spc->voices + 5)->rate = 0.02;

	(spc->voices + 0)->ratio = 0.96;
	(spc->voices + 1)->ratio = 0.90;
	(spc->voices + 2)->ratio = 0.92;
	(spc->voices + 3)->ratio = 0.95;
	(spc->voices + 4)->ratio = 0.97;
	(spc->voices + 5)->ratio = 0.94;

	spc->ratio = DF_RATIO;
	spc->voiceCount = 3;

	spc->sampleRate = info->sampleRate;

	spc->startIdx = 0;
	spc->length = 0.5 * info->sampleRate + info->maxBufferLength;
	spc->pastBuffer = (double*)calloc(spc->length, sizeof(double));
	if (!spc->pastBuffer) return 1;

	return 0;

}

void process(void* inBuffer, void* outBuffer, int bufferLength, void* space) {

	double* const inBuff = inBuffer;
	double* const outBuff = outBuffer;

	Space* const spc = (Space*)(space);
	
	double globalRatio = spc->ratio;
	double feedbackRatio = spc->feedbackRatio;

	int voiceCount = spc->voiceCount;
	Voice* const voices = spc->voices;

	const double sampleRate = spc->sampleRate;

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

	// copy input first
	memcpy(outBuff, inBuff, sizeof(double) * bufferLength);
	
	// then add voices
	for (int j = 0; j < voiceCount; j++) {

		double phase = voices[j].phase;
		const double rate = voices[j].rate;
		const double ratio = voices[j].ratio;
		const int delay = voices[j].delay;

		int baseIdx = pastBuffStartIdx - 1 - voices[j].delayOffset;
		for (int i = 0; i < bufferLength; i++) {

			const int idx = baseIdx - floor(delay * (0.5 + 0.5 * sin(phase)));
			outBuff[i] += globalRatio * (ratio * (pastBuff[(idx < 0) ? pastBuffLen + idx : idx]));

			baseIdx = (baseIdx + 1 >= pastBuffLen) ? 0 : baseIdx + 1;
			phase = NEW_PHASE(phase);

		}

		(voices + j)->phase = phase;
	}

}

void ratioChange(PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	Space* const spc = (Space*) source->plugin->space;

	spc->ratio = source->value;

}

void voiceCountChange(PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	Space* const spc = (Space*) source->plugin->space;

	spc->voiceCount = (int) source->value;

}

IPlugin* getPlugin() {

	IPlugin* plugin = (IPlugin*)malloc(sizeof(IPlugin));
	if (plugin == NULL) return NULL;

	// ui
	PluginUIHandler* uihnd = buildPluginUIHandler();
	uihnd->controls[0]->backgroundColor = 0xFFEAC863;
	uihnd->controls[0]->fillType = PFP_DOTS;

	PluginControl* ratioKnob = addControl(uihnd, PCT_KNOB);
	ratioKnob->color = TEXT_COLOR;
	ratioKnob->MIN_VALUE = MIN_RATIO;
	ratioKnob->MAX_VALUE = MAX_RATIO;
	ratioKnob->value = DF_RATIO;
	ratioKnob->label = "Ratio";
	ratioKnob->eChange = &ratioChange;

	PluginControl* voiceCountKnob = addControl(uihnd, PCT_STEP_KNOB);
	voiceCountKnob->color = TEXT_COLOR;
	voiceCountKnob->MIN_VALUE = MIN_VOICE_COUNT;
	voiceCountKnob->MAX_VALUE = MAX_VOICE_COUNT;
	voiceCountKnob->step = 1;
	voiceCountKnob->minValue = 0;
	voiceCountKnob->maxValue = MAX_VOICE_COUNT;
	voiceCountKnob->value = DF_VOICE_COUNT;
	voiceCountKnob->label = "Voices";
	voiceCountKnob->eChange = &voiceCountChange;

	// plugin itself
	plugin->name = L"Chorus";
	plugin->uihnd = uihnd;
	plugin->init = &init;
	plugin->free = &freePlugin;
	plugin->process = &process;

	return plugin;

}
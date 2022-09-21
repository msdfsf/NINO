#pragma once
#define _CRT_SECURE_NO_WARNINGS

// https://thewolfsound.com/allpass-filter/
// https://www.w3.org/TR/audio-eq-cookbook/

#include "Plugin.h"
#include <math.h>
#include <stdlib.h>

#define M_PI 3.14159265358979323846

#define TEXT_COLOR 0xFF000000
#define BACK_COLOR 0xFFEAC863

#define MIN_RATE 0
#define MAX_RATE 1
#define DF_RATE 0.5

#define MIN_RATIO 0
#define MAX_RATIO 1
#define DF_RATIO 0.6

#define MIN_FREQ 100
#define MAX_FREQ 10000

#define MIN_ALL_PASS_COUNT 1
#define MAX_ALL_PASS_COUNT 32
#define DF_ALL_PASS_COUNT 4

#define NEW_PHASE(phase) (((phase) > 2 * M_PI) ? (phase) - 2 * M_PI : (phase) + 2 * M_PI * rate / sampleRate);

typedef struct Space {

	int sampleRate;

	double phase;
	double rate;
	double ratio;

	double c;

	// stroed last input
	double x1;
	double x2;

	int allPassCount;
	double* allPassBuffer; // storing out values for each filter [y1 - 1, y1 - 2, ... , yn - 1, yn - 2]

} Space;

int freePlugin(void* space) {

	free(((Space*) space)->allPassBuffer);

}

int init(IPluginInfo* info, void** space) {

	if (*space) {
		freePlugin(*space);
	} else {
		*space = malloc(sizeof(Space));
		if (!*space) return 1;
	}

	Space* const spc = (Space*) *space;

	spc->rate = DF_RATE;
	spc->phase = 0;
	spc->ratio = DF_RATIO;

	spc->sampleRate = info->sampleRate;

	spc->x1 = 0;
	spc->x2 = 0;

	const double band = 0.08; // BW / sampleRate;
	spc->c = (tan(M_PI * band) - 1) / (tan(M_PI * band) + 1);

	spc->allPassCount = DF_ALL_PASS_COUNT;
	spc->allPassBuffer = (double*) calloc(2 * MAX_ALL_PASS_COUNT, sizeof(double));
	if (!spc->allPassBuffer) return 1;

	return 0;

}

void process(void* inBuffer, void* outBuffer, int bufferLength, void* space) {

	double* const inBuff = inBuffer;
	double* const outBuff = outBuffer;

	Space* const spc = (Space*)(space);

	const double c = spc->c;
	const double sampleRate = spc->sampleRate;

	double phase = spc->phase;
	const double ratio = spc->ratio;
	const double rate = spc->rate;

	const int allPassCount = spc->allPassCount;
	double* const allPassBuff = spc->allPassBuffer;

	double x1 = spc->x1;
	double x2 = spc->x2;

	// save input at first, as for now inBuffer and outBuffer points to the same memory
	spc->x1 = inBuff[bufferLength - 1];
	spc->x2 = inBuff[bufferLength - 2];

	for (int i = 0; i < bufferLength; i++) {

		const double x = inBuff[i]; // 'pure' input value to mix in the end, as it will be rewriten

		const double fc = MIN_FREQ + 0.5 * MAX_FREQ + 0.5 * MAX_FREQ * sin(phase);
		const double d = -cos(2 * M_PI * fc / sampleRate);
		const double b0 = -c;
		const double b1 = d * (1 - c);

		// out values of all pass pipeline, input for each all pass filter
		double y = x;
		double y1 = x1;
		double y2 = x2;
		for (int j = 0; j < allPassCount; j++) {

			const double lastOut = allPassBuff[2 * j];
			const double preLastOut = allPassBuff[2 * j + 1];
			
			const double out = b0 * y + b1 * y1 + y2 - b1 * lastOut - b0 * preLastOut;

			y2 = preLastOut;
			y1 = lastOut;
			y = out;

			allPassBuff[2 * j] = y;
			allPassBuff[2 * j + 1] = lastOut;
		
		}


		outBuff[i] = x + ratio * y;

		x2 = x1;
		x1 = x;

		phase = NEW_PHASE(phase);

	}

	spc->phase = phase;

}

void rateChange(PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	Space* const spc = (Space*)source->plugin->space;

	spc->rate = source->value;

}

void ratioChange(PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	Space* const spc = (Space*)source->plugin->space;

	spc->ratio = source->value;

}

void countChange(PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	Space* const spc = (Space*)source->plugin->space;

	spc->allPassCount = source->value;

}

IPlugin* getPlugin() {

	IPlugin* plugin = (IPlugin*)malloc(sizeof(IPlugin));
	if (plugin == NULL) return NULL;

	// ui
	PluginUIHandler* uihnd = buildPluginUIHandler();
	uihnd->controls[0]->backgroundColor = BACK_COLOR;
	uihnd->controls[0]->fillType = PFP_DOTS;

	PluginControl* freqKnob = addControl(uihnd, PCT_KNOB);
	freqKnob->color = TEXT_COLOR;
	freqKnob->MIN_VALUE = MIN_RATE;
	freqKnob->MAX_VALUE = MAX_RATE;
	freqKnob->value = DF_RATE;
	freqKnob->label = "Rate";
	freqKnob->eChange = &rateChange;

	PluginControl* ratioKnob = addControl(uihnd, PCT_KNOB);
	ratioKnob->color = TEXT_COLOR;
	ratioKnob->MIN_VALUE = MIN_RATIO;
	ratioKnob->MAX_VALUE = MAX_RATIO;
	ratioKnob->value = DF_RATIO;
	ratioKnob->label = "Ratio";
	ratioKnob->eChange = &ratioChange;

	PluginControl* countKnob = addControl(uihnd, PCT_STEP_KNOB);
	countKnob->color = TEXT_COLOR;
	countKnob->MIN_VALUE = MIN_ALL_PASS_COUNT;
	countKnob->MAX_VALUE = MAX_ALL_PASS_COUNT;
	countKnob->step = 4;
	countKnob->minValue = 0;
	countKnob->maxValue = MAX_ALL_PASS_COUNT;
	countKnob->value = DF_ALL_PASS_COUNT;
	countKnob->label = "All Pass Count";
	countKnob->eChange = &countChange;

	// plugin itself
	plugin->name = L"Phaser";
	plugin->uihnd = uihnd;
	plugin->init = &init;
	plugin->free = &freePlugin;
	plugin->process = &process;

	return plugin;

}
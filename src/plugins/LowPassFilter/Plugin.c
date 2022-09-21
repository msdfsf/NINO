
// https://en.wikipedia.org/wiki/Distortion_(music)
// https://manual.audacityteam.org/man/distortion.html

#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "Plugin.h"
#include <math.h>
#include <stdlib.h>

#define M_PI 3.14159265358979323846

#define TEXT_COLOR 0xFF000000

#define MIN_FREQUENCY 20.0
#define MAX_FREQUENCY 1000.0
#define DF_FREQUENCY 300.0

#define MIN_QUALITY_FACTOR 0.01
#define MAX_QUALITY_FACTOR 2.0
#define DF_QUALITY_FACTOR 0.8

typedef struct Space {

	int sampleRate;

	double frequency;
	double qualityFactor;

	double inCoef0;
	double inCoef1;
	double inCoef2;

	double outCoef1;
	double outCoef2;

	// stroed last input
	double x1;
	double x2;

	// stored last output
	double y1;
	double y2;

} Space;

void setCoefs(Space* const spc, const double fc, const double Q) {

	const double w0 = (2 * M_PI * fc) / (double) spc->sampleRate;
	const double a = sin(w0) / (2 * Q);

	const double b0 = (1 - cos(w0)) / 2;
	const double b1 = 1 - cos(w0);
	const double b2 = (1 - cos(w0)) / 2;

	const double a0 = 1 + a;
	const double a1 = -2 * cos(w0);
	const double a2 = 1 - a;

	spc->inCoef0 = (b0 / a0);
	spc->inCoef1 = (b1 / a0);
	spc->inCoef2 = (b2 / a0);

	spc->outCoef1 = (a1 / a0);
	spc->outCoef2 = (a2 / a0);

}


int init(IPluginInfo* info, void** space) {

	*space = malloc(sizeof(Space));
	if (!*space) return 1;

	((Space*) *space)->frequency = DF_FREQUENCY;
	((Space*) *space)->qualityFactor = DF_QUALITY_FACTOR;

	((Space*) *space)->sampleRate = info->sampleRate;

	((Space*) *space)->x1 = 0;
	((Space*) *space)->x2 = 0;

	((Space*) *space)->y1 = 0;
	((Space*) *space)->y2 = 0;

	setCoefs(*space, DF_FREQUENCY, DF_QUALITY_FACTOR);

	return 0;

}

void process(void* inBuffer, void* outBuffer, int bufferLength, void* space) {

	double* const inBuff = inBuffer;
	double* const outBuff = outBuffer;

	Space* const spc = (Space*) (space);

	const double inCoef0 = spc->inCoef0;
	const double inCoef1 = spc->inCoef1;
	const double inCoef2 = spc->inCoef2;

	const double outCoef1 = spc->outCoef1;
	const double outCoef2 = spc->outCoef2;

	double x1 = spc->x1;
	double x2 = spc->x2;

	double y1 = spc->y1;
	double y2 = spc->y2;

	// save input at first, as for now inBuffer and outBuffer points to the same memory
	spc->x1 = inBuff[bufferLength - 1];
	spc->x2 = inBuff[bufferLength - 2];

	for (int i = 0; i < bufferLength; i++) {
		
		const double x = inBuff[i];

		const double y = inCoef0 * x + inCoef1 * x1 + inCoef2 * x2 - outCoef1 * y1 - outCoef2 * y2;

		outBuff[i] = y;

		x2 = x1;
		x1 = x;

		y2 = y1;
		y1 = y;

	}

	spc->y1 = y1;
	spc->y2 = y2;

}

void freqChange(PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	Space* const spc = (Space*)source->plugin->space;
	
	spc->frequency = source->value;
	setCoefs(spc, spc->frequency, spc->qualityFactor);

}

void qFactorChange(PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	Space* const spc = (Space*)source->plugin->space;

	spc->qualityFactor = source->value;
	setCoefs(spc, spc->frequency, spc->qualityFactor);

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
	freqKnob->MIN_VALUE = MIN_FREQUENCY;
	freqKnob->MAX_VALUE = MAX_FREQUENCY;
	freqKnob->value = DF_FREQUENCY;
	freqKnob->label = "Frequency";
	freqKnob->eChange = &freqChange;

	PluginControl* qFactorKnob = addControl(uihnd, PCT_KNOB);
	qFactorKnob->color = TEXT_COLOR;
	qFactorKnob->MIN_VALUE = MIN_QUALITY_FACTOR;
	qFactorKnob->MAX_VALUE = MAX_QUALITY_FACTOR;
	qFactorKnob->value = DF_QUALITY_FACTOR;
	qFactorKnob->label = "Q Factor";
	qFactorKnob->eChange = &qFactorChange;

	// plugin itself
	plugin->name = L"Low Pass Filter";
	plugin->uihnd = uihnd;
	plugin->init = &init;
	plugin->free = NULL;
	plugin->process = &process;

	return plugin;

}

// https://en.wikipedia.org/wiki/Distortion_(music)
// https://manual.audacityteam.org/man/distortion.html

#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "Plugin.h"
#include <limits.h>
#include <math.h>
#include <stdlib.h>

#define M_PI 3.14159265358979323846

#define TEXT_COLOR 0xFF000000
#define MAX_DB 10

#define DF_GATE 5

#define MIN_OUT_GAIN 0
#define MAX_OUT_GAIN 10
#define DF_OUT_GAIN 1

#define MIN_DRIVE 0
#define MAX_DRIVE 10
#define DF_DRIVE 5

#define MIN_TONE 0
#define MAX_TONE 1
#define DF_TONE 0.5

#define PRE_HIGH_PASS_FREQ 33
#define TONE_LOW_PASS_FREQ 234
#define TONE_HIGH_PASS_FREQ 1063

#define PRE_HIGH_PASS_Q 0.05
#define TONE_HIGH_PASS_Q 0.05
#define TONE_LOW_PASS_Q 0.05

#define DB_TO_MAG(db) pow(10, (db) / 20);

#define FILTER_CALCULATE(f, x) (f).in0 * (x) + (f).in1 * (f).x1 + (f).in2 * (f).x2 - (f).out1 * (f).y1 - (f).out2 * (f).y2;
#define FILTER_UPDATE_BUFFER(f, x, y) f->x2 = f->x1; f->x1 = (x); f->y2 = f->y1; f->y1 = (y);
	

typedef struct Filter {

	double x1;
	double x2;

	double y1;
	double y2;

	double in0;
	double in1;
	double in2;

	double out1;
	double out2;

} Filter;

typedef struct Space {

	double outGainCoef;
	double inGainCoef;
	double toneLowPassRatio;
	double toneHighPassRatio;

	int sampleRate;

	Filter preHgPass;
	Filter toneHgPass;
	Filter toneLwPass;

} Space;

void setInGainCoef(Space* const spc, const double value) {

	// db
	const double maxGain = 70;//56;
	const double minGain = 36;//36;

	const double ratio = (value - MIN_DRIVE) / (MAX_DRIVE - MIN_DRIVE);
	const double gain = minGain + (maxGain - minGain) * ratio;

	spc->inGainCoef = DB_TO_MAG(gain);

}

void setOutGainCoef(Space* const spc, const double value) {

	const double val = MAX_DRIVE - value;

	// db
	const double maxGain = 5;
	const double minGain = 0;

	const double ratio = (val - MIN_OUT_GAIN) / (MAX_OUT_GAIN - MIN_OUT_GAIN);
	const double gain = minGain + (maxGain - minGain) * ratio;

	spc->outGainCoef = DB_TO_MAG(gain);
	// spc->outGainCoef = 1 / DB_TO_MAG(gain);

}

void setHighPass(Filter* const filter, const double fc, const double Q, const int fs) {

	const double w0 = 2 * M_PI * fc / (double) fs;
	const double a = sin(w0) / (2 * Q);

	const double b0 = (1 + cos(w0)) / 2;
	const double b1 = -1 - cos(w0);
	const double b2 = (1 + cos(w0)) / 2;

	const double a0 = 1 + a;
	const double a1 = -2 * cos(w0);
	const double a2 = 1 - a;

	filter->in0 = (b0 / a0);
	filter->in1 = (b1 / a0);
	filter->in2 = (b2 / a0);

	filter->out1 = (a1 / a0);
	filter->out2 = (a2 / a0);

}

void setLowPass(Filter* const filter, const double fc, const double Q, const int fs) {

	const double w0 = (2 * M_PI * fc) / (double) fs;
	const double a = sin(w0) / (2 * Q);

	const double b0 = (1 - cos(w0)) / 2;
	const double b1 = 1 - cos(w0);
	const double b2 = (1 - cos(w0)) / 2;

	const double a0 = 1 + a;
	const double a1 = -2 * cos(w0);
	const double a2 = 1 - a;

	filter->in0 = (b0 / a0);
	filter->in1 = (b1 / a0);
	filter->in2 = (b2 / a0);

	filter->out1 = (a1 / a0);
	filter->out2 = (a2 / a0);

}

void nullFilterBuffer(Filter* const filter) {

	filter->x1 = 0;
	filter->x2 = 0;

	filter->y1 = 0;
	filter->y2 = 0;

}

int init(IPluginInfo* info, void** space) {

	*space = malloc(sizeof(Space));
	if (!*space) return 1;

	Space* const spc = (Space*) *space;

	setOutGainCoef(spc, DF_OUT_GAIN);
	setInGainCoef(spc, DF_DRIVE);

	spc->toneLowPassRatio = 0.5;
	spc->toneHighPassRatio = 0.5;

	spc->sampleRate = info->sampleRate;

	nullFilterBuffer(&(spc->preHgPass));
	setHighPass(&(spc->preHgPass), PRE_HIGH_PASS_FREQ, PRE_HIGH_PASS_Q, info->sampleRate);

	nullFilterBuffer(&(spc->toneHgPass));
	setHighPass(&(spc->toneHgPass), TONE_HIGH_PASS_FREQ, TONE_HIGH_PASS_Q, info->sampleRate);

	nullFilterBuffer(&(spc->toneLwPass));
	setLowPass(&(spc->toneLwPass), TONE_LOW_PASS_FREQ, TONE_LOW_PASS_Q, info->sampleRate);

	return 0;

}

void process(void* inBuffer, void* outBuffer, int bufferLength, void* space) {

	double* const inBuff = inBuffer;
	double* const outBuff = outBuffer;

	Space* const spc = (Space*) space;

	const double outGainCoef = spc->outGainCoef;
	const double inGainCoef = spc->inGainCoef;

	const double toneLowFilterRatio = spc->toneLowPassRatio;
	const double toneHighFilterRatio = spc->toneHighPassRatio;

	Filter* const preHighFilter = &(spc->preHgPass);
	Filter* const toneLowFilter = &(spc->toneLwPass);
	Filter* const toneHighFilter = &(spc->toneHgPass);

	for (int i = 0; i < bufferLength; i++) {

		const double x = inBuff[i] * inGainCoef;

		// pre filter
		const double yPreHigh = FILTER_CALCULATE(*preHighFilter, x);
		FILTER_UPDATE_BUFFER(preHighFilter, x, yPreHigh);

		// clip
		double val = 0;
		if (yPreHigh >= 1) {
			val = 1;
		} else if (yPreHigh <= -1) {
			val = -1;
		} else {
			if (yPreHigh > 0) {
				val = 1 / (-0.5 * yPreHigh - 0.5) + 2;
			} else {
				val = (3 / 2) * (yPreHigh - yPreHigh * yPreHigh * yPreHigh / 3);
			}
		}

		// tone clip
		const double yToneLow = FILTER_CALCULATE(*toneLowFilter, val);
		FILTER_UPDATE_BUFFER(toneLowFilter, val, yToneLow);

		const double yToneHigh = FILTER_CALCULATE(*toneHighFilter, val);
		FILTER_UPDATE_BUFFER(toneHighFilter, val, yToneHigh);

		//outBuff[i] = val;
		outBuff[i] = (toneLowFilterRatio * yToneLow + toneHighFilterRatio * yToneHigh) * outGainCoef;

	}

}

void volumeChange (PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	setOutGainCoef(source->plugin->space, source->value);

}

void driveChange(PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	setInGainCoef(source->plugin->space, source->value);

}

void toneChange(PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	const double ratio = source->value;
	Space* const spc = (Space*)source->plugin->space;

	spc->toneLowPassRatio = 1 - ratio;
	spc->toneHighPassRatio = ratio;

}

IPlugin* getPlugin() {
	
	IPlugin* plugin = (IPlugin*) malloc(sizeof(IPlugin));
	if (plugin == NULL) return NULL;

	// ui
	PluginUIHandler* uihnd = buildPluginUIHandler();
	uihnd->controls[0]->backgroundColor = 0xFFEAC863;
	uihnd->controls[0]->fillType = PFP_DOTS;

	PluginControl* volumeKnob = addControl(uihnd, PCT_KNOB);
	volumeKnob->color = TEXT_COLOR;
	volumeKnob->MIN_VALUE = MIN_OUT_GAIN;
	volumeKnob->MAX_VALUE = MAX_OUT_GAIN;
	volumeKnob->value = DF_OUT_GAIN;
	volumeKnob->label = "Volume";
	volumeKnob->eChange = &volumeChange;

	PluginControl* driveKnob = addControl(uihnd, PCT_KNOB);
	driveKnob->color = TEXT_COLOR;
	driveKnob->MIN_VALUE = MIN_DRIVE;
	driveKnob->MAX_VALUE = MAX_DRIVE;
	driveKnob->value = DF_DRIVE;
	driveKnob->label = "Drive";
	driveKnob->eChange = &driveChange;

	PluginControl* toneKnob = addControl(uihnd, PCT_KNOB);
	toneKnob->color = TEXT_COLOR;
	toneKnob->MIN_VALUE = MIN_TONE;
	toneKnob->MAX_VALUE = MAX_TONE;
	toneKnob->value = DF_TONE;
	toneKnob->label = "Tone";
	toneKnob->eChange = &toneChange;

	// plugin itself
	plugin->name = L"Distortion";
	plugin->uihnd = uihnd;
	plugin->init = &init;
	plugin->free = NULL;
	plugin->process = &process;

	return plugin;

}

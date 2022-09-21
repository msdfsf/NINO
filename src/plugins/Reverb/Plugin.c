#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "Plugin.h"
#include <math.h>
#include <stdlib.h>

#define ALL_PASS_COUNT 3
#define FEEDBACK_COMB_COUNT 4

// ms
#define MIN_COMB_DELAY 10
#define MAX_COMB_DELAY 100

#define COMB_1_DELAY 30
#define COMB_2_DELAY 32
#define COMB_3_DELAY 36
#define COMB_4_DELAY 33

#define CALC_ALL_PASS_LEN(idx, fs) ceil( 0.1 * (fs) / pow(3, (idx)) )

typedef struct AllPass {
	
	double coef;
	int buffIdx;
	int buffLen;
	double* inBuff;
	double* outBuff;

} AllPass;

typedef struct FeedbackComb {

	double coef;
	int buffIdx;
	int buffLen;
	int delayLen;
	double* buff;

} FeedbackComb;

typedef struct Space {

	int sampleRate;

	AllPass* allPass;
	FeedbackComb* feedbackComb;

} Space;

int init(IPluginInfo* info, void** space) {

	*space = malloc(sizeof(Space));
	if (*space == NULL) return 1;

	Space* spc = (Space*) *space;

	spc->sampleRate = info->sampleRate;

	spc->allPass = malloc(ALL_PASS_COUNT * sizeof(AllPass));
	if (!spc->allPass) return 1;

	for (int i = 0; i < ALL_PASS_COUNT; i++) {
		spc->allPass[i].coef = 0.7;
		spc->allPass[i].buffIdx = 0;
		spc->allPass[i].buffLen = CALC_ALL_PASS_LEN(i, info->sampleRate);
		spc->allPass[i].inBuff = calloc(spc->allPass[i].buffLen * sizeof(double), 1);
		spc->allPass[i].outBuff = calloc(spc->allPass[i].buffLen * sizeof(double), 1);
	}

	spc->feedbackComb = malloc(FEEDBACK_COMB_COUNT * sizeof(FeedbackComb));
	if (!spc->feedbackComb) return 1;

	for (int i = 0; i < FEEDBACK_COMB_COUNT; i++) {
		spc->feedbackComb[i].coef = 0.7;
		spc->feedbackComb[i].buffIdx = 0;
		spc->feedbackComb[i].buffLen = ceil((MAX_COMB_DELAY / (double) 1000) * info->sampleRate);
		spc->feedbackComb[i].buff = calloc(spc->feedbackComb[i].buffLen * sizeof(double), 1);
	}

	spc->feedbackComb[0].delayLen = ceil((COMB_1_DELAY / 1000) * info->sampleRate);
	spc->feedbackComb[1].delayLen = ceil((COMB_1_DELAY / 1000) * info->sampleRate);
	spc->feedbackComb[2].delayLen = ceil((COMB_1_DELAY / 1000) * info->sampleRate);
	spc->feedbackComb[3].delayLen = ceil((COMB_1_DELAY / 1000) * info->sampleRate);

	return 0;

}

void process(void* inBuffer, void* outBuffer, int bufferLength, void* space) {

	double* const inBuff = inBuffer;
	double* const outBuff = outBuffer;

	Space* const spc = (Space*) space;

	AllPass* const allPass = spc->allPass;
	FeedbackComb* const feedbackComb = spc->feedbackComb;

	for (int i = 0; i < bufferLength; i++) {

		double yy = 0;
		{
			const double x = inBuff[i];

			for (int i = 0; i < FEEDBACK_COMB_COUNT; i++) {

				FeedbackComb* const filter = feedbackComb + i;

				const int idx = (filter->buffIdx - filter->delayLen < 0) ? filter->buffLen + (filter->buffIdx - filter->delayLen) : filter->buffIdx - filter->delayLen;
				const double y = x + filter->coef * filter->buff[idx];

				filter->buffIdx = (filter->buffIdx + 1 >= filter->buffLen) ? 0 : filter->buffIdx + 1;
				filter->buff[filter->buffIdx] = y;

				yy += y;

			}
		}

		double x = yy;

		for (int i = 0; i < ALL_PASS_COUNT; i++) {

			AllPass* const filter = allPass + i;

			const int idx = (filter->buffIdx + 1 >= filter->buffLen) ? 0 : filter->buffIdx + 1;
			const double y = -filter->coef * x + filter->inBuff[idx] + filter->coef * filter->outBuff[idx];

			filter->buffIdx = idx;
			filter->inBuff[idx] = x;
			filter->outBuff[idx] = y;

			x = y;

		}

		outBuff[i] = x;
	
	}

}

void comb1Change(PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	Space* const spc = (Space*) source->plugin->space;

	const int val = ceil((source->value / (double)1000) * spc->sampleRate);
	spc->feedbackComb[0].delayLen = (val > spc->feedbackComb[0].buffLen) ? spc->feedbackComb[0].buffLen - 1 : val;

}

void comb2Change(PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	Space* const spc = (Space*)source->plugin->space;

	const int val = ceil((source->value / (double) 1000) * spc->sampleRate);
	spc->feedbackComb[1].delayLen = (val > spc->feedbackComb[1].buffLen) ? spc->feedbackComb[1].buffLen - 1 : val;

}

void comb3Change(PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	Space* const spc = (Space*)source->plugin->space;

	const int val = ceil((source->value / (double)1000) * spc->sampleRate);
	spc->feedbackComb[2].delayLen = (val > spc->feedbackComb[2].buffLen) ? spc->feedbackComb[2].buffLen - 1 : val;

}

void comb4Change(PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	Space* const spc = (Space*)source->plugin->space;

	const int val = ceil((source->value / (double)1000) * spc->sampleRate);
	spc->feedbackComb[3].delayLen = (val > spc->feedbackComb[3].buffLen) ? spc->feedbackComb[3].buffLen - 1 : val;

}

IPlugin* getPlugin() {

	IPlugin* plugin = (IPlugin*)malloc(sizeof(IPlugin));
	if (plugin == NULL) return NULL;

	PluginUIHandler* uihnd = buildPluginUIHandler();
	uihnd->controls[0]->backgroundColor = 0xFFEA6363;
	uihnd->controls[0]->fillType = PFP_DOTS;

	PluginControl* comb1Knob = addControl(uihnd, PCT_KNOB);
	comb1Knob->MIN_VALUE = MIN_COMB_DELAY;
	comb1Knob->MAX_VALUE = MAX_COMB_DELAY;
	comb1Knob->value = 30;
	comb1Knob->color = 0xFF000000;
	comb1Knob->label = "Delay1";
	comb1Knob->eChange = &comb1Change;

	PluginControl* comb2Knob = addControl(uihnd, PCT_KNOB);
	comb2Knob->MIN_VALUE = MIN_COMB_DELAY;
	comb2Knob->MAX_VALUE = MAX_COMB_DELAY;
	comb2Knob->value = 32;
	comb2Knob->color = 0xFF000000;
	comb2Knob->label = "Delay2";
	comb2Knob->eChange = &comb2Change;

	PluginControl* comb3Knob = addControl(uihnd, PCT_KNOB);
	comb3Knob->MIN_VALUE = MIN_COMB_DELAY;
	comb3Knob->MAX_VALUE = MAX_COMB_DELAY;
	comb3Knob->value = 36;
	comb3Knob->color = 0xFF000000;
	comb3Knob->label = "Delay3";
	comb3Knob->eChange = &comb3Change;

	PluginControl* comb4Knob = addControl(uihnd, PCT_KNOB);
	comb4Knob->MIN_VALUE = MIN_COMB_DELAY;
	comb4Knob->MAX_VALUE = MAX_COMB_DELAY;
	comb4Knob->value = 33;
	comb4Knob->color = 0xFF000000;
	comb4Knob->label = "Delay4";
	comb4Knob->eChange = &comb4Change;

	plugin->name = L"Reverb";
	plugin->uihnd = uihnd;
	plugin->process = &process;
	plugin->init = &init;
	plugin->free = NULL;

	return plugin;

}
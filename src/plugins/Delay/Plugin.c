
#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "Plugin.h"
#include <stdlib.h>
#include <math.h>

// in db
#define MIN_VOLUME 0
#define MAX_VOLUME 10
#define DF_VOLUME 5

// in sec
#define MIN_DELAY 0.01
#define MAX_DELAY 1
#define DF_DELAY 0.5

// times
#define MIN_FEEDBACK_RATIO 0
#define MAX_FEEDBACK_RATIO 1 
#define DF_FEEDBACK_RATIO 0

#define FRONT_COLOR 0xFFFFFFFF
#define BACK_COLOR 0xFF639DEA

typedef struct Space {

	double volume;
	double feedback;
	double delay;

	int sampleRate;

	int startIdx;
	int length;
	double* pastInBuffer;
	double* pastOutBuffer;

	int absoluteEndIdx;

} Space;

void pluginFree(void* space) {
	
	free(((Space*) space)->pastInBuffer);
	free(((Space*) space)->pastOutBuffer);

}


int init(IPluginInfo* info, void** space) {

	const int sampleRate = info->sampleRate;
	const int length = 3 * MAX_DELAY * sampleRate;

	if (*space != NULL) {

		Space* const spc = *space;
		
		free(spc->pastInBuffer);
		free(spc->pastOutBuffer);

		spc->pastInBuffer = calloc(length, sizeof(double));
		if (!((Space*)(*space))->pastInBuffer) {

			free(*space);
			*space = NULL;

			return 1;

		}

		spc->pastOutBuffer = calloc(length, sizeof(double));
		if (!spc->pastInBuffer) {

			free(*space);
			*space = NULL;

			return 1;

		}

		spc->startIdx = 0;
		spc->length = length;

		spc->sampleRate = sampleRate;

		return 0;

	}

	*space = malloc(sizeof(Space));
	if (!*space) return 1;

	Space* const spc = *space;

	spc->volume = DF_VOLUME;
	spc->feedback = DF_FEEDBACK_RATIO;
	spc->delay = DF_DELAY;

	spc->startIdx = 0;
	spc->length = length;

	spc->sampleRate = sampleRate;

	spc->pastInBuffer = calloc(length, sizeof(double));
	if (!spc->pastInBuffer) {

		free(*space);
		*space = NULL;

		return 1;
	
	}

	spc->pastOutBuffer = calloc(length, sizeof(double));
	if (!spc->pastInBuffer) {

		free(*space);
		*space = NULL;

		return 1;

	}

	return 0;

}

void process(void* inBuffer, void* outBuffer, int bufferLength, void* space) {

	Space* const spc = space;

	const double gainCoef = spc->volume;
	const double feedbackRatio = spc->feedback;
	const double delay = spc->delay;

	const sampleRate = spc->sampleRate;

	int buffStartIdx = spc->startIdx;
	const int pastBuffLength = spc->length;
	double* const pastInBuff = spc->pastInBuffer;
	double* const pastOutBuff = spc->pastOutBuffer;

	double* const inBuff = inBuffer;
	double* const outBuff = outBuffer;

	const int delaySamples = sampleRate * delay;

	// [1][2][3][4]...[n]
	//
	// buffStartIdx is idx that will be next segment write from (by next is meant at next function call)

	if (buffStartIdx + bufferLength > pastBuffLength) {

		for (int i = buffStartIdx; i < pastBuffLength; i++) {
			pastInBuff[i] = inBuff[i - buffStartIdx] + feedbackRatio * pastInBuff[i - delaySamples];
		}

		int i = 0;
		for (; i < buffStartIdx + bufferLength - pastBuffLength; i++) {
			pastInBuff[i] = inBuff[i] + feedbackRatio * pastInBuff[pastBuffLength - delaySamples + i];
		}

		spc->startIdx = i;

	}
	else {

		if (buffStartIdx - delaySamples >= 0) {
			
			for (int i = 0; i < bufferLength; i++) {
				pastInBuff[buffStartIdx + i] = inBuff[i] + feedbackRatio * pastInBuff[buffStartIdx - delaySamples + i];
			}

		}
		else {

			if (buffStartIdx - delaySamples <= -bufferLength) {

				const int offset = pastBuffLength + (buffStartIdx - delaySamples);
				for (int i = 0; i < bufferLength; i++) {
					pastInBuff[buffStartIdx + i] = inBuff[i] + feedbackRatio * pastInBuff[offset + i];
				}

			} else {

				const int len = bufferLength + (buffStartIdx - delaySamples);
				const int offset = pastBuffLength + (buffStartIdx - delaySamples);
				const int offsetLen = bufferLength - len;

				int i = 0;
				for (; i < offsetLen; i++) {
					pastInBuff[buffStartIdx + i] = inBuff[i] + feedbackRatio * pastInBuff[offset + i];
				}

				for (; i < bufferLength; i++) {
					pastInBuff[buffStartIdx + i] = inBuff[i] + feedbackRatio * pastInBuff[buffStartIdx - delaySamples + i];
				}

			}
		
		}

		spc->startIdx += bufferLength;

	}

	buffStartIdx = spc->startIdx;

	if ((buffStartIdx - delaySamples) < 0) {
		// start idx is behind the zero, so we count it from the end
		// we supposely can have enough space at least for delay duration
		// so lets check just this.

		const int startIdx = pastBuffLength + buffStartIdx - delaySamples - 1;
		const int length = (pastBuffLength / 2 < bufferLength) ? pastBuffLength / 2 : bufferLength;

		for (int i = 0; i < length; i++) {
			const double val = gainCoef * (inBuff[bufferLength - i - 1] + pastInBuff[startIdx - i]);
			outBuff[bufferLength - i - 1] = val;
		}

		for (int i = length; i < bufferLength; i++) {
			outBuff[bufferLength - i - 1] = gainCoef * inBuff[bufferLength - i - 1];
		}

	} else {
		// start idx is in safe place, but then we cannot be sure about
		// the length we are gona fill, as it can cross zero, so we have to
		// check it.

		const int startIdx = buffStartIdx - delaySamples;
		const int length = (pastBuffLength / 2 < bufferLength) ? pastBuffLength / 2 : bufferLength;

		if (startIdx - length < 0) {
			// overflow

			const endIdx = startIdx - length < 0 ? 0 : startIdx - length;
			for (int i = startIdx - 1; i >= endIdx; i--) {
				const double val = gainCoef * (inBuff[bufferLength - startIdx + i] + pastInBuff[i]);
				outBuff[bufferLength - startIdx + i] = val;
			}

			const overflowOffset = bufferLength - startIdx + endIdx;
			const overflowEndIdx = pastBuffLength - overflowOffset;// pastBuffLength - startIdx - length;
			for (int i = pastBuffLength - 1; i >= overflowEndIdx; i--) {
				const double val = gainCoef * (inBuff[overflowOffset - pastBuffLength + i] + pastInBuff[i]);
				outBuff[overflowOffset - pastBuffLength + i] = val;
			}
		
		} else {
			// we are ok

			const endIdx = startIdx - length < 0 ? 0 : startIdx - length;
			int i = startIdx - 1;

			for (; i >= endIdx; i--) {
				const double val = gainCoef * (inBuff[bufferLength - startIdx + i] + pastInBuff[i]);
				outBuff[bufferLength - startIdx + i] = val;
			}

			for (int j = 0; j < bufferLength - startIdx + i + 1; j++) {
				outBuff[j] = gainCoef * inBuff[j];
			}

		}
	
	}

}

void volumeChange(PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	(((Space*) source->plugin->space)->volume) = pow(10, source->value / 20.0);

}

void repeatChange(PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	(((Space*) source->plugin->space)->feedback) = source->value;

}

void delayChange(PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	((Space*) source->plugin->space)->delay = source->value;

}

IPlugin* getPlugin() {
	
	IPlugin* plugin = (IPlugin*) malloc(sizeof(IPlugin));
	if (plugin == NULL) return NULL;

	// ui
	PluginUIHandler* uihnd = buildPluginUIHandler();
	uihnd->controls[0]->backgroundColor = BACK_COLOR;
	uihnd->controls[0]->fillType = PFP_DOTS;

	PluginControl* volumeKnob = addControl(uihnd, PCT_KNOB);
	volumeKnob->color = FRONT_COLOR;
	volumeKnob->MIN_VALUE = MIN_VOLUME;
	volumeKnob->MAX_VALUE = MAX_VOLUME;
	volumeKnob->value = DF_VOLUME;
	volumeKnob->label = "Volume";
	volumeKnob->eChange = &volumeChange;

	PluginControl* repeatKnob = addControl(uihnd, PCT_KNOB);
	repeatKnob->color = FRONT_COLOR;
	repeatKnob->MIN_VALUE = MIN_FEEDBACK_RATIO;
	repeatKnob->MAX_VALUE = MAX_FEEDBACK_RATIO;
	repeatKnob->value = DF_FEEDBACK_RATIO;
	repeatKnob->label = "Feedback";
	repeatKnob->eChange = &repeatChange;

	PluginControl* delayKnob = addControl(uihnd, PCT_KNOB);
	delayKnob->color = FRONT_COLOR;
	delayKnob->MIN_VALUE = MIN_DELAY;
	delayKnob->MAX_VALUE = MAX_DELAY;
	delayKnob->value = DF_DELAY;
	delayKnob->label = "Delay";
	delayKnob->eChange = &delayChange;

	// plugin itself
	plugin->name = L"Delay";
	plugin->uihnd = uihnd;
	plugin->init = &init;
	plugin->process = &process;
	plugin->free = &pluginFree;

	return plugin;

}
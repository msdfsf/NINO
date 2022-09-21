
// https://ccrma.stanford.edu/~jos/pasp/Soft_Clipping.html

#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "Plugin.h"
#include <stdlib.h>

#define M_PI 3.14159265358979323846

#define TEXT_COLOR 0xFF000000

#define MIN_CLIP 0
#define MAX_CLIP 1
#define DF_CLIP 0.5

typedef struct Space {

	double clip;

} Space;

int init(IPluginInfo* info, void** space) {

	*space = malloc(sizeof(Space));
	if (!*space) return 1;

	((Space*) *space)->clip = 0;

	return 0;

}

void process(void* inBuffer, void* outBuffer, int bufferLength, void* space) {

	double* const inBuff = inBuffer;
	double* const outBuff = outBuffer;

	Space* const spc = (Space*) space;

	const double clip = spc->clip;

	for (int i = 0; i < bufferLength; i++) {

		const double val = inBuff[i];

		if (val > clip) {
			outBuff[i] = clip;
		} else if (val < -clip) {
			outBuff[i] = -clip;
		} else {
			outBuff[i] = 3 * (val + val * val * val / 3) / 2;
		}

	}



}

void clipChange(PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	(((Space*)source->plugin->space)->clip) = source->value;

}

IPlugin* getPlugin() {

	IPlugin* plugin = (IPlugin*)malloc(sizeof(IPlugin));
	if (plugin == NULL) return NULL;

	// ui
	PluginUIHandler* uihnd = buildPluginUIHandler();
	uihnd->controls[0]->backgroundColor = 0xFFEAC863;
	uihnd->controls[0]->fillType = PFP_DOTS;

	PluginControl* clipKnob = addControl(uihnd, PCT_KNOB);
	clipKnob->color = TEXT_COLOR;
	clipKnob->MIN_VALUE = MIN_CLIP;
	clipKnob->MAX_VALUE = MAX_CLIP;
	clipKnob->value = DF_CLIP;
	clipKnob->label = "Clip";
	clipKnob->eChange = &clipChange;

	// plugin itself
	plugin->name = L"Soft Clip";
	plugin->uihnd = uihnd;
	plugin->init = &init;
	plugin->free = NULL;
	plugin->process = &process;

	return plugin;

}
#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "Plugin.h"
#include <math.h>
#include <stdlib.h>

#define DF_GAIN_COEF 1

typedef struct Space {

	double gainCoef;

} Space;

int init(IPluginInfo* info, void** space) {

	*space = malloc(sizeof(Space));
	if (*space == NULL) return 1;

	((Space*) (*space))->gainCoef = DF_GAIN_COEF;
	
	return 0;

}

void gainChange(PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	((Space*) source->plugin->space)->gainCoef = pow(10, source->value / 20.0);

}

void process(void* inBuffer, void* outBuffer, int bufferLength, void* space) {

	double* const inBuff = inBuffer;
	double* const outBuff = outBuffer;

	const double gainCoef = ((Space*) space)->gainCoef;
	
	for (int i = 0; i < bufferLength; i++) {
		outBuff[i] = gainCoef * inBuff[i];
	}

}

IPlugin* getPlugin() {
	
	IPlugin* const plugin = (IPlugin*) malloc(sizeof(IPlugin));
	if (plugin == NULL) return NULL;

	PluginUIHandler* const uihnd = buildPluginUIHandler();
	uihnd->controls[0]->backgroundColor = 0xFFEA6363;
	uihnd->controls[0]->fillType = PFP_DOTS;

	PluginControl* const gainKnob = addControl(uihnd, PCT_KNOB);
	gainKnob->MAX_VALUE = 30.0;
	gainKnob->value = DF_GAIN_COEF;
	gainKnob->color = 0xFF000000;
	gainKnob->label = "Gain";
	gainKnob->eChange = &gainChange;

	plugin->name = L"Gain Booster";
	plugin->uihnd = uihnd;
	plugin->process = &process;
	plugin->init = &init;
	plugin->free = NULL;

	return plugin;

}

#pragma once

#include "IPlugin.h"
#include <stdlib.h>

// ============================== //
//	UI Handler
// ============================== //

PluginUIHandler* buildPluginUIHandler() {

	PluginUIHandler* uihnd = (PluginUIHandler*) malloc(sizeof(PluginUIHandler));
	if (uihnd == NULL) return NULL;

	const int margin = 10;

	uihnd->controls = NULL;
	uihnd->controlCount = 0;

	addControl(uihnd, PCT_BACKGROUND);

	uihnd->x = 0;
	uihnd->y = 0;

	uihnd->width = 0;
	uihnd->height = 0;

	return uihnd;

}



// ============================== //
//	Controls
// ============================== //

PluginControl* buildPluginControl(PluginControlType type) {

	PluginControl* ctrl = (PluginControl*) malloc(sizeof(PluginControl));
	if (ctrl == NULL) return NULL;

	*(int*)&ctrl->type = type;

	*(double*)&ctrl->MIN_VALUE = 0.0;
	*(double*)&ctrl->MAX_VALUE = 1.0;

	ctrl->step = 0;

	ctrl->x = 0;
	ctrl->y = 0;

	ctrl->width = 0;
	ctrl->height = 0;

	ctrl->color = 0xFFFFFFFF;
	ctrl->backgroundColor = 0xFF000000;

	ctrl->fillType = PFP_SOLID_COLOR;

	ctrl->label = (char*)"";

	ctrl->sensitivity = 0.2;
	ctrl->selected = 0;

	ctrl->value = ctrl->MIN_VALUE + (ctrl->MAX_VALUE - ctrl->MIN_VALUE) / 2;

	ctrl->eMouseClick = NULL;
	ctrl->eMouseMove = NULL;
	ctrl->eMouseDblClick = NULL;
	ctrl->eMouseDown = NULL;
	ctrl->eMouseUp = NULL;

	ctrl->eChange = NULL;

	return ctrl;

};

PluginControl* addControl(PluginUIHandler* uihnd, PluginControlType controlType) {

	uihnd->controlCount++;

	PluginControl** tmp = (PluginControl**) realloc(uihnd->controls, uihnd->controlCount * sizeof(PluginControl*));
	if (tmp == NULL) {
		uihnd->controlCount--;
		return NULL;
	}
	uihnd->controls = tmp;

	uihnd->controls[uihnd->controlCount - 1] = buildPluginControl(controlType);

	return uihnd->controls[uihnd->controlCount - 1];

}

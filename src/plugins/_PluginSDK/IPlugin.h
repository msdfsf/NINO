#pragma once

#include <wchar.h>

#define NULL 0
typedef long long CTRL_PARAM;

typedef enum PluginControlType {

	PCT_BACKGROUND,
	PCT_KNOB,
	PCT_STEP_KNOB,
	PCT_SIGNAL_VIEWER,
	PCT_FREQUENCY_VIEWER

} PluginControlType;

typedef enum PluginFillPattern {

	PFP_SOLID_COLOR,
	PFP_CHECKERBOARD,
	PFP_DOTS,

} PluginFillPattern;

typedef struct IPlugin IPlugin;
typedef struct IPluginInfo IPluginInfo;
typedef struct PluginUIHandler PluginUIHandler;
typedef struct PluginControl PluginControl;



// ============================== //
//	Main Interface
// ============================== //

struct IPlugin {

	wchar_t* name;

	PluginUIHandler* uihnd;

	int (*init) (IPluginInfo* info, void** space);
	void (*free) (void* space);

	void (*process) (void* inBuffer, void* outBuffer, int bufferLength, void* space);

	int state;

	void* space;

};



// ============================== //
//	Info
// ============================== //

struct IPluginInfo {

	int sampleRate;
	int maxBufferLength; // in doubles

};



// ============================== //
//	UI Handler
// ============================== //

struct PluginUIHandler {

	// coords of component, program will set them
	int x;
	int y;

	// overall height and width of component, program will set them
	int width;
	int height;

	int maxTopY;
	int maxBottomY;

	int visible;

	PluginControl** controls;
	int controlCount;

	IPlugin* plugin;

};

PluginControl* addControl(PluginUIHandler* uihnd, PluginControlType controlType);

PluginUIHandler* buildPluginUIHandler();



// ============================== //
//	Controls
// ============================== //

struct PluginControl {

	IPlugin* plugin;

	int type;

	// logic
	double MIN_VALUE;
	double MAX_VALUE;

	double minValue;
	double maxValue;
	double value;
	double step;

	// ui
	int x;
	int y;

	int width;
	int height;

	int color;
	int backgroundColor;

	int fillType;

	char* label;

	// controls
	double sensitivity;
	int selected;

	// events
	void (*eMouseClick) (PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB);
	void (*eMouseMove) (PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB);
	void (*eMouseDblClick) (PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB);
	void (*eMouseDown) (PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB);
	void (*eMouseUp) (PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB);

	void (*eChange) (PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB);

};

struct plotInfo {

	int lenSamples;
	int sampleRate;
	int endIdx;
	double* dataBuffer;
	void* renderBuffer;

};

PluginControl* buildPluginControl(PluginControlType type);
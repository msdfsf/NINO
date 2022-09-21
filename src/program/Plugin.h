#pragma once

#include "IPlugin.h"
#include "ControlEvent.h"

namespace Plugin {

	// recalculates controls values, use to init new controls, or to recalculate properties
	void initDrawPlugin(IPlugin* plugin);

	void drawPlugin(IPlugin* const plugin);

	void processEvent(
		PluginControl** controls,
		const int controlCount,
		ControlEvent::ControlEvent controlEvent,
		CTRL_PARAM paramA,
		CTRL_PARAM paramB
	);

	int copyPlugin(IPlugin* dest, IPlugin* src);
	int copyPluginUIHandelr(PluginUIHandler* const dest, PluginUIHandler* const src);
	void copyPluginControl(PluginControl* const dest, PluginControl* const src);

	void setY(PluginUIHandler* const uihnd, const int y);

	void scrollY(IPlugin** const plugins, const int pluginCount, const int value);

	void setValue(PluginControl* ctrl, double x);

}

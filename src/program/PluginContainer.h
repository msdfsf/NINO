#pragma once

#include "Control.h"
#include "IPlugin.h"

class PluginContainer : public Control {

	public:

		const int rowSpace = 4;

		int pluginCount = 0;
		int maxPluginCount = 0;

		IPlugin** samplePlugins = NULL;
		IPlugin** plugins = NULL;
		Control** pluginsUI = NULL;

		using Control::Control;

		PluginContainer();
		PluginContainer(IPlugin** plugins, int count);

		virtual void draw();
		virtual int processMessage(ControlEvent::ControlEvent controlEvent, CTRL_PARAM paramA, CTRL_PARAM paramB);

		void setPluginCount(int count);
		void removeAll();

		~PluginContainer();

};
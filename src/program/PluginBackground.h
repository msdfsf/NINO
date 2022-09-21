#pragma once

#include "PluginControl.h"

class PluginBackground : public PluginControl {

	public:

		using PluginControl::PluginControl;

		PluginBackground();

		virtual void draw();

		~PluginBackground();

	private:

};

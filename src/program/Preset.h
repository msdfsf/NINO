#pragma once

#include "IPlugin.h"

class Preset {

	public:

		struct Control {
			int count;
			double* values;
		};

		struct State {
			int state;
		};

		static Preset** load(wchar_t* dirname);

		int pluginCount = 0;
		IPlugin** plugins = NULL;
		Control* controls = NULL;
		State* states = NULL;
		
		char* filename;

		char* name;
		int nameLen;

		Preset();

		int addPlugin(IPlugin* const plugin);
		int addAndCopyPlugin(IPlugin* const plugin);
		
		int replacePlugins(IPlugin** const plugins, IPlugin** const samplePlugins, const int pluginCount);
		int save();
		int saveAs(char* const filename);
		int load();

		~Preset();

};

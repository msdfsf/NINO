#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "Preset.h"
#include "Main.h"
#include "FileDriver.h"
#include "Utils.h"

#include <iostream>
#include <cstring>
#include <codecvt>
#include <unordered_map>
#include <stdlib.h>

#include "GlobalVars.h"

int processFile(char* filename, Preset** const ppreset) {

	FILE* file = fopen(filename, "r");
	if (file == NULL) {
		return 1;
	}

	Preset* preset = *ppreset;

	const char SPLIT_SYMBOL = ':';
	const char NEW_LINE = '\n';

	fseek(file, 0, SEEK_END);
	const int fsize = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* data = (char*)malloc((fsize + 1) * sizeof(char));
	fread(data, 1, fsize, file);
	data[fsize] = '\0';
	fclose(file);


	char* presetName;


	const int offset = Utils::trimLeft(data);
	int i = offset;
	int lineStart = i;

	// get first line
	for (; i < fsize; i++) {

		const char ch = data[i];
		if (ch == '\0' || ch == '\n') {
			break;
		}

	}
	presetName = data + offset;
	data[i] = '\0';

	// read plugins, each on separate line
	preset->pluginCount = 0;
	preset->plugins = NULL;
	preset->controls = NULL;
	preset->states = NULL;
	while (i < fsize) {

		char* pluginName;
		char* pluginFolderName;
		char* onoffState = 0;
		int numberOfControls = 0;

		int offset = i + 1;
		i++;
		for (; i < fsize; i++) {

			const char ch = data[i];
			if (ch == '\0' || ch == '\n') {
				break;
			}

		}

		int j = offset;

		// plugin name
		{
			for (; j < i; j++) {

				const char ch = data[j];
				if (ch == SPLIT_SYMBOL) {
					break;
				}

			}

			if (j >= i) {
				continue;
			}

			const int offLeft = Utils::trimLeft(data + offset);
			pluginName = data + offset + offLeft;
			pluginName[Utils::trimRight(data + offLeft + offset, j - offset - offLeft) + 1] = '\0';
		}

		j++;
		offset = j;

		// folder name
		{
			for (; j < i; j++) {

				const char ch = data[j];
				if (ch == SPLIT_SYMBOL) {
					break;
				}

			}

			if (j >= i) {
				continue;
			}

			const int offLeft = Utils::trimLeft(data + offset);
			pluginFolderName = data + offLeft + offset;
			pluginFolderName[Utils::trimRight(data + offLeft + offset, j - offset - offLeft) + 1] = '\0';
		}

		j++;
		offset = j;

		// on/off state
		{
			for (; j < i; j++) {

				const char ch = data[j];
				if (ch == SPLIT_SYMBOL) {
					break;
				}

			}

			if (j >= i) {
				continue;
			}

			const int offLeft = Utils::trimLeft(data + offset);
			onoffState = data + offset + offLeft;
			onoffState[Utils::trimRight(data + offLeft + offset, j - offset - offLeft) + 1] = '\0';
		}

		j++;
		offset = j;

		// number of controls
		{
			for (; j < i; j++) {

				const char ch = data[j];
				if (ch == '[') {
					break;
				}

			}

			if (j >= i) {
				continue;
			}

			const int offLeft = Utils::trimLeft(data + offset);
			char* tmp = data + offLeft + offset;
			tmp[Utils::trimRight(data + offLeft + offset, j - offset - offLeft) + 1] = '\0';
			numberOfControls = strtoll(tmp, NULL, 10);

		}

		j++;
		offset = j;

		// controls value
		double* values = (double*)malloc(sizeof(double) * numberOfControls);
		{
			int idx = 0;
			for (; j < i; j++) {

				const char ch = data[j];

				if (ch == ',' || ch == ']') {

					const int offLeft = Utils::trimLeft(data + offset);
					char* tmp = data + offLeft + offset;
					tmp[Utils::trimRight(data + offLeft + offset, j - offset - offLeft) + 1] = '\0';
					values[idx] = strtod(tmp, NULL);

					idx++;
					if (idx >= numberOfControls) {
						break;
					}

					offset = j + 1;

					if (ch == ']') break;

				}

			}

		}

		IPlugin* plugin = NULL;
		const int nameCount = pluginsNameMap.count(pluginName);
		if (nameCount <= 0) {

			free(values);
			continue;

		}
		else if (nameCount > 1) {

			const int folderCount = pluginsFilenameMap.count(pluginFolderName);
			if (folderCount != 1) {

				free(values);
				continue;

			}
			plugin = pluginsFilenameMap.find(pluginFolderName)->second;

		}
		else {

			plugin = pluginsNameMap.find(pluginName)->second;

		}

		preset->addPlugin(plugin);

		// 0x2ad92a50
		// 0x2ad779f8
		Preset::Control* pctrl = preset->controls + preset->pluginCount - 1;
		pctrl->count = numberOfControls;
		pctrl->values = values;

		Preset::State* pstate = preset->states + preset->pluginCount - 1;
		pstate->state = strtoll(onoffState, NULL, 10);

	}

	if (preset->pluginCount <= 0) {
		free(data);
		//*ppreset = NULL;
		return 1;
	}

	preset->nameLen = strlen(presetName);
	preset->name = (char*)malloc(sizeof(char) * (preset->nameLen + 1));
	if (preset->name == NULL) {
		free(data);
		return 1;
	}

	strcpy(preset->name, presetName);

	free(data);

	return 0;

}

// returned presets are null terminated
Preset** Preset::load(wchar_t* dirname) {

	const int count = FileDriver::countAllFilesInDir(dirname, (wchar_t*)PRESET_EXTENSION_WCHAR);

	char** files = (char**)malloc(sizeof(FILE*) * count);
	if (files == NULL) return NULL;

	Preset** presets = (Preset**)malloc(sizeof(Preset*) * (count + 1));
	if (presets == NULL) return NULL;

	for (int i = 0; i < count; i++) {
		presets[i] = (Preset*)malloc(sizeof(Preset));
		if (presets[i] == NULL) {
			for (int j = 0; j < i; j++) {
				free(presets[j]);
			}
			return NULL;
		}

	}

	FileDriver::getAllFilesInDir(dirname, (wchar_t*)PRESET_EXTENSION_WCHAR, &files, count, "r");

	int j = 0;
	for (int i = 0; i < count; i++) {

		const int dirnameLen = wcslen(dirname);

		char dirnameUtf8[3 * 256 / 2 + 1];
		char* tmp = dirnameUtf8;
		const int dirnameUtf8Len = Utils::wc2utf8(dirname, dirnameLen, &tmp);

		dirnameUtf8[dirnameUtf8Len] = '\0';
		strcat(dirnameUtf8, files[i]);

		if (processFile(dirnameUtf8, &presets[j])) continue;
		presets[j]->filename = files[i];
		j++;
	}

	presets[j] = NULL;
	return presets;

}

int Preset::addPlugin(IPlugin* const plugin) {

	{
		IPlugin** tmp = (IPlugin**)realloc(plugins, (pluginCount + 1) * sizeof(IPlugin*));
		if (tmp == NULL) {
			return 1;
		}
		else {
			plugins = tmp;
		}
	}

	{
		Preset::Control* tmp = (Preset::Control*)realloc(controls, (pluginCount + 1) * sizeof(Preset::Control));
		if (tmp == NULL) {
			return 1;
		}
		else {
			controls = tmp;
			(controls + pluginCount)->count = 0;
			(controls + pluginCount)->values = NULL;
		}
	}

	{
		Preset::State* tmp = (Preset::State*)realloc(states, (pluginCount + 1) * sizeof(Preset::State));
		if (tmp == NULL) {
			return 1;
		}
		else {
			states = tmp;
			(states + pluginCount)->state = 0;
		}
	}

	plugins[pluginCount] = plugin;
	pluginCount++;

	return 0;

}

int Preset::addAndCopyPlugin(IPlugin* const plugin) {

	if (addPlugin(plugin) != 0) {
		return 1;
	}

	Preset::Control* const ctrls = controls + pluginCount - 1;

	PluginUIHandler* const uihnd = plugin->uihnd;
	PluginControl** const pluginCtrls = uihnd->controls;

	const int pluginCtrlCount = uihnd->controlCount - 1;
	ctrls->count = pluginCtrlCount;
	ctrls->values = (double*)malloc(sizeof(double) * pluginCtrlCount);
	if (ctrls->values == NULL) {
		return 2;
	}

	for (int i = 1; i <= pluginCtrlCount; i++) {
		ctrls->values[i - 1] = pluginCtrls[i]->value;
	}

	Preset::State* state = states + pluginCount - 1;
	state->state = plugin->state;

	return 0;

}

int Preset::replacePlugins(IPlugin** const plugins, IPlugin** const samplePlugins, const int pluginCount) {

	const int oldPluginCount = this->pluginCount;

	for (int i = pluginCount; i < oldPluginCount; i++) {
		free((this->controls + i)->values);
	}

	Preset::Control* const tmp = (Preset::Control*)realloc(this->controls, pluginCount * sizeof(Preset::Control));
	IPlugin** const tmp2 = (IPlugin**)realloc(this->plugins, pluginCount * sizeof(IPlugin*));
	Preset::State* const tmp3 = (Preset::State*)realloc(this->states, pluginCount * sizeof(Preset::State));
	if (tmp == NULL || tmp2 == NULL || tmp3 == NULL) {
		Utils::showError("Cannot save file, malloc error!");
		return 1;
	}

	for (int i = 0; i < pluginCount; i++) {

		PluginUIHandler* const uihnd = plugins[i]->uihnd;
		PluginControl** const ctls = uihnd->controls;
		const int ctrlCount = uihnd->controlCount - 1;

		double* const newValues = (double*)realloc((i < oldPluginCount) ? (tmp + i)->values : NULL, ctrlCount * sizeof(double));
		if (newValues == NULL) {
			Utils::showError("Cannot save file, malloc error!");
			return 2;
		}
		(tmp + i)->values = newValues;
		(tmp + i)->count = ctrlCount;

		for (int j = 1; j <= ctrlCount; j++) {
			(tmp + i)->values[j - 1] = ctls[j]->value;
		}

		(tmp3 + i)->state = plugins[i]->state;

		tmp2[i] = samplePlugins[i];

	}

	this->plugins = tmp2;
	this->controls = tmp;
	this->states = tmp3;
	this->pluginCount = pluginCount;

	return 0;
}

int Preset::save() {

	char* const tmpFilename = (char*)"tmp";

	// save to the new tmp file
	if (saveAs(tmpFilename)) {
		return 1;
	}

	// get full filename
	char fullFilename[256 + 1];
	strcpy(fullFilename, PRESET_FOLDER);
	strcpy(fullFilename + strlen(PRESET_FOLDER), filename);

	// delete old preset file
	if (remove(fullFilename)) {
		// file is not deleted, just return and keep tmp file, 
		// just in case user want to manipulate with data anyway
		return 2;
	}

	// and rename tmp one
	if (rename(tmpFilename, fullFilename)) {
		return 3;
	}

	return 0;

}

// flname as utf8 null terminated string
int Preset::saveAs(char* const filename) {

	const char newLine = '\n';
	const char delimiter = ':';
	const char ctrlDelimiter = ',';

	FILE* file = fopen(filename, "w");
	if (file == NULL) return 1;

	// preset name
	fwrite(this->name, sizeof(char), this->nameLen, file);
	fwrite(&newLine, sizeof(char), 1, file);

	// plugins
	const int pluginCount = this->pluginCount;
	IPlugin** const plugins = this->plugins;
	for (int i = 0; i < pluginCount; i++) {

		IPlugin* const plugin = plugins[i];
		Preset::Control* const ctrls = controls + i;
		Preset::State* const state = states + i;

		const int ctrlCount = ctrls->count;

		// name
		char pluginName[3 * 256 + 1];
		char* tmp = pluginName;
		const int pluginNameLen = Utils::wc2utf8((wchar_t*)plugin->name, wcslen(plugin->name), &tmp);
		pluginName[pluginNameLen] = '\0';
		fwrite(tmp, sizeof(char), pluginNameLen, file);
		fwrite(&delimiter, sizeof(char), 1, file);

		// filename
		std::unordered_map<IPlugin*, std::string>::iterator it = pluginToFilename.find(plugin);
		if (it != pluginToFilename.end()) {
			const char* pluginFilename = pluginToFilename.find(plugin)->second.c_str();
			fwrite(pluginFilename, sizeof(char), strlen(pluginFilename) - 4, file);
		}
		fwrite(&delimiter, sizeof(char), 1, file);

		// on off state

		fprintf(file, "%i:", state->state);

		// controls

		// count
		fprintf(file, "%i[", ctrlCount);

		// values
		for (int i = 0; i < ctrlCount - 1; i++) {
			fprintf(file, "%.17g,", ctrls->values[i]);
		}
		fprintf(file, "%.17g]\n", ctrls->values[ctrlCount - 1]);

	}

	fclose(file);

	return 0;

}

Preset::Preset() {

	plugins = NULL;
	controls = NULL;
	states = NULL;

	filename = NULL;
	name = NULL;
	nameLen = 0;

}

Preset::~Preset() {

}

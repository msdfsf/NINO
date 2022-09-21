#pragma once

#include <unordered_map>
#include "IPlugin.h"

extern std::vector<IPlugin*> plugins;
extern std::unordered_multimap<std::string, IPlugin*> pluginsNameMap;
extern std::unordered_multimap<std::string, IPlugin*> pluginsFilenameMap;
extern std::unordered_map<IPlugin*, std::string> pluginToFilename;

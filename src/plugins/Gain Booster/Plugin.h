#pragma once

#include "../_PluginSDK/IPlugin.h"

#ifdef DLLDIR_EX
#define DLLDIR  __declspec(dllexport)   // export DLL information

#else
#define DLLDIR  __declspec(dllimport)   // import DLL information

#endif

extern DLLDIR IPlugin* getPlugin();

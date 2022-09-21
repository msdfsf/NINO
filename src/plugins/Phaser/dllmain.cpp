#pragma once

#define DLLDIR_EX
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
) {

    return 1;

}
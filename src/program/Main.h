#pragma once

#define DEBUG 1

#define GET_HIGH_ORDER_WORD(param) ((short)(param >> (sizeof(int*) * 4)))
#define GET_LOW_ORDER_WORD(param) ((short)(param))

#define WHEEL_DELTA 120

#define M_PI_LONG          3.141592653589793238462643383279502884L

#define MAIN_SHADOW_COLOR 0xFF00FFFF
#define MAIN_HIGHLIGHT_COLOR 0xFFA57BF4
#define MAIN_FRONT_COLOR 0xFFFFFFFF
#define MAIN_BACK_COLOR 0xFF000000

#define ON_COLOR 0xFFA1DB2C
#define OFF_COLOR MAIN_FRONT_COLOR

#define PRESET_FOLDER "./Presets/"
#define PRESET_FOLDER_WCHAR L"./Presets/"
#define PRESET_EXTENSION "NINO"
#define PRESET_EXTENSION_WCHAR L"NINO"
#define MAX_FILE_NAME_LENGTH 256

// 0xFF30475E
#define _CRT_SECURE_NO_WARNINGS

#pragma once

#include <windows.h>
#include <vector>
#include <unordered_map>
#include <stdlib.h>

#include "IPlugin.h"

// global stuff
std::vector<IPlugin*> plugins;
std::unordered_multimap<std::string, IPlugin*> pluginsNameMap;
std::unordered_multimap<std::string, IPlugin*> pluginsFilenameMap;
std::unordered_map<IPlugin*, std::string> pluginToFilename;

#include "Main.h"

#include "Config.h"
#include "Resources.h"
#include "Render.h"

#include "Plugin.h"
#include "IPlugin.h"

#include "Preset.h"

#include "AudioDriver.h"

#include "TickEventsDriver.h"

#include "Utils.h"

#define MAX_LOADSTRING 100

#define IDI_TESTBC      203
#define IDI_SMALL       204

// Global Variables:
HINSTANCE hInst;
const WCHAR WINDOW_TITLE[] = L"[NINO] No Input, No Output";
const WCHAR WINDOW_CLASS_NAME[] = L"CLASS_NAME";



// My global stuff
const wchar_t PLUGINS_FOLDER[] = L"./Plugins/";

const int DRAG_DISTANCE_THRESHOLD = 10; // in pixels

int lastLeftMouseButtonDownCoords = 0x80008000;
int isLeftMouseButtonDown = 0;
int isAnythingDragged = 0;

// all supported devices by selected driver
int deviceCount = 0;
AudioDriver::Device** devices = NULL;

std::vector<Preset*> presets;

void scaleMouseCoords(POINT* coords);

int loadPlugins(wchar_t* dir, int dirLen);
void loadPluginsAndInit();

// Forward declarations of functions included in this code module:
HWND                InitInstance(HINSTANCE, HINSTANCE);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// bu
#include "UIPlayground.h"

//int initWindow();
//int initBasicControls();

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow
) {

    // loading config
    Config::load();

    // load resources
    Resources::load();

    // main control, known as window, initialization
    initWindow();
    initBasicControls();

    // select audio device, that was written in config
    AudioDriver::select(Config::audioDriver);

    // get audio devices supported by selected audio driver
    devices = AudioDriver::getDevices(&deviceCount);
    setDevices(devices, deviceCount);

    // init audio driver
    if (devices == NULL || deviceCount <= 0) {
        // cannot init driver, no supported device
        
        Utils::showError("Cannot init driver, no supported device!");
    
    } else {

        Config::setAudioDevice(Config::audioDeviceId >= deviceCount ? 0 : Config::audioDeviceId);
        AudioDriver::info.device = devices[Config::audioDeviceId];
        
        AudioDriver::info.channelIn = (Config::leftChannelIn ? AudioDriver::CHANNEL_1 : 0) |
            (Config::rightChannelIn ? AudioDriver::CHANNEL_2 : 0);

        AudioDriver::info.channelOut = (Config::leftChannelOut ? AudioDriver::CHANNEL_1 : 0) |
            (Config::rightChannelOut ? AudioDriver::CHANNEL_2 : 0);

        AudioDriver::info.sampleRate = Config::sampleRate;

        AudioDriver::init(&AudioDriver::info);

    }

    // set channels in gui
    setChannel(AudioDriver::CHANNEL_1, 1, Config::leftChannelIn);
    setChannel(AudioDriver::CHANNEL_1, 0, Config::leftChannelOut);
    setChannel(AudioDriver::CHANNEL_2, 1, Config::rightChannelIn);
    setChannel(AudioDriver::CHANNEL_2, 0, Config::rightChannelOut);

    // load external plugins
    loadPluginsAndInit();

    // load presets if any and add them to the ui
    Preset** presetsToAdd = Preset::load((wchar_t*) PRESET_FOLDER_WCHAR);
    for (int i = 0; presetsToAdd[i] != NULL; i++) {
        presets.push_back(presetsToAdd[i]);
    }
    setPresets(presets);

    // perform application initialization
    HWND hWnd = InitInstance(hInst, hInstance);
    if (hWnd == NULL) {
        return 0;
    }

    System::windowHandler = hWnd;

    // render initialization
    Render::init(hWnd, Config::renderWidth, Config::renderHeight);

    // display and paint window for the first time
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    setResizeType((ResizeType::ResizeType) Config::windowResize);


    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0) > 0) {

        TranslateMessage(&msg);
        DispatchMessage(&msg);
       
    }

    return (int) msg.wParam;

}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND InitInstance(
    HINSTANCE target,
    HINSTANCE hInstance
) {

    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEXW);

    wcex.style = CS_DBLCLKS;//CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TESTBC));
    wcex.hCursor = NULL;// LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = NULL;//(HBRUSH) (COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = WINDOW_CLASS_NAME;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    if (!RegisterClassExW(&wcex)) {

        // do something later
        return NULL;

    }

    target = hInstance;

    DWORD styleFlags = Config::windowResize ? WS_OVERLAPPEDWINDOW : WS_OVERLAPPEDWINDOW ^ WS_SIZEBOX;
    RECT winRect = { 0, 0, Config::windowWidth, Config::windowHeight };

    if (AdjustWindowRectEx(&winRect, styleFlags, 0, WS_EX_CLIENTEDGE) == 0) {

        // do something later
        return NULL;

    }

    int winHeight = winRect.bottom - winRect.top;
    int winWidth = winRect.right - winRect.left;

    HWND hWnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        WINDOW_CLASS_NAME,
        WINDOW_TITLE,
        styleFlags,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        winWidth,
        winHeight,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    HANDLE icon = LoadImage(0, L"./icon.ico", IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
    if (icon) {
        SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM) icon);
        SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM) icon);

        SendMessage(GetWindow(hWnd, GW_OWNER), WM_SETICON, ICON_SMALL, (LPARAM) icon);
        SendMessage(GetWindow(hWnd, GW_OWNER), WM_SETICON, ICON_BIG, (LPARAM) icon);
    }

    return hWnd;

}


#define SCALE_MOUSE_COORDS \
if (window->resizeType == ResizeType::SCALE) {\
    POINT mouseCoords = {\
        GET_LOW_ORDER_WORD(lParam),\
        GET_HIGH_ORDER_WORD(lParam)\
    };\
    scaleMouseCoords(&mouseCoords);\
    lParam = SET_WORD(mouseCoords.y, mouseCoords.x);\
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(
    HWND hwnd, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam
) {

    switch (message) {

        case WM_COMMAND: {
            
            break;

        }

        case WM_PAINT: {

            if (IsIconic(hwnd)) break;

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            window->draw();

            const ResizeType::ResizeType resizeType = window->resizeType;
            if (resizeType == ResizeType::NONE) {
                Render::render(hdc);
            } else if (resizeType == ResizeType::SCALE) {
                Render::render(hdc, Config::windowWidth, Config::windowHeight);
            }

            EndPaint(hwnd, &ps);

            break;

        }

        case WM_SIZE: {
            
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);

            Config::windowWidth = width;
            Config::windowHeight = height;

            RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);

            break;
        
        }

        case WM_LBUTTONDOWN: {

            lastLeftMouseButtonDownCoords = lParam;
            isLeftMouseButtonDown = 1;
            
            SetCapture(hwnd);

            SCALE_MOUSE_COORDS;
            window->processMessage(ControlEvent::MOUSE_DOWN, lParam, NULL);

            break;

        }

        case WM_LBUTTONUP: {

            if (isLeftMouseButtonDown) {
                // click

                SCALE_MOUSE_COORDS;
                window->processMessage(ControlEvent::MOUSE_CLICK, lParam, NULL);

            }

            if (isAnythingDragged) {

                SCALE_MOUSE_COORDS;
                window->processMessage(ControlEvent::DRAG_END, lParam, wParam);
                window->processMessage(ControlEvent::DROP, lParam, wParam);
                isAnythingDragged = 0;
            
            }
            
            isLeftMouseButtonDown = 0;

            SCALE_MOUSE_COORDS;
            window->processMessage(ControlEvent::MOUSE_UP, lParam, NULL);

            ReleaseCapture();

            break;

        }

        case WM_MOUSEWHEEL: {

            SCALE_MOUSE_COORDS;
            window->processMessage(ControlEvent::MOUSE_SCROLL, lParam, GET_HIGH_ORDER_WORD(wParam));

            break;

        }

        case WM_MOUSEMOVE: {

            SCALE_MOUSE_COORDS;

            if (isAnythingDragged) {

                //SCALE_MOUSE_COORDS;
                window->processMessage(ControlEvent::DRAG, lParam, wParam);

            } else {

                if (wParam == MK_LBUTTON) {
                    
                    const int oldX = GET_LOW_ORDER_WORD(lParam);
                    const int oldY = GET_HIGH_ORDER_WORD(lParam);

                    const int newX = GET_LOW_ORDER_WORD(lastLeftMouseButtonDownCoords);
                    const int newY = GET_HIGH_ORDER_WORD(lastLeftMouseButtonDownCoords);
                    
                    if (
                        abs(oldX - newX) > DRAG_DISTANCE_THRESHOLD ||
                        abs(oldY - newY) > DRAG_DISTANCE_THRESHOLD
                    ) {

                        isAnythingDragged = 1;
                        //SCALE_MOUSE_COORDS;
                        window->processMessage(ControlEvent::DRAG_START, lParam, wParam);
                        
                        //break;
                    
                    }
                
                }

            }

            //SCALE_MOUSE_COORDS;
            window->processMessage(ControlEvent::MOUSE_MOVE, lParam, wParam);

            break;

        }

        case WM_LBUTTONDBLCLK: {
            
            SCALE_MOUSE_COORDS;
            window->processMessage(ControlEvent::MOUSE_DBL_CLICK, lParam, wParam);
            
            break;
        
        }

        case WM_CHAR: {

            //if (GetKeyState(VK_BACK) & 0x8000) return;
            window->processMessage(ControlEvent::CHAR_INPUT, wParam, lParam);
            
            break;

        }

        case WM_DESTROY: {
            
            TickEventsDriver::terminate();
            AudioDriver::exit();
            PostQuitMessage(0);
            break;
        
        }

        default: {

            return DefWindowProc(hwnd, message, wParam, lParam);

        }

    }
    
    return 0;

}

void scaleMouseCoords(POINT* coords) {

    if (Render::renderHeight * Config::windowWidth > Render::renderWidth * Config::windowHeight) {

        int wd = (Render::renderWidth * Config::windowHeight) / Render::renderHeight;
        int x = (Config::windowWidth - wd) / 2;
        coords->x = ((Render::renderWidth * (coords->x - x)) / wd);
        coords->y = (coords->y * Render::renderHeight) / Config::windowHeight;

    }
    else {

        int hg = (Render::renderHeight * Config::windowWidth) / Render::renderWidth;
        int y = (Config::windowHeight - hg) / 2;
        coords->y = y + ((Render::renderHeight * (coords->y - y)) / hg);
        coords->x = (coords->x * Render::renderWidth) / Config::windowWidth;

    }

}

int loadPlugins(wchar_t* dir, int dirLen) {

    int err = 0;

    HANDLE hFind;
    WIN32_FIND_DATA findFileData;

    wchar_t flFilter[] = L"*.dll";
    wchar_t* flName = (wchar_t*) malloc(sizeof(wchar_t) * dirLen + sizeof(flFilter));
    if (flName == NULL) {
        err = 1;
        goto cleanup;
    }
    flName[0] = L'\0';

    wcscat(flName, dir);
    wcscat(flName, flFilter);

    if ((hFind = FindFirstFile(flName, &findFileData)) != INVALID_HANDLE_VALUE) {

        do {

            // path to dll constructor or whatever
            wchar_t* dllPath = (wchar_t*) malloc(sizeof(wchar_t) * (wcslen(findFileData.cFileName) + dirLen + 1));
            if (dllPath == NULL) {
                err = 1;
                goto cleanup;
            }
            dllPath[0] = L'\0';

            wcscat(dllPath, dir);
            wcscat(dllPath, findFileData.cFileName);

            // trying to load this dll
            HINSTANCE dllInstance = LoadLibrary(dllPath);

            if (!dllInstance) {
                // could not load the dynamic library
                err = 3;
                goto cleanup;
            }

            FARPROC fceAddr = GetProcAddress(dllInstance, "getPlugin");
            if (!fceAddr) {
                // could not locate the function
                err = 4;
                goto cleanup;
            }

            IPlugin* plugin = (IPlugin*) fceAddr();
            plugin->state = 1;
            plugins.push_back(plugin);

            // need to convert unicode to utf8 before std::string conversion
            char utf8Buffer[256 * 3];
            char* tmp = utf8Buffer;
            int utf8BufferLen = 0;
            
            utf8BufferLen = Utils::wc2utf8((wchar_t*)plugin->name, wcslen(plugin->name), &tmp);
            utf8Buffer[utf8BufferLen] = '\0';
            pluginsNameMap.insert({ utf8Buffer, plugin });

            utf8BufferLen = Utils::wc2utf8(findFileData.cFileName, wcslen(findFileData.cFileName), &tmp);
            utf8Buffer[utf8BufferLen] = '\0';
            pluginsFilenameMap.insert({ utf8Buffer, plugin });

            pluginToFilename.insert({ plugin, utf8Buffer });

            free(dllPath);

        } while (FindNextFile(hFind, &findFileData));

        FindClose(hFind);

    } else {

        err = 2;

    }

    cleanup:
    
    free(flName);
    return err;

}

void loadPluginsAndInit() {

    // load plugins from dll files to the global variable std::vector plugins
    const int err = loadPlugins((wchar_t*)PLUGINS_FOLDER, sizeof(PLUGINS_FOLDER) / sizeof(wchar_t));
    if (err) {
        // use enum later
        switch (err) {
        case 1: Utils::showError("Malloc did not feel well today..."); return;
        case 2: return;
        case 3: Utils::showError("Could not load DLL..."); return;
        case 4: Utils::showError("Could not locate the function..."); return;
        default: return;
        }
    }

    // add plugins to the plugin list
    const int pluginCount = plugins.size();
    char** items = (char**) malloc(sizeof(char*) * pluginCount);
    for (int i = 0; i < pluginCount; i++) {

        const int strLen = wcslen(plugins[i]->name);

        char* item = (char*) malloc((strLen + 1) * sizeof(char)); // strLen + 1 maybe, to fit \0
        if (item == NULL) {
            // handle it better on refactoring

            free(item);
            for (int j = i - 1; i >= 0; j--) {
                free(items[i]);
            }
            free(items);

            return;

        }

        wcstombs(item, plugins[i]->name, strLen);
        item[strLen] = '\0';

        items[i] = item;

    }

    pluginSelectMenu->addItems(items, pluginCount);
    pluginSelectMenu->setItemHeight();

    // dont think free is what we want
    //free(items);

}

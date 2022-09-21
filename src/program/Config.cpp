#pragma once

#include "Config.h"
#include <fstream>

#define strdup _strdup

namespace Config {
    
    //
    // default properties values
    //
    const int PROPERTIES_COUNT = 3;

    // client sizes
    const int DF_WINDOW_WIDTH   = 640;
    const int DF_WINDOW_HEIGHT  = 800;

    const int DF_RENDER_WIDTH   = DF_WINDOW_WIDTH;
    const int DF_RENDER_HEIGHT  = DF_WINDOW_HEIGHT;

    const int DF_WINDOW_RESIZE      = 0;
    const int DF_WINDOW_MAXIMIZE    = 0;
    const int DF_FULL_SCREEN        = 0;

    // audio settings
    const AudioDriver::Driver DF_AUDIO_DRIVER = AudioDriver::AD_ASIO;
    const int DF_AUDIO_DEVICE_ID    = 0;
    const int DF_LEFT_CHANNEL_IN    = 1;
    const int DF_RIGHT_CHANNEL_IN   = 0;
    const int DF_LEFT_CHANNEL_OUT   = 1;
    const int DF_RIGHT_CHANNEL_OUT  = 1;
    const int DF_SAMPLE_RATE        = 44100;

    // other
    #define DF_OUT_FILE_NAME "OutFile"
    #define DF_IN_FILE_NAME "InFile"

    //
    // actual properties values, are set to default as default KEKW
    // 
    
    int windowWidth     = DF_WINDOW_WIDTH;
    int windowHeight    = DF_WINDOW_HEIGHT;

    int renderWidth     = DF_RENDER_WIDTH;
    int renderHeight    = DF_RENDER_HEIGHT;

    int windowResize    = DF_WINDOW_RESIZE;
    int windowMaximize  = DF_WINDOW_MAXIMIZE;
    int fullScreen      = DF_FULL_SCREEN;

    AudioDriver::Driver audioDriver = DF_AUDIO_DRIVER;
    int audioDeviceId   = DF_AUDIO_DEVICE_ID;
    int leftChannelIn   = DF_LEFT_CHANNEL_IN;
    int rightChannelIn  = DF_RIGHT_CHANNEL_IN;
    int leftChannelOut  = DF_LEFT_CHANNEL_OUT;
    int rightChannelOut = DF_RIGHT_CHANNEL_OUT;
    int sampleRate      = DF_SAMPLE_RATE;

    char* outFileName   = (char*) DF_OUT_FILE_NAME;
    char* inFileName    = (char*) DF_IN_FILE_NAME;

    // so, each propertie is pre-hashed and written into following enum
    // this solution requaries no collision, so names has to be re-chossen
    // if something like this occures
    enum Properties {

        WINDOW_WIDTH        = 688673884,
        WINDOW_HEIGHT       = 659792757,

        RENDER_WIDTH        = 1667443684,
        RENDER_HEIGHT       = 2894425085,

        WINDOW_RESIZE       = 1051508814,
        WINDOW_MAXIMIZE     = 3612261312,
        FULL_SCREEN         = 1540445015,
        

        AUDIO_DRIVER        = 643383298,
        AUDIO_DEVICE        = 628419270,
        LEFT_CHANNEL_IN     = 2856149054,
        RIGHT_CHANNEL_IN    = 1777780753,
        LEFT_CHANNEL_OUT    = 4058612447,
        RIGHT_CHANNEL_OUT   = 2832196882,
        SAMPLE_RATE         = 3187368306,

        OUT_FILE_NAME       = 3775738460,
        IN_FILE_NAME        = 4142588603,

    };

    // djb2 -> http://www.cse.yorku.ca/~oz/hash.html
    int hash(std::string& str) {

        uint32_t hash = 5381;

        for (int i = 0; i < str.size(); i++) {
            int ch = str[i];
            hash = ((hash << 5) + hash) + ch;
        }

        return hash;

    }

    // loads config from file
    // returns 1 if file could not be opened
    int load() {

        std::ifstream inFile(fileName, std::ifstream::in);
        if (!inFile.is_open()) return 1;

        std::string line;
        while (std::getline(inFile, line)) {

            int st = 0;
            for (int i = 0; i < line.size(); i++) {
                if (!isspace(line.at(i))) {
                    st = i;
                    break;
                }
            }

            int ls = st;
            for (int i = st + 1; i < line.size(); i++) {
                if (isspace(line.at(i))) {
                    ls = i - 1;
                    break;
                }
            }

            std::string propertieName = line.substr(st, ls - st + 1);

            st = ls + 1;
            for (int i = st; i < line.size(); i++) {
                if (!isspace(line.at(i))) {
                    st = i;
                    break;
                }
            }

            ls = st - 1;
            for (int i = line.size() - 1; i >= st; i--) {
                if (!isspace(line.at(i))) {
                    ls = i;
                    break;
                }
            }

            if (ls < st) {
                // seems no param value included
                continue;
            }

            std::string propertieValue = line.substr(st, ls - st + 1);

            const unsigned int strHash = (int) hash(propertieName);

            switch (strHash) {

                case WINDOW_WIDTH:
                    
                    setWindowWidth(std::stoi(propertieValue));
                    break;

                case WINDOW_HEIGHT:

                    setWindowHeight(std::stoi(propertieValue));
                    break;

                case RENDER_WIDTH:

                    setRenderWidth(std::stoi(propertieValue));
                    break;

                case RENDER_HEIGHT:

                    setRenderHeight(std::stoi(propertieValue));
                    break;

                case WINDOW_RESIZE:

                    setWindowResize(std::stoi(propertieValue));
                    break;

                case WINDOW_MAXIMIZE:

                    setWindowMaximize(std::stoi(propertieValue));
                    break;

                case FULL_SCREEN:

                    setWindowFullScreen(std::stoi(propertieValue));
                    break;

                case AUDIO_DRIVER:

                    setAudioDriver((AudioDriver::Driver) std::stoi(propertieValue));
                    break;

                case AUDIO_DEVICE:

                    setAudioDevice(std::stoi(propertieValue));
                    break;

                case LEFT_CHANNEL_IN:

                    setLeftChannelIn(std::stoi(propertieValue));
                    break;

                case RIGHT_CHANNEL_IN:

                    setRightChannelIn(std::stoi(propertieValue));
                    break;

                case LEFT_CHANNEL_OUT:

                    setLeftChannelOut(std::stoi(propertieValue));
                    break;

                case RIGHT_CHANNEL_OUT:

                    setRightChannelOut(std::stoi(propertieValue));
                    break;

                case SAMPLE_RATE:

                    setSampleRate(std::stoi(propertieValue));
                    break;

                case OUT_FILE_NAME:

                    setOutFileName(propertieValue);
                    break;

                case IN_FILE_NAME:

                    setInFileName(propertieValue);
                    break;

            }

        }

        inFile.close();

        return 0;

    }

    void setWindowWidth(int width) {

        windowWidth = (width >= 0) ? width : 0;

    }

    void setWindowHeight(int height) {

        windowHeight = (height >= 0) ? height : 0;

    }

    void setRenderWidth(int width) {

        renderWidth = (width >= 0) ? width : 0;

    }

    void setRenderHeight(int height) {

        renderHeight = (height >= 0) ? height : 0;

    }

    void setWindowResize(int resize) {

        if (resize > 1) windowResize = 1;
        else if (resize < 0) windowResize = 0;
        else windowResize = resize;

    }

    void setWindowMaximize(int maximize) {

        if (maximize > 1) windowMaximize = 1;
        else if (maximize < 0) windowMaximize = 0;
        else windowMaximize = maximize;

    }

    void setWindowFullScreen(int full) {

        if (full > 1) fullScreen = 1;
        else if (full < 0) fullScreen = 0;
        else fullScreen = fullScreen;

    }

    void setAudioDriver(AudioDriver::Driver driver) {

        if (driver >= AudioDriver::AD_COUNT || driver < 0) {
            audioDriver = DF_AUDIO_DRIVER;
        } else {
            audioDriver = driver;
        }

    }

    void setAudioDevice(int deviceId) {

        audioDeviceId = (deviceId < 0) ? DF_AUDIO_DEVICE_ID : deviceId;

    }

    void setLeftChannelIn(int on) {

        leftChannelIn = on == 1;

    }

    void setRightChannelIn(int on) {

        rightChannelIn = on == 1;

    }

    void setLeftChannelOut(int on) {

        leftChannelOut = on == 1;

    }

    void setRightChannelOut(int on) {

        rightChannelOut = on == 1;

    }


    void setSampleRate(int smplrt) {

        sampleRate = (smplrt >= 0) ? smplrt : 0;

    }

    void setOutFileName(std::string name) {

        outFileName = (char*) strdup(name.c_str());

    }

    void setInFileName(std::string name) {

        inFileName = (char*) strdup(name.c_str());

    }

}

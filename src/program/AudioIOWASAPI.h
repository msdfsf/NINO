#pragma once

#include "IAudioIO.h"
#include <windows.h>

#include <avrt.h>
#include <thread>
#include <chrono>
#include <Audioclient.h>
#include <Audiopolicy.h>
#include <mmdeviceapi.h>

#pragma comment(lib, "avrt")

class AudioIOWASAPI : IAudioIO {
	// for now there is a lot of assuming, dunno how all formats has to look like, etc.
	// so, for example, in and out has to have same format

	public:

		// dunno where to find all supported formats, so there are some samples for now
		enum SampleFormat {

			NONE,
			INT_8,
			INT_16,
			INT_24,
			INT_32,
			FLOAT_32,
			FLOAT_64

		};

		AudioIOWASAPI();

		virtual int init(AudioDriver::DriverInfo* info);

		virtual int start();
		virtual int stop();
		virtual int exit();

		virtual void setChannels(AudioDriver::DriverInfo* info);
		virtual void openExternalConfig();

		virtual AudioDriver::Device** getDevices(int* deviceCount);

		void toggleMode();

		virtual ~AudioIOWASAPI();

	private:

		int initialized = 0;

		std::thread workingThread;

		int run = 0;

		int sharedMode = 1;

		AudioDriver::DriverInfo* driverInfo = NULL;

		int sampleRate = 48000;
		int bitResolution = 24;

		int leftChannelIn = 0;
		int leftChannelOut = 0;
		int rightChannelIn = 0;
		int rightChannelOut = 0;

		double* dataBuffers = NULL;
		int dataBufferInCount = 0;
		int dataBufferOutCount = 0;
		int dataBufferLength = 0;
		int dataBufferIndex = 0;

		IAudioClient* renderClient;
		IAudioClient* captureClient;

		IAudioRenderClient* renderService;
		IAudioCaptureClient* captureService;

		WAVEFORMATEX* renderFormat;
		WAVEFORMATEX* captureFormat;

		HANDLE renderEvent;
		HANDLE captureEvent;

		uint32_t cBufferFrameCount;
		uint32_t rBufferFrameCount;

		BYTE* outData;
		BYTE* inData;

		REFERENCE_TIME captureBufferDuration;

		HRESULT error;

		int allocDataBuffers(int inCount, int outCount, int length);
		void freeDataBuffers();

		HRESULT initRender(
			IAudioClient** audioClient,
			IAudioRenderClient** renderClient,
			uint32_t* bufferFrameCount,
			WAVEFORMATEX** waveFormat,
			HANDLE* renderEvent,
			LPCGUID sessionId,
			int sharedMode
		);

		HRESULT initCapture(
			IAudioClient** audioClient,
			IAudioCaptureClient** captureClient,
			uint32_t* bufferFrameCount,
			WAVEFORMATEX** waveFormat,
			HANDLE* captureEvent,
			LPCGUID sessionId,
			int sharedMode
		);

		int getStreamFormat(
			WAVEFORMATEXTENSIBLE** waveFormatEx,
			unsigned int sampleRate,
			unsigned int bitResolution,
			int channelCount
		);

		AudioIOWASAPI::SampleFormat getSampleFormat(WAVEFORMATEXTENSIBLE* format);

		void clean();

		void handleIO();

};

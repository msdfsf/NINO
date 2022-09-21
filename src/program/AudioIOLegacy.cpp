#pragma once

#include "AudioIOLegacy.h"
#include "SampleFormat.h"

#include <limits.h>
#include <stdint.h>
#include <Audioclient.h>
//#include <Audiopolicy.h>
#include <mmdeviceapi.h>
#include <windows.h>
//#include <mmeapi.h>
#include <mmsystem.h>
#include <cmath>
#pragma comment(lib, "winmm")

#include "ErrorCodes.h"

#define SAFE_RELEASE(punk) if ((punk) != NULL) { (punk)->Release(); (punk) = NULL; }
#define FREE(mem) { free((mem)); mem = NULL; }
#define CLEAN_RETURN(err) { clean(this); return (err); }

// in header?
#define BASIC_CLIP(in, out, max){\
	const double val = (in);\
	if (val >= 1) { out = (max); }\
	else if (val <= -1) { out = -(max); }\
	else { out = val * (max); }\
}

AudioIOLegacy::AudioIOLegacy() {

	waveFormat = NULL;

	waveIn = NULL;
	waveOut = NULL;

	waveHeaders = NULL;
	waveBuffers = NULL;

	buffer = NULL;

	bufferLen = 0;
	bufferCount = 0;
	bufferIdx = 0;

	leftChannelIn = 0;
	rightChannelIn = 0;
	leftChannelOut = 0;
	rightChannelOut = 0;

	reset = 0;

	sampleFormat = SampleFormat::NONE;

}

void getDefaultWaveFormat(WAVEFORMATEX** waveFormat) {

	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	const IID IID_IAudioClient = __uuidof(IAudioClient);

	HRESULT error;
	IAudioClient* audioClient = NULL;
	IMMDeviceEnumerator* deviceEnumerator = NULL;
	IMMDevice* device = NULL;

	error = CoCreateInstance(
		CLSID_MMDeviceEnumerator,
		NULL,
		CLSCTX_ALL,
		IID_IMMDeviceEnumerator,
		(void**)&deviceEnumerator
	);
	if (FAILED(error)) goto cleanupAndExit;

	error = deviceEnumerator->GetDefaultAudioEndpoint(
		eRender,
		eMultimedia, // dunno whats that
		&device
	);
	if (FAILED(error)) goto cleanupAndExit;

	error = device->Activate(
		IID_IAudioClient,
		CLSCTX_ALL, // dunno how CLSCTX param works, so CLSCTX_ALL for now
		NULL,
		(void**)&audioClient
	);
	if (FAILED(error)) goto cleanupAndExit;

	error = audioClient->GetMixFormat((WAVEFORMATEX**)waveFormat);
	if (FAILED(error)) {
		CoTaskMemFree(waveFormat);
		*waveFormat = NULL;
	}

cleanupAndExit:
	SAFE_RELEASE(deviceEnumerator)
		SAFE_RELEASE(device)
		SAFE_RELEASE(audioClient)

}

void clean(AudioIOLegacy* const driver) {

	for (int i = 0; i < driver->bufferCount; i++) {
		if (driver->waveBuffers[i] != NULL) FREE(driver->waveBuffers[i]);
		if (driver->waveHeaders[i] != NULL) FREE(driver->waveHeaders[i]);
	}
	if (driver->waveBuffers != NULL) FREE(driver->waveBuffers);
	if (driver->waveHeaders != NULL) FREE(driver->waveHeaders);

	if (driver->buffer != NULL) FREE(driver->buffer);

	if (driver->waveFormat != NULL) {
		CoTaskMemFree(driver->waveFormat);
		driver->waveFormat = NULL;
	}

	if (driver->waveIn != NULL) FREE(driver->waveIn);
	if (driver->waveOut != NULL) FREE(driver->waveOut);

}

int resolveSampleFormat(WAVEFORMATEX* format) {

	if (format->wFormatTag != WAVE_FORMAT_EXTENSIBLE) {
		// assuming PCM format

		if (format->wFormatTag == WAVE_FORMAT_PCM) {
			// assuming int
			// only 8 or 16 bits

			if (format->wBitsPerSample == 8) {
				return SampleFormat::INT_8;
			}
			else {
				return SampleFormat::INT_16;
			}

		}
		else if (format->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
			// assuming float

			return SampleFormat::FLOAT_32;

		}
		else {
			// dunno

			return SampleFormat::NONE;

		}

	}
	else {
		// could be almost anything, huh?

		if (((WAVEFORMATEXTENSIBLE*)format)->SubFormat == KSDATAFORMAT_SUBTYPE_PCM) {
			// assuming int

			if (format->wBitsPerSample == 8) {
				return SampleFormat::INT_8;
			}
			else if (format->wBitsPerSample == 16) {
				return SampleFormat::INT_16;
			}
			else if ((format->wBitsPerSample == 32)) {
				return SampleFormat::INT_32;
			}
			else {
				return SampleFormat::NONE;
			}

		}
		else if (((WAVEFORMATEXTENSIBLE*)format)->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
			// assuming float		

			if (format->wBitsPerSample == 32) {
				return SampleFormat::FLOAT_32;
			}
			else if (format->wBitsPerSample == 64) {
				return SampleFormat::FLOAT_64;
			}
			else {
				return SampleFormat::NONE;
			}

		}
		else {
			// dunno

			return SampleFormat::NONE;

		}

	}

}

template <typename InData, typename OutData>
void fProcess(
	AudioIOLegacy* driver,
	InData inData,
	OutData outData,
	double* const inBuff,
	double* const outBuff,
	const int buffLen
) {

	const double OUT_MAX = 1.0;

	const int inLeft = driver->leftChannelIn;
	const int inRight = driver->rightChannelIn;
	const int outLeft = driver->leftChannelOut;
	const int outRight = driver->rightChannelOut;

	if (inLeft && inRight) {

		for (int i = 0; i < buffLen; i++) {
			const int off = 2 * i;
			inBuff[i] = (double)(inData[off]) + (double)(inData[off + 1]);
		}

	}
	else if (inLeft) {

		for (int i = 0; i < buffLen; i++) {
			inBuff[i] = inData[2 * i];
		}

	}
	else if (inRight) {

		for (int i = 0; i < buffLen; i++) {
			inBuff[i] = inData[2 * i + 1];
		}

	}
	else {

		for (int i = 0; i < buffLen; i++) {
			inBuff[i] = 0;
		}

	}

	driver->processInput((void*)inBuff, (void*)outBuff, buffLen);

	if (outLeft && outRight) {

		for (int i = 0; i < buffLen; i++) {
			const int off = 2 * i;
			BASIC_CLIP(outBuff[i], outData[off], OUT_MAX);
			BASIC_CLIP(outBuff[i], outData[off + 1], OUT_MAX);
		}

	}
	else if (outLeft) {

		for (int i = 0; i < buffLen; i++) {
			const int off = 2 * i;
			BASIC_CLIP(outBuff[i], outData[off], OUT_MAX);
			outData[off + 1] = 0;
		}

	}
	else if (outRight) {

		for (int i = 0; i < buffLen; i++) {
			const int off = 2 * i;
			outData[off] = 0;
			BASIC_CLIP(outBuff[i], outData[off + 1], OUT_MAX);
		}

	}
	else {
		memset(outData, 0, 2 * buffLen * sizeof(*outData));
	}

}

template <typename InData, typename OutData>
void iProcess(
	AudioIOLegacy* driver,
	InData inData,
	OutData outData,
	double* const inBuff,
	double* const outBuff,
	const int buffLen
) {

	// will it work allways? is it pre compiled or something?
	const unsigned int inDataSize = sizeof(*inData) * 8;
	const unsigned int outDataSize = sizeof(*outData) * 8;

	const long int IN_MAX = (1 << (inDataSize - 1)) - 1;
	const long int OUT_MAX = (1 << (outDataSize - 1)) - 1;

	const int inLeft = driver->leftChannelIn;
	const int inRight = driver->rightChannelIn;
	const int outLeft = driver->leftChannelOut;
	const int outRight = driver->rightChannelOut;

	if (inLeft && inRight) {

		for (int i = 0; i < buffLen; i++) {
			const int off = 2 * i;
			inBuff[i] = (double)(inData[off]) + (double)(inData[off + 1]) / (double)IN_MAX;
		}

	}
	else if (inLeft) {

		for (int i = 0; i < buffLen; i++) {
			inBuff[i] = inData[2 * i] / (double)IN_MAX;
		}

	}
	else if (inRight) {

		for (int i = 0; i < buffLen; i++) {
			inBuff[i] = inData[2 * i + 1] / (double)IN_MAX;
		}

	}
	else {

		for (int i = 0; i < buffLen; i++) {
			inBuff[i] = 0;
		}

	}

	driver->processInput((void*)inBuff, (void*)outBuff, buffLen);

	if (outLeft && outRight) {

		for (int i = 0; i < buffLen; i++) {
			const int off = 2 * i;
			BASIC_CLIP(outBuff[i], outData[off], OUT_MAX);
			BASIC_CLIP(outBuff[i], outData[off + 1], OUT_MAX);
		}

	}
	else if (outLeft) {

		for (int i = 0; i < buffLen; i++) {
			const int off = 2 * i;
			BASIC_CLIP(outBuff[i], outData[off], OUT_MAX);
			outData[off + 1] = 0;
		}

	}
	else if (outRight) {

		for (int i = 0; i < buffLen; i++) {
			const int off = 2 * i;
			outData[off] = 0;
			BASIC_CLIP(outBuff[i], outData[off + 1], OUT_MAX);
		}

	}
	else {
		memset(outData, 0, 2 * buffLen * sizeof(*outData));
	}

}

void processCallback(HWAVEIN hwi, UINT msg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) {

	switch (msg) {

	case WIM_OPEN: {

		break;

	}

	case WIM_DATA: {

		AudioIOLegacy* const driver = (AudioIOLegacy*)dwInstance;

		const int frameSize = driver->frameSize;
		const int framesInBuffer = driver->framesInBuffer;

		int buffIdx = driver->bufferIdx;
		const int buffLen = driver->bufferLen;
		const int buffCnt = driver->bufferCount;

		BYTE** const buffers = driver->waveBuffers;
		WAVEHDR** const headers = driver->waveHeaders;

		double* const internalBuffer = driver->buffer;

		const HWAVEOUT waveOut = *(driver->waveOut);



		double* const inBuff = internalBuffer;
		double* const outBuff = internalBuffer + framesInBuffer;

		switch (driver->sampleFormat) {

		case SampleFormat::INT_8: {

			iProcess(
				driver,
				(int8_t*)buffers[buffIdx],
				(int8_t*)buffers[buffIdx + 2],
				inBuff,
				outBuff,
				(buffLen / frameSize) / 2 // or maybe better exchange for framesInBuffer
			);

			break;

		}

		case SampleFormat::INT_16: {

			iProcess(
				driver,
				(int16_t*)buffers[buffIdx],
				(int16_t*)buffers[buffIdx + 2],
				inBuff,
				outBuff,
				(buffLen / frameSize) / 2
			);

			break;

		}

		case SampleFormat::INT_32: {

			iProcess(
				driver,
				(int32_t*)buffers[buffIdx],
				(int32_t*)buffers[buffIdx + 2],
				inBuff,
				outBuff,
				(buffLen / frameSize) / 2
			);

			break;

		}

		case SampleFormat::FLOAT_32: {

			fProcess(
				driver,
				(float*)buffers[buffIdx],
				(float*)buffers[buffIdx + 2],
				inBuff,
				outBuff,
				(buffLen / frameSize) / 2 // or maybe better exchange for framesInBuffer
			);

			break;

		}

		case SampleFormat::FLOAT_64: {

			fProcess(
				driver,
				(double*)buffers[buffIdx],
				(double*)buffers[buffIdx + 2],
				inBuff,
				outBuff,
				(buffLen / frameSize) / 2 // or maybe better exchange for framesInBuffer
			);

			break;

		}

		}



		// manage driver buffers
		//
		headers[buffIdx + 2]->dwFlags = 0;
		waveOutPrepareHeader(waveOut, (LPWAVEHDR)headers[buffIdx + 2], sizeof(WAVEHDR));
		waveOutWrite(waveOut, (LPWAVEHDR)headers[buffIdx + 2], sizeof(WAVEHDR));

		headers[buffIdx]->dwFlags = 0;
		headers[buffIdx]->dwBytesRecorded = 0;
		if (driver->reset == 0) {
			waveInPrepareHeader(hwi, (LPWAVEHDR)headers[buffIdx], sizeof(WAVEHDR));
			waveInAddBuffer(hwi, (LPWAVEHDR)headers[buffIdx], sizeof(WAVEHDR));
		}


		// update idx
		//
		if (buffIdx + 1 >= buffCnt - 2) buffIdx = 0;
		else buffIdx++;

		driver->bufferIdx = buffIdx;

		break;

	}

	case WIM_CLOSE: {

		break;

	}

	}

}

int AudioIOLegacy::init(AudioDriver::DriverInfo* info) {

	leftChannelIn = info->channelIn & AudioDriver::CHANNEL_1;
	rightChannelIn = (info->channelIn & AudioDriver::CHANNEL_2) >> 1;
	leftChannelOut = info->channelOut & AudioDriver::CHANNEL_1;
	rightChannelOut = (info->channelOut & AudioDriver::CHANNEL_2) >> 1;

	getDefaultWaveFormat(&waveFormat);
	if (!waveFormat) return ERR_AD_INVALID_FORMAT;

	sampleFormat = (SampleFormat::SampleFormat)resolveSampleFormat(waveFormat);

	MMRESULT error;

	const int frameSize = waveFormat->wBitsPerSample / 8; // in bytes
	const int bytesPerSecond = waveFormat->nSamplesPerSec * frameSize;
	const int bufferLen = (bytesPerSecond * waveFormat->nChannels) * 0.02f;
	const int bufferCount = 4;

	info->maxBufferLength = (int)ceil(bufferLen / (double)frameSize);

	waveBuffers = (BYTE**)malloc(bufferCount * (sizeof(BYTE*)));
	if (!waveBuffers) return ERR_ALLOC;

	waveHeaders = (WAVEHDR**)malloc(bufferCount * (sizeof(WAVEHDR*)));
	if (!waveHeaders) return ERR_ALLOC;

	// whats that, is it any needed? if so do we realy need bufferCount, and not bufferCount / 2?
	buffer = (double*)malloc(bufferCount * info->maxBufferLength * (sizeof(double)));
	if (!buffer) return 1;

	for (int i = 0; i < bufferCount; i++) {

		waveBuffers[i] = (BYTE*)malloc(bufferLen);
		if (!waveBuffers[i]) {

			for (int j = 0; j < i; j++) {
				free(waveBuffers[j]);
			}

			FREE(waveBuffers);

			return 1;

		}

		waveHeaders[i] = (WAVEHDR*)malloc(sizeof(WAVEHDR));
		if (!waveHeaders[i]) {

			for (int j = 0; j < i; j++) {
				free(waveHeaders[j]);
			}

			FREE(waveHeaders);

			return 1;

		}

	}

	waveOut = (HWAVEOUT*)malloc(sizeof(HWAVEOUT));
	if (!waveOut) CLEAN_RETURN(ERR_ALLOC);

	error = waveOutOpen(waveOut, WAVE_MAPPER, waveFormat, NULL, NULL, CALLBACK_FUNCTION);
	if (error != MMSYSERR_NOERROR) CLEAN_RETURN(ERR_AD_INVALID_OUTPUT_DEVICE);

	waveIn = (HWAVEIN*)malloc(sizeof(HWAVEIN));
	if (!waveIn) CLEAN_RETURN(ERR_ALLOC);

	error = waveInOpen(waveIn, WAVE_MAPPER, waveFormat, (DWORD_PTR)&processCallback, (DWORD)this, CALLBACK_FUNCTION);
	if (error != MMSYSERR_NOERROR) CLEAN_RETURN(ERR_AD_INVALID_INPUT_DEVICE);

	int i = 0;
	for (; i < bufferCount / 2; i++) {

		waveHeaders[i]->lpData = (LPSTR)waveBuffers[i];
		waveHeaders[i]->dwBufferLength = bufferLen;
		waveHeaders[i]->dwFlags = 0;
		waveHeaders[i]->dwLoops = 0L;

		error = waveInPrepareHeader(*waveIn, waveHeaders[i], sizeof(WAVEHDR));
		if (error != MMSYSERR_NOERROR) CLEAN_RETURN(ERR_ALLOC);

		error = waveInAddBuffer(*waveIn, (LPWAVEHDR)waveHeaders[i], sizeof(WAVEHDR));
		if (error != MMSYSERR_NOERROR) CLEAN_RETURN(ERR_ALLOC);

	}

	for (; i < bufferCount; i++) {

		waveHeaders[i]->lpData = (LPSTR)waveBuffers[i];
		waveHeaders[i]->dwBufferLength = bufferLen;
		waveHeaders[i]->dwFlags = 0;
		waveHeaders[i]->dwLoops = 0L;

		error = waveOutPrepareHeader(*waveOut, waveHeaders[i], sizeof(WAVEHDR));
		if (error != MMSYSERR_NOERROR) CLEAN_RETURN(ERR_ALLOC);

	}

	this->frameSize = frameSize;
	this->framesInBuffer = info->maxBufferLength;

	this->bufferIdx = 0;
	this->bufferCount = bufferCount;
	this->bufferLen = bufferLen;

	return ERR_OK;

}

int AudioIOLegacy::start() {

	MMRESULT error = 0;
	//waveOutWrite(*waveOut, (LPWAVEHDR) waveHeaders[2], sizeof(WAVEHDR));
	//waveOutWrite(*waveOut, (LPWAVEHDR) waveHeaders[3], sizeof(WAVEHDR));

	// rename......
	if (reset > 0) {

		//waveOutWrite(*waveOut, (LPWAVEHDR)waveHeaders[2], sizeof(WAVEHDR));
		//waveOutWrite(*waveOut, (LPWAVEHDR) waveHeaders[3], sizeof(WAVEHDR));
		reset = 0;
		/*
		bufferIdx = 0;

		waveHeaders[0]->dwFlags = 0;
		waveHeaders[0]->dwBytesRecorded = 0;
		waveInPrepareHeader(*waveIn, waveHeaders[0], sizeof(WAVEHDR));
		waveInAddBuffer(*waveIn, (LPWAVEHDR)waveHeaders[0], sizeof(WAVEHDR));

		waveHeaders[1]->dwFlags = 0;
		waveHeaders[1]->dwBytesRecorded = 0;
		waveInPrepareHeader(*waveIn, waveHeaders[1], sizeof(WAVEHDR));
		waveInAddBuffer(*waveIn, (LPWAVEHDR) waveHeaders[1], sizeof(WAVEHDR));
		*/
		//waveHeaders[2]->dwFlags = 0;
		//waveOutPrepareHeader(*waveOut, waveHeaders[2], sizeof(WAVEHDR));

		//waveHeaders[3]->dwFlags = 0;
		//waveOutPrepareHeader(*waveOut, waveHeaders[3], sizeof(WAVEHDR));

	}
	//reset = 0;

	if (waveIn != NULL) error = waveInStart(*waveIn);

	return error != MMSYSERR_NOERROR;

}

int AudioIOLegacy::stop() {

	MMRESULT error = 0;
	//reset = 1;
	if (waveIn != NULL) error = waveInStop(*waveIn);

	return error != MMSYSERR_NOERROR;

}

int AudioIOLegacy::exit() {

	// stop();

	reset = 1;
	if (waveIn != NULL) waveInReset(*waveIn);
	if (waveOut != NULL) waveOutReset(*waveOut);

	if (waveIn != NULL) waveInClose(*waveIn);
	if (waveOut != NULL) waveOutClose(*waveOut);

	clean(this);

	return 0;

}

void AudioIOLegacy::setChannels(AudioDriver::DriverInfo* info) {

	leftChannelIn = info->channelIn & AudioDriver::CHANNEL_1;
	rightChannelIn = (info->channelIn & AudioDriver::CHANNEL_2) >> 1;
	leftChannelOut = info->channelOut & AudioDriver::CHANNEL_1;
	rightChannelOut = (info->channelOut & AudioDriver::CHANNEL_2) >> 1;

}

void AudioIOLegacy::openExternalConfig() {

}

AudioDriver::Device** AudioIOLegacy::getDevices(int* deviceCount) {

	AudioDriver::Device** devices;

	devices = (AudioDriver::Device**) malloc(sizeof(AudioDriver::Device*));
	devices[0] = (AudioDriver::Device*) malloc(sizeof(AudioDriver::Device));
	devices[0]->name = (char*) "Default";
	devices[0]->id = 0;

	*deviceCount = 1;

	return devices;

}

AudioIOLegacy::~AudioIOLegacy() {

	exit();

}

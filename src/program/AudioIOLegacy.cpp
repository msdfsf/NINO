#pragma once

#include "AudioIOLegacy.h"
#include "SampleFormat.h"

#include <limits.h>
#include <Audioclient.h>
//#include <Audiopolicy.h>
#include <mmdeviceapi.h>
#include <windows.h>
//#include <mmeapi.h>
#include <mmsystem.h>
#include <cmath>
#pragma comment(lib, "winmm")
#//pragma comment(lib,"libwinmm")

#define SAFE_RELEASE(punk) if ((punk) != NULL) { (punk)->Release(); (punk) = NULL; }
#define FREE(mem) { free((mem)); mem = NULL; }
#define CLEAN_RETURN(err) { clean(this); return (err); }

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

	error = audioClient->GetMixFormat((WAVEFORMATEX**) waveFormat);
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

	if (driver->waveFormat != NULL) FREE(driver->waveFormat);

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
			} else {
				return SampleFormat::INT_16;
			}

		} else if (format->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
			// assuming float

			return SampleFormat::FLOAT_32;
			
		} else {
			// dunno

			return SampleFormat::NONE;

		}

	} else {
		// could be almost anything, huh?

		if (((WAVEFORMATEXTENSIBLE*) format)->SubFormat == KSDATAFORMAT_SUBTYPE_PCM) {
			// assuming int

			if (format->wBitsPerSample == 8) {
				return SampleFormat::INT_8;
			} else if (format->wBitsPerSample == 16) {
				return SampleFormat::INT_16;
			} else if ((format->wBitsPerSample == 32)) {
				return SampleFormat::INT_32;
			} else {
				return SampleFormat::NONE;
			}

		} else if (((WAVEFORMATEXTENSIBLE*)format)->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
			// assuming float		

			if (format->wBitsPerSample == 32) {
				return SampleFormat::FLOAT_32;
			} else if (format->wBitsPerSample == 64) {
				return SampleFormat::FLOAT_64;
			} else {
				return SampleFormat::NONE;
			}

		} else {
			// dunno
			
			return SampleFormat::NONE;

		}
	
	}

}

void processCallback(HWAVEIN hwi, UINT msg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) {

	switch (msg) {
	
		case WIM_OPEN: {
	
			int x = 0;
			int y = x + 1;
			hwi;

			break;

		}

		case WIM_DATA: {

			// get needed data from driver
			//
			AudioIOLegacy* const driver = (AudioIOLegacy*) dwInstance;

			int buffIdx = driver->bufferIdx;
			const int buffLen = driver->bufferLen;
			const int buffCnt = driver->bufferCount;

			BYTE** const buffers = driver->waveBuffers;
			WAVEHDR** const headers = driver->waveHeaders;

			double* const internalBuffer = driver->buffer;
			
			const HWAVEOUT waveOut = *(driver->waveOut);



			// convert ready in buffer to our format
			//
			double* const inBuff = internalBuffer;
			double* const outBuff = internalBuffer + (2 * buffLen);

			switch (driver->sampleFormat) {

				case SampleFormat::INT_8: {

					const int iBuffLen = buffLen;
					int* const fInBuff = (int*)buffers[buffIdx];
					int* const fOutBuff = (int*)buffers[buffIdx + 2];

					// in
					if (!driver->leftChannelIn && !driver->rightChannelIn) {
						// zero fill

						for (int i = 0; i < iBuffLen; i++) {
							inBuff[i / 2] = 0;
						}

					} else if (driver->leftChannelIn && driver->rightChannelIn) {
						// both

						for (int i = 0; i < iBuffLen; i += 2) {
							inBuff[i / 2] = fInBuff[i] / (double) SCHAR_MAX + fInBuff[i + 1] / (double) SCHAR_MAX; // is it preciser? than double + double / int ?
						}

					} else if (driver->leftChannelIn) {
						// only left

						for (int i = 0; i < iBuffLen; i += 2) {
							inBuff[i / 2] = fInBuff[i] / (double) SCHAR_MAX;
						}

					} else {
						// only right

						for (int i = 1; i < iBuffLen; i += 2) {
							inBuff[i / 2] = fInBuff[i] / (double) SCHAR_MAX;
						}

					}

					// process
					driver->processInput((void*)inBuff, (void*)outBuff, buffLen / sizeof(float) / 2);

					// out
					if (!driver->leftChannelOut && !driver->rightChannelOut) {
						// zero fill

						for (int i = 0; i < iBuffLen; i++) {
							fOutBuff[i] = 0;
						}

					} else if (driver->leftChannelOut && driver->rightChannelOut) {
						// both

						for (int i = 0; i < iBuffLen; i += 2) {
							const double val = (outBuff[i / 2] * (double) SCHAR_MAX);
							fOutBuff[i] = outBuff[i + 1] = (val > SCHAR_MAX) ? SCHAR_MAX : val;
						}

					} else if (driver->leftChannelOut) {
						// only left

						for (int i = 0; i < iBuffLen; i += 2) {
							const double val = (outBuff[i / 2] * (double) SCHAR_MAX);
							fOutBuff[i] = (val > SCHAR_MAX) ? SCHAR_MAX : val;
						}

					} else {
						// only right

						for (int i = 1; i < iBuffLen; i += 2) {
							const double val = (outBuff[i / 2] * (double)SCHAR_MAX);
							fOutBuff[i] = (val > SCHAR_MAX) ? SCHAR_MAX : val;
						}

					}

					break;

				}
			
				case SampleFormat::INT_16: {

					const int iBuffLen = buffLen / 2;
					int* const fInBuff = (int*)buffers[buffIdx];
					int* const fOutBuff = (int*)buffers[buffIdx + 2];

					// in
					if (!driver->leftChannelIn && !driver->rightChannelIn) {
						// zero fill

						for (int i = 0; i < iBuffLen; i++) {
							inBuff[i / 2] = 0;
						}

					} else if (driver->leftChannelIn && driver->rightChannelIn) {
						// both

						for (int i = 0; i < iBuffLen; i += 2) {
							inBuff[i / 2] = fInBuff[i] / (double) SHRT_MAX + fInBuff[i + 1] / (double) SHRT_MAX; // is it preciser? than double + double / int ?
						}

					} else if (driver->leftChannelIn) {
						// only left

						for (int i = 0; i < iBuffLen; i += 2) {
							inBuff[i / 2] = fInBuff[i] / (double) SHRT_MAX;
						}

					} else {
						// only right

						for (int i = 1; i < iBuffLen; i += 2) {
							inBuff[i / 2] = fInBuff[i] / (double) SHRT_MAX;
						}

					}

					// process
					driver->processInput((void*)inBuff, (void*)outBuff, buffLen / sizeof(float) / 2);

					// out
					if (!driver->leftChannelOut && !driver->rightChannelOut) {
						// zero fill

						for (int i = 0; i < iBuffLen; i++) {
							fOutBuff[i] = 0;
						}

					} else if (driver->leftChannelOut && driver->rightChannelOut) {
						// both

						for (int i = 0; i < iBuffLen; i += 2) {
							const double val = (outBuff[i / 2] * (double) SHRT_MAX);
							fOutBuff[i] = outBuff[i + 1] = (val > SHRT_MAX) ? SHRT_MAX : val;
						}

					} else if (driver->leftChannelOut) {
						// only left

						for (int i = 0; i < iBuffLen; i += 2) {
							const double val = (outBuff[i / 2] * (double) SHRT_MAX);
							fOutBuff[i] = (val > SHRT_MAX) ? SHRT_MAX : val;
						}

					} else {
						// only right

						for (int i = 1; i < iBuffLen; i += 2) {
							const double val = (outBuff[i / 2] * (double) SHRT_MAX);
							fOutBuff[i] = (val > SHRT_MAX) ? SHRT_MAX : val;
						}

					}

					break;

				}

				case SampleFormat::INT_32: {

					const int iBuffLen = buffLen / 4;
					int* const fInBuff = (int*) buffers[buffIdx];
					int* const fOutBuff = (int*) buffers[buffIdx + 2];

					// in
					if (!driver->leftChannelIn && !driver->rightChannelIn) {
						// zero fill

						for (int i = 0; i < iBuffLen; i++) {
							inBuff[i / 2] = 0;
						}

					} else if (driver->leftChannelIn && driver->rightChannelIn) {
						// both

						for (int i = 0; i < iBuffLen; i += 2) {
							inBuff[i / 2] = fInBuff[i] / (double) INT_MAX + fInBuff[i + 1] / (double) INT_MAX; // is it preciser? than double + double / int ?
						}

					} else if (driver->leftChannelIn) {
						// only left

						for (int i = 0; i < iBuffLen; i += 2) {
							inBuff[i / 2] = fInBuff[i] / (double) INT_MAX;
						}

					} else {
						// only right

						for (int i = 1; i < iBuffLen; i += 2) {
							inBuff[i / 2] = fInBuff[i] / (double) INT_MAX;
						}

					}

					// process
					driver->processInput((void*)inBuff, (void*)outBuff, buffLen / sizeof(float) / 2);

					// out
					if (!driver->leftChannelOut && !driver->rightChannelOut) {
						// zero fill

						for (int i = 0; i < iBuffLen; i++) {
							fOutBuff[i] = 0;
						}

					} else if (driver->leftChannelOut && driver->rightChannelOut) {
						// both

						for (int i = 0; i < iBuffLen; i += 2) {
							const double val = (outBuff[i / 2] * (double) INT_MAX);
							fOutBuff[i] = outBuff[i + 1] = (val > INT_MAX) ? INT_MAX : val;
						}

					} else if (driver->leftChannelOut) {
						// only left

						for (int i = 0; i < iBuffLen; i += 2) {
							const double val = (outBuff[i / 2] * (double) INT_MAX);
							fOutBuff[i] = (val > INT_MAX) ? INT_MAX : val;
						}

					} else {
						// only right

						for (int i = 1; i < iBuffLen; i += 2) {
							const double val = (outBuff[i / 2] * (double)INT_MAX);
							fOutBuff[i] = (val > INT_MAX) ? INT_MAX : val;
						}

					}

					break;

				}

				case SampleFormat::FLOAT_32: {

					const int fBuffLen = buffLen / sizeof(float);
					float* const fInBuff = (float*) buffers[buffIdx];
					float* const fOutBuff = (float*) buffers[buffIdx + 2];

					// in
					if (!driver->leftChannelIn && !driver->rightChannelIn) {
						// zero fill

						for (int i = 0; i < fBuffLen; i++) {
							inBuff[i / 2] = 0;
						}

					} else if (driver->leftChannelIn && driver->rightChannelIn) {
						// both

						for (int i = 0; i < fBuffLen; i += 2) {
							inBuff[i / 2] = (double) fInBuff[i] + (double) fInBuff[i + 1];
						}

					} else if (driver->leftChannelIn) {
						// only left

						for (int i = 0; i < fBuffLen; i += 2) {
							inBuff[i / 2] = fInBuff[i];
						}

					} else {
						// only right

						for (int i = 1; i < fBuffLen; i += 2) {
							inBuff[i / 2] = fInBuff[i];
						}

					}

					// process
					driver->processInput((void*) inBuff, (void*) outBuff, fBuffLen / 2);
					
					// out
					if (!driver->leftChannelOut && !driver->rightChannelOut) {
						// zero fill

						for (int i = 0; i < fBuffLen; i++) {
							fOutBuff[i] = 0;
						}

					} else if (driver->leftChannelOut && driver->rightChannelOut) {
						// both

						for (int i = 0; i < fBuffLen; i += 2) {
							fOutBuff[i] = fOutBuff[i + 1] = outBuff[i / 2];
						}

					} else if (driver->leftChannelOut) {
						// only left

						for (int i = 0; i < fBuffLen; i += 2) {
							fOutBuff[i] = outBuff[i / 2];
						}

					} else {
						// only right

						for (int i = 1; i < fBuffLen; i += 2) {
							fOutBuff[i] = outBuff[i / 2];
						}

					}

					break;

				}

				case SampleFormat::FLOAT_64: {

					const int fBufflen = buffLen / sizeof(double);
					double* const fInBuff = (double*) buffers[buffIdx];
					double* const fOutBuff = (double*) buffers[buffIdx + 2];

					// in
					if (!driver->leftChannelIn && !driver->rightChannelIn) {
						// zero fill

						for (int i = 0; i < fBufflen; i++) {
							inBuff[i / 2] = 0;
						}

					} else if (driver->leftChannelIn && driver->rightChannelIn) {
						// both

						for (int i = 0; i < fBufflen; i += 2) {
							inBuff[i / 2] = fInBuff[i] + fInBuff[i + 1];
						}

					} else if (driver->leftChannelIn) {
						// only left

						for (int i = 0; i < fBufflen; i += 2) {
							inBuff[i / 2] = fInBuff[i];
						}

					} else {
						// only right

						for (int i = 1; i < fBufflen; i += 2) {
							inBuff[i / 2] = fInBuff[i];
						}

					}

					// process
					driver->processInput((void*) inBuff, (void*) outBuff, fBufflen / 2);

					// out
					if (!driver->leftChannelOut && !driver->rightChannelOut) {
						// zero fill

						for (int i = 0; i < fBufflen; i++) {
							fOutBuff[i] = 0;
						}

					} else if (driver->leftChannelOut && driver->rightChannelOut) {
						// both

						for (int i = 0; i < fBufflen; i += 2) {
							fOutBuff[i] = fOutBuff[i + 1] = outBuff[i / 2];
						}

					} else if (driver->leftChannelOut) {
						// only left

						for (int i = 0; i < fBufflen; i += 2) {
							fOutBuff[i] = outBuff[i / 2];
						}

					} else {
						// only right

						for (int i = 1; i < fBufflen; i += 2) {
							fOutBuff[i] = outBuff[i / 2];
						}

					}

					break;

				}

			}



			// manage driver buffers
			//
			headers[buffIdx + 2]->dwFlags = 0;
			waveOutPrepareHeader(waveOut, (LPWAVEHDR) headers[buffIdx + 2], sizeof(WAVEHDR));
			waveOutWrite(waveOut, (LPWAVEHDR) headers[buffIdx + 2], sizeof(WAVEHDR));

			headers[buffIdx]->dwFlags = 0;
			headers[buffIdx]->dwBytesRecorded = 0;

			waveInPrepareHeader(hwi, (LPWAVEHDR) headers[buffIdx], sizeof(WAVEHDR));
			waveInAddBuffer(hwi, (LPWAVEHDR) headers[buffIdx], sizeof(WAVEHDR));



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
	if (!waveFormat) return 1;

	sampleFormat = (SampleFormat::SampleFormat) resolveSampleFormat(waveFormat);

	MMRESULT error;

	const int bytesPerSecond = waveFormat->nSamplesPerSec * waveFormat->wBitsPerSample / 8;
	const int bufferLen = (bytesPerSecond * waveFormat->nChannels) * 0.016f;
	const int bufferCount = 4;

	info->maxBufferLength = (int) ceil(bufferLen / (double) sizeof(double));

	waveBuffers = (BYTE**) malloc(bufferCount * (sizeof(BYTE*)));
	if (!waveBuffers) return 1;

	waveHeaders = (WAVEHDR**) malloc(bufferCount * (sizeof(WAVEHDR*)));
	if (!waveHeaders) return 1;

	// whats that, is it any needed?
	buffer = (double*) malloc(4 * bufferCount * (sizeof(double)));
	if (!buffer) return 1;

	for (int i = 0; i < bufferCount; i++) {
		
		waveBuffers[i] = (BYTE*) malloc(bufferLen);
		if (!waveBuffers[i]) {
			
			for (int j = 0; j < i; j++) {
				free(waveBuffers[j]);
			}
			
			FREE(waveBuffers);
			
			return 1;
		
		}

		waveHeaders[i] = (WAVEHDR*) malloc(sizeof(WAVEHDR));
		if (!waveHeaders[i]) {

			for (int j = 0; j < i; j++) {
				free(waveHeaders[j]);
			}

			FREE(waveHeaders);

			return 1;

		}
	
	}

	waveOut = (HWAVEOUT*) malloc(sizeof(HWAVEOUT));
	if (!waveOut) CLEAN_RETURN(1);

	error = waveOutOpen(waveOut, WAVE_MAPPER, waveFormat, NULL, NULL, CALLBACK_FUNCTION);
	if (error != MMSYSERR_NOERROR) CLEAN_RETURN(1);

	waveIn = (HWAVEIN*) malloc(sizeof(HWAVEIN));
	if (!waveIn) CLEAN_RETURN(1);

	error = waveInOpen(waveIn, WAVE_MAPPER, waveFormat, (DWORD_PTR) &processCallback, (DWORD) this, CALLBACK_FUNCTION);
	if (error != MMSYSERR_NOERROR) CLEAN_RETURN(1);

	int i = 0;
	for (; i < bufferCount / 2; i++) {

		waveHeaders[i]->lpData = (LPSTR) waveBuffers[i];
		waveHeaders[i]->dwBufferLength = bufferLen;
		waveHeaders[i]->dwFlags = 0;
		waveHeaders[i]->dwLoops = 0L;

		error = waveInPrepareHeader(*waveIn, waveHeaders[i], sizeof(WAVEHDR));
		if (error != MMSYSERR_NOERROR) CLEAN_RETURN(1);

		error = waveInAddBuffer(*waveIn, (LPWAVEHDR) waveHeaders[i], sizeof(WAVEHDR));
		if (error != MMSYSERR_NOERROR) CLEAN_RETURN(1);

	}

	for (; i < bufferCount; i++) {

		waveHeaders[i]->lpData = (LPSTR) waveBuffers[i];
		waveHeaders[i]->dwBufferLength = bufferLen;
		waveHeaders[i]->dwFlags = 0;
		waveHeaders[i]->dwLoops = 0L;

		error = waveOutPrepareHeader(*waveOut, waveHeaders[i], sizeof(WAVEHDR));
		if (error != MMSYSERR_NOERROR) CLEAN_RETURN(1);

	}

	this->bufferIdx = 0;
	this->bufferCount = bufferCount;
	this->bufferLen = bufferLen;

}

int AudioIOLegacy::start() {

	waveOutWrite(*waveOut, (LPWAVEHDR) waveHeaders[2], sizeof(WAVEHDR));
	waveOutWrite(*waveOut, (LPWAVEHDR) waveHeaders[3], sizeof(WAVEHDR));
	waveInStart(*waveIn);

	return 0;

}

int AudioIOLegacy::stop() {

	MMRESULT error = waveInStop(*waveIn);

	return error != MMSYSERR_NOERROR;

}

int AudioIOLegacy::exit() {

	stop();

	waveInReset(*waveIn);
	waveOutReset(*waveOut);

	waveInClose(*waveIn);
	waveOutClose(*waveOut);
	
	clean(this);

	return 0;

}

void AudioIOLegacy::setChannels(AudioDriver::DriverInfo* info) {

}

void AudioIOLegacy::openExternalConfig() {

}

AudioDriver::Device** AudioIOLegacy::getDevices(int* deviceCount) {
	
	AudioDriver::Device** devices;

	devices = (AudioDriver::Device**)malloc(sizeof(AudioDriver::Device*));
	devices[0] = (AudioDriver::Device*)malloc(sizeof(AudioDriver::Device));
	devices[0]->name = (char*) "Default";
	devices[0]->id = 0;

	*deviceCount = 1;

	return devices;

}

AudioIOLegacy::~AudioIOLegacy() {

	exit();

}
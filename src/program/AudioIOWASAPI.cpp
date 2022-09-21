#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "AudioIOWASAPI.h"

#include <thread>
#include <avrt.h>
#include <thread>
#include <chrono>
#include <Audioclient.h>
#include <Audiopolicy.h>
#include <mmdeviceapi.h>
#include "Functiondiscoverykeys_devpkey.h"
#include "Utils.h"
#include "ErrorCodes.h"

#pragma comment(lib, "avrt")

#define REFERENCE_TIME_PER_SECOND 10000000
#define BUFFER_SIZE 128

#define EXIT_ON_ERROR(hres) if (FAILED(hres)) { goto cleanupAndExit; }
#define SAFE_RELEASE(punk) if ((punk) != NULL) { (punk)->Release(); (punk) = NULL; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

AudioIOWASAPI::AudioIOWASAPI() {

	renderClient = NULL;
	captureClient = NULL;

	renderService = NULL;
	captureService = NULL;

	renderFormat = NULL;
	captureFormat = NULL;

	renderEvent = NULL;
	captureEvent = NULL;

	cBufferFrameCount = 0;
	rBufferFrameCount = 0;

	outData = 0;
	inData = 0;

	captureBufferDuration = 0;

	error = NULL;

};

int AudioIOWASAPI::allocDataBuffers(const int inCount, const int outCount, const int length) {

	const int count = inCount + outCount;

	dataBuffers = (double*) malloc(count * length * sizeof(double));
	if (dataBuffers == NULL) {
		dataBufferInCount = 0;
		dataBufferOutCount = 0;
		return 1;
	}

	dataBufferInCount = inCount;
	dataBufferOutCount = outCount;
	dataBufferLength = length;

	return 0;

}

void AudioIOWASAPI::freeDataBuffers() {

	free(dataBuffers);
	dataBuffers = NULL;

	dataBufferInCount = 0;
	dataBufferOutCount = 0;
	dataBufferLength = 0;

}

int AudioIOWASAPI::init(AudioDriver::DriverInfo* info) {

	driverInfo = info;

	//
	// resolve info
	//

	char* const driverName = info->device->name;
	leftChannelIn = info->channelIn & AudioDriver::CHANNEL_1;
	rightChannelIn = (info->channelIn & AudioDriver::CHANNEL_2) >> 1;
	leftChannelOut = info->channelOut & AudioDriver::CHANNEL_1;
	rightChannelOut = (info->channelOut & AudioDriver::CHANNEL_2) >> 1;

	//
	// WASAPI itself
	//

	if (initialized) {
		
		stop();
		clean();
		initialized = 0;

	}

	error = initCapture(
		&captureClient,
		&captureService,
		&cBufferFrameCount,
		&captureFormat,
		&captureEvent,
		NULL,
		sharedMode
	);
	
	if (error != S_OK) {
		return ERR_AD_INVALID_INPUT_DEVICE;
	}

	error = initRender(
		&renderClient,
		&renderService,
		&rBufferFrameCount,
		&renderFormat,
		&renderEvent,
		NULL,
		sharedMode
	);

	if (error != S_OK) {
		return ERR_AD_INVALID_OUTPUT_DEVICE;
	}

	// now init our internal buffers
	const int buffSize = max(rBufferFrameCount, cBufferFrameCount);

	allocDataBuffers(2, 2, buffSize);
	driverInfo->maxBufferLength = buffSize;

	initialized = 1;
	return ERR_OK;

};

void AudioIOWASAPI::setChannels(AudioDriver::DriverInfo* info) {

	leftChannelIn = info->channelIn & AudioDriver::CHANNEL_1;
	rightChannelIn = (info->channelIn & AudioDriver::CHANNEL_2) >> 1;
	leftChannelOut = info->channelOut & AudioDriver::CHANNEL_1;
	rightChannelOut = (info->channelOut & AudioDriver::CHANNEL_2) >> 1;

}

void AudioIOWASAPI::openExternalConfig() {
	
}

int AudioIOWASAPI::start() {

	if (S_OK != captureClient->Start()) return 1;
	if (S_OK != renderClient->Start()) return 2;

	run = 1;
	workingThread = std::thread(&AudioIOWASAPI::handleIO, this);
	
	return 0;

};

AudioIOWASAPI::SampleFormat AudioIOWASAPI::getSampleFormat(WAVEFORMATEXTENSIBLE* format) {

	const int isInt = format->Format.wFormatTag == WAVE_FORMAT_PCM; // dunno

	switch (format->Format.wBitsPerSample) {

		case 8: {
			// assuming only int
			return AudioIOWASAPI::SampleFormat::INT_8;
		}

		case 16: {
			// assuming only int
			return AudioIOWASAPI::SampleFormat::INT_16;
		}

		case 32: {
			return (isInt) ? AudioIOWASAPI::SampleFormat::INT_32 : AudioIOWASAPI::SampleFormat::FLOAT_32;
		}

		case 64: {
			// assuming only float
			return AudioIOWASAPI::SampleFormat::FLOAT_64;
		}

		default: {
			return AudioIOWASAPI::SampleFormat::NONE;
		}

	}

}

// there has to be a better name, but ...
// assuming only 2 channels
void AudioIOWASAPI::handleIO() {

	DWORD flags;

	UINT32	rFramesAvailable;
	UINT32	cFramesAvailable;

	const int rChannelCount = renderFormat->nChannels;
	const int cChannelCount = captureFormat->nChannels;

	// in bytes
	const int rFrameSize = (renderFormat->wBitsPerSample / 8) * rChannelCount;
	const int cFrameSize = (captureFormat->wBitsPerSample / 8) * cChannelCount;

	// assuming valid (> 8 and %8 = 0) and same formats for both
	const SampleFormat rFormat = getSampleFormat((WAVEFORMATEXTENSIBLE*) renderFormat);
	const SampleFormat cFormat = getSampleFormat((WAVEFORMATEXTENSIBLE*) captureFormat);

	// ask for thread performance boost
	HANDLE taskHandler;
	unsigned long taskIndex = 0;
	//taskHandler = AvSetMmThreadCharacteristicsA("Pro Audio", &taskIndex);

	HANDLE events[2];
	events[0] = captureEvent;
	events[1] = renderEvent;

	while (run) {

		UINT32 framesAvailable;
		
		WaitForMultipleObjects(
			2,
			events,
			TRUE,
			INFINITE
		);

		if (
			!SUCCEEDED(captureService->GetBuffer(&inData, &framesAvailable, &flags, NULL, NULL)) ||
			!SUCCEEDED(renderService->GetBuffer(framesAvailable, &outData))
		) {
			continue;
		}

		if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
			memset(&outData, 0, framesAvailable * rFrameSize);
		} else {

			// input
			switch (cFormat) {
				// assuming that at least one buffer exists
				// all input will be summed into one

				case FLOAT_32: {

					double* const dataBuffer = dataBuffers;
					float* const inDataFloat = (float*) inData;

					if (!leftChannelIn && !rightChannelIn) {
						for (int i = 0; i < framesAvailable; i++) {
							dataBuffer[i] = 0;
						}
						break;
					}

					const int len = framesAvailable * cChannelCount;
					if (!leftChannelIn) {
						// only right

						for (int i = 1; i < len; i += 2) {
							dataBuffer[i / 2] = (double) inDataFloat[i];
						}

					}
					else if (!rightChannelIn) {
						// only left

						for (int i = 0; i < len; i += 2) {
							dataBuffer[i / 2] = (double) inDataFloat[i];
						}

					}
					else {
						// both

						for (int i = 0; i < len; i += 2) {
							dataBuffer[i / 2] = (double) inDataFloat[i] + (double) inDataFloat[i + 1];
						}

					}

					break;

				}

			}

		}

		// process
		double* const inBuff = dataBuffers;
		double* const outBuff = dataBuffers + dataBufferInCount * dataBufferLength;

		const int mix = processInput((void*) inBuff, (void*) outBuff, framesAvailable);

		// output
		switch (rFormat) {
			// assumes that at least one buffer exists

			case INT_8: {

				memset(&outData, 0, framesAvailable * rFrameSize);
				break;

			}

			case INT_16: {

				memset(&outData, 0, framesAvailable * rFrameSize);
				break;

			}

			case INT_32: {

				memset(&outData, 0, framesAvailable * rFrameSize);
				break;

			}

			case FLOAT_32: {

				float* const outDataFloat = (float*) outData;

				if (!leftChannelOut && !rightChannelOut) {
					for (int i = 0; i < framesAvailable * rChannelCount; i++) {
						outDataFloat[i] = 0;
					}
					// memset(&outData, 0, framesAvailable * rFrameSize); ??
					break;
				}

				if (!leftChannelOut) {
					// only right

					double* const outBuffL = (mix) ? outBuff + framesAvailable : outBuff;
					for (int i = 1; i < framesAvailable * rChannelCount; i += 2) {
						outDataFloat[i] = outBuffL[i / 2];
						outDataFloat[i - 1] = 0; // is memset faster?
					}

				} else if (!rightChannelOut) {
					// only left

					double* const outBuffR = (mix) ? outBuff + framesAvailable : outBuff;
					for (int i = 0; i < framesAvailable * rChannelCount; i += 2) {
						outDataFloat[i] = outBuffR[i / 2];
						outDataFloat[i + 1] = 0; // is memset faster?
					}

				} else {
					// both

					if (mix) {
						double* const outBuffR = outBuff + framesAvailable;
						for (int i = 0; i < framesAvailable * rChannelCount; i += 2) {
							const int idx = i / 2;
							outDataFloat[i] = outBuff[idx];
							outDataFloat[i + 1] = outBuffR[idx];
						}
					} else {
						for (int i = 0; i < framesAvailable * rChannelCount; i += 2) {
							outDataFloat[i] = (outDataFloat[i + 1] = outBuff[i / 2]);
						}
					}

				}

				break;

			}

			case FLOAT_64: {

				memset(&outData, 0, framesAvailable * rFrameSize);
				break;

			}

		}

		dataBufferIndex = !dataBufferIndex;

		renderService->ReleaseBuffer(framesAvailable, 0);
		captureService->ReleaseBuffer(framesAvailable);

		/*
		UINT32 framesAvailable;

		WaitForSingleObject(captureEvent, INFINITE);
		//WaitForSingleObject(renderEvent, INFINITE);
		if (SUCCEEDED(captureService->GetBuffer(&inData, &framesAvailable, &flags, NULL, NULL))) {

			WaitForSingleObject(renderEvent, INFINITE);

			HRESULT err = renderService->GetBuffer(framesAvailable, &outData);
			if (SUCCEEDED(err)) {

				if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
					
					memset(&outData, 0, framesAvailable * rFrameSize);
				
				} else {

					// input
					switch (cFormat) {
						// assuming that at least one buffer exists
						// all input will bi summed into one

						case FLOAT_32: {

							double* dataBuffer = dataBuffers[0] + dataBufferLength * (1 - dataBufferIndex);
							float* const inDataFloat = (float*) inData;

							if (!leftChannelIn && !rightChannelIn) {
								for (int i = 0; i < framesAvailable; i++) {
									dataBuffer[i] = 0;
								}
								break;
							}

							const int len = framesAvailable * cChannelCount;
							if (!leftChannelIn) {
								// only right

								for (int i = 1; i < len; i += 2) {
									dataBuffer[i / 2] = (double) inDataFloat[i];
								}
							
							} else if (!rightChannelIn) {
								// only left

								for (int i = 0; i < len; i += 2) {
									dataBuffer[i / 2] = (double) inDataFloat[i];
								}

							} else {
								// both

								for (int i = 0; i < len; i += 2) {
									dataBuffer[i / 2] = (double) inDataFloat[i] + (double) inDataFloat[i + 1];
								}

							}

							break;

						}
					
					}

					captureService->ReleaseBuffer(framesAvailable);

					// output
					switch (rFormat) {
						// assumes that at least one buffer exists

						case INT_8: {

							memset(&outData, 0, framesAvailable * rFrameSize);
							break;
						
						}

						case INT_16: {
							
							memset(&outData, 0, framesAvailable * rFrameSize);
							break;

						}

						case INT_32: {
							
							memset(&outData, 0, framesAvailable * rFrameSize);
							break;

						}

						case FLOAT_32: {

							float* const outDataFloat = (float*) outData;

							if (!leftChannelOut && !rightChannelOut) {
								for (int i = 0; i < framesAvailable * rChannelCount; i++) {
									outDataFloat[i] = 0;
								}
								// memset(&outData, 0, framesAvailable * rFrameSize); ??
								break;
							}

							const int inOffset = dataBufferLength * dataBufferIndex;
							const int outOffset = inOffset + 2 * dataBufferLength;

							// left
							const double* inBuffLeft = dataBuffers[0] + inOffset;
							const double* outBuffLeft = dataBuffers[0] + outOffset;

							processInput((void*) inBuffLeft, (void*) outBuffLeft, framesAvailable);

							if (!leftChannelOut) {
								// only right

								for (int i = 1; i < framesAvailable * rChannelCount; i += 2) {
									outDataFloat[i] = (outBuffLeft[i / 2]);
									outDataFloat[i - 1] = 0; // is memset faster?
								}

							} else if (!rightChannelOut) {
								// only left

								for (int i = 0; i < framesAvailable * rChannelCount; i += 2) {
									outDataFloat[i] = (outBuffLeft[i / 2]);
									outDataFloat[i + 1] = 0; // is memset faster?
								}

							} else {
								// both

								for (int i = 0; i < framesAvailable * rChannelCount; i += 2) {
									outDataFloat[i] = (outDataFloat[i + 1] = outBuffLeft[i / 2]);
								}

							}

							break;

						}

						case FLOAT_64: {

							memset(&outData, 0, framesAvailable * rFrameSize);
							break;

						}

					}

					dataBufferIndex = !dataBufferIndex;

				}

				err = renderService->ReleaseBuffer(framesAvailable, 0);

			}

			//renderService->ReleaseBuffer(framesAvailable, flags);
			captureService->ReleaseBuffer(framesAvailable);

		}
		*/

	}

}

int AudioIOWASAPI::stop() {
	
	if (!run) return 0;

	run = 0;

	workingThread.join();

	renderClient->Stop();
	captureClient->Stop();

	dataBufferIndex = 0;

	return 0;

};

int AudioIOWASAPI::exit() {

	stop();
	clean();

	return 0;

};

void AudioIOWASAPI::clean() {

	CoTaskMemFree(renderFormat);
	//SAFE_RELEASE(pEnumerator)
	//SAFE_RELEASE(pDevice)
	SAFE_RELEASE(renderClient)
	SAFE_RELEASE(renderService)

	CoTaskMemFree(captureFormat);
	//SAFE_RELEASE(pEnumerator)
	//SAFE_RELEASE(pDevice)
	SAFE_RELEASE(captureClient)
	SAFE_RELEASE(captureService)

	if (dataBuffers != NULL) {
		freeDataBuffers();
	}

}

//
//
// https://docs.microsoft.com/cs-cz/windows/win32/coreaudio/device-properties?redirectedfrom=MSDN
AudioDriver::Device** AudioIOWASAPI::getDevices(int* deviceCount) {

	AudioDriver::Device** devices = NULL;
	unsigned int i = 0;
	HRESULT hr = S_OK;
	IMMDeviceEnumerator* pEnumerator = NULL;
	IMMDeviceCollection* pCollection = NULL;
	IMMDevice* pEndpoint = NULL;
	IPropertyStore* pProps = NULL;
	LPWSTR pwszID = NULL;

	hr = CoCreateInstance(
		CLSID_MMDeviceEnumerator, 
		NULL,
		CLSCTX_ALL, 
		IID_IMMDeviceEnumerator,
		(void**) &pEnumerator
	);
	if (FAILED(hr)) goto exitAndCleanup;

	hr = pEnumerator->EnumAudioEndpoints(
		eRender, 
		DEVICE_STATE_ACTIVE,
		&pCollection
	);
	if (FAILED(hr)) goto exitAndCleanup;

	unsigned int count;
	hr = pCollection->GetCount(&count);
	if (FAILED(hr)) goto exitAndCleanup;

	if (count == 0) {
		// no endpoint devices
		*deviceCount = 0;
		return NULL;
	}

	{
		// go through found devices and store them

		const unsigned int len = count;

		devices = (AudioDriver::Device**) malloc(sizeof(AudioDriver::Device*) * len);
		if (devices == NULL) {
			goto exitAndCleanup;
		}

		for (; i < len; i++) {

			hr = pCollection->Item(i, &pEndpoint);
			if (FAILED(hr)) goto exitAndCleanup;

			hr = pEndpoint->GetId(&pwszID);
			if (FAILED(hr)) goto exitAndCleanup;

			hr = pEndpoint->OpenPropertyStore(STGM_READ, &pProps);
			if (FAILED(hr)) goto exitAndCleanup;

			PROPVARIANT varName;
			PropVariantInit(&varName);

			hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
			if (FAILED(hr)) goto exitAndCleanup;

			// store device
			devices[i] = (AudioDriver::Device*) malloc(sizeof(AudioDriver::Device));
			if (devices[i] == NULL) {
				goto exitAndCleanup;
			};

			const int wcDeviceNameLen = wcslen(varName.pwszVal);
			const int maxUtf8DeviceNameLen = 3 * 256;
			const int minDeviceNameLen = (maxUtf8DeviceNameLen < wcDeviceNameLen) ? maxUtf8DeviceNameLen : wcDeviceNameLen;

			char utf8DeviceName[maxUtf8DeviceNameLen + 1];
			char* tmp = (char*) utf8DeviceName;
			const int utf8DeviceNameLen = Utils::wc2utf8(varName.pwszVal, minDeviceNameLen, &tmp);
			utf8DeviceName[utf8DeviceNameLen] = '\0';

			devices[i]->name = (char*) malloc(sizeof(char) * utf8DeviceNameLen);
			if (devices[i]->name == NULL) {
				goto exitAndCleanup;
			}
			strcpy(devices[i]->name, tmp);
			
			devices[i]->id = i;
			
			//pwszID;

			CoTaskMemFree(pwszID);
			pwszID = NULL;
			PropVariantClear(&varName);
			if (pProps != NULL) {
				pProps->Release(); 
				pProps = NULL; 
			}
			if (pEndpoint != NULL) {
				pEndpoint->Release();
				pEndpoint = NULL;
			}

		}
		
		if (pEnumerator != NULL) {
			pEnumerator->Release();
			pEnumerator = NULL;
		}
		if (pCollection != NULL) {
			pCollection->Release();
			pCollection = NULL;
		}

		*deviceCount = len;

	}

	return devices;

exitAndCleanup:
	CoTaskMemFree(pwszID);
	if (pEnumerator != NULL) {
		pEnumerator->Release();
		pEnumerator = NULL;
	}
	if (pCollection != NULL) {
		pCollection->Release();
		pCollection = NULL;
	}
	if (pProps != NULL) {
		pProps->Release();
		pProps = NULL;
	}
	if (pEndpoint != NULL) {
		pEndpoint->Release();
		pEndpoint = NULL;
	}

	for (unsigned int j = 0; j < i; j++) {
		free(devices[i]->name);
		free(devices[j]);
	}
	free(devices);
	*deviceCount = 0;
	
	return NULL;

}

HRESULT AudioIOWASAPI::initRender(
	IAudioClient** audioClient,
	IAudioRenderClient** renderClient,
	uint32_t* bufferFrameCount,
	WAVEFORMATEX** waveFormat,
	HANDLE* renderEvent,
	LPCGUID sessionId,
	int sharedMode
) {

	const AUDCLNT_SHAREMODE SHARE_MODE = (sharedMode) ? AUDCLNT_SHAREMODE_SHARED : AUDCLNT_SHAREMODE_EXCLUSIVE;

	HRESULT error;

	// get default endpoint audio device for now

	IMMDeviceEnumerator* deviceEnumerator = NULL;
	IMMDevice* device = NULL;

	const int channelCount = (leftChannelOut && rightChannelOut) ? 2 : 1;

	error = CoCreateInstance(
		CLSID_MMDeviceEnumerator,
		NULL,
		CLSCTX_ALL,
		IID_IMMDeviceEnumerator,
		(void**) &deviceEnumerator
	);
	EXIT_ON_ERROR(error);

	// use later deviceEnumerator->lpVtbl->GetDevice(pEnumerator, DevID, &iMMDevice)
	error = deviceEnumerator->GetDefaultAudioEndpoint(
		eRender,
		eMultimedia, // dunno whats that
		&device
	);
	EXIT_ON_ERROR(error);

	// get audio client from audio device

	error = device->Activate(
		IID_IAudioClient,
		CLSCTX_ALL, // dunno how CLSCTX param works, so CLSCTX_ALL for now
		NULL,
		(void**) audioClient
	);
	EXIT_ON_ERROR(error);

	// get format
	
	if (SHARE_MODE == AUDCLNT_SHAREMODE_EXCLUSIVE) {
		error = getStreamFormat(
			(WAVEFORMATEXTENSIBLE**) waveFormat,
			sampleRate,
			bitResolution,
			channelCount
		);
	} else {
		error = (*audioClient)->GetMixFormat((WAVEFORMATEX**) waveFormat);
	}
	EXIT_ON_ERROR(error);

	// check if our format is supported, pretty useless
	// error = (*audioClient)->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, (WAVEFORMATEX*) *waveFormat, NULL);
	// 
	// init audio client

	// get minimal possible buffer duration
	REFERENCE_TIME requestedDuration; // 100-nanosecond units

	error = (*audioClient)->GetDevicePeriod(NULL, &requestedDuration);
	EXIT_ON_ERROR(error);

	error = (*audioClient)->Initialize(
		SHARE_MODE,
		AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
		requestedDuration, // 100-nanosecond units
		requestedDuration,
		*waveFormat,
		sessionId
	);

	if (error == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED) {
		// need to align buffer

		uint32_t frameCount = 0;
		error = (*audioClient)->GetBufferSize(&frameCount);
		EXIT_ON_ERROR(error);

		requestedDuration = (REFERENCE_TIME)((double)REFERENCE_TIME_PER_SECOND /
			(*waveFormat)->nSamplesPerSec * frameCount + 0.5);

		error = (*audioClient)->Initialize(
			SHARE_MODE,
			AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
			requestedDuration,
			requestedDuration,
			(*waveFormat),
			NULL
		);
		EXIT_ON_ERROR(error);

	} else {
		EXIT_ON_ERROR(error);
	}

	// create an event handle for buffer-event notifications
	*renderEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (*renderEvent == NULL) {
		error = -1;
		goto cleanupAndExit;
	}

	error = (*audioClient)->SetEventHandle(*renderEvent);
	EXIT_ON_ERROR(error);

	// get the actual size of buffers
	error = (*audioClient)->GetBufferSize(&(*bufferFrameCount));
	EXIT_ON_ERROR(error);

	// get render client

	error = (*audioClient)->GetService(
		IID_IAudioRenderClient,
		(void**)renderClient
	);
	EXIT_ON_ERROR(error);

	return S_OK;

cleanupAndExit:

	CoTaskMemFree(*waveFormat);
	SAFE_RELEASE(deviceEnumerator)
	SAFE_RELEASE(device)
	SAFE_RELEASE(*renderClient)
	SAFE_RELEASE(renderService)

	return error;

}

HRESULT AudioIOWASAPI::initCapture(
	IAudioClient** audioClient,
	IAudioCaptureClient** captureClient,
	uint32_t* bufferFrameCount,
	WAVEFORMATEX** waveFormat,
	HANDLE* captureEvent,
	LPCGUID sessionId,
	int sharedMode
) {

	const AUDCLNT_SHAREMODE SHARE_MODE = (sharedMode) ? AUDCLNT_SHAREMODE_SHARED : AUDCLNT_SHAREMODE_EXCLUSIVE;

	HRESULT error;

	// get default endpoint audio device for now

	IMMDeviceEnumerator* deviceEnumerator = NULL;
	IMMDevice* device = NULL;

	const int channelCount = (leftChannelIn && rightChannelIn) ? 2 : 1;

	error = CoCreateInstance(
		CLSID_MMDeviceEnumerator,
		NULL,
		CLSCTX_ALL,
		IID_IMMDeviceEnumerator,
		(void**) &deviceEnumerator
	);
	EXIT_ON_ERROR(error);

	// use later deviceEnumerator->lpVtbl->GetDevice(pEnumerator, DevID, &iMMDevice)
	error = deviceEnumerator->GetDefaultAudioEndpoint(
		eCapture,
		eMultimedia, // dunno whats that
		&device
	);
	EXIT_ON_ERROR(error);

	// get audio client from audio device

	error = device->Activate(
		IID_IAudioClient,
		CLSCTX_ALL, // dunno how CLSCTX param works, so CLSCTX_ALL for now
		NULL,
		(void**) audioClient
	);
	EXIT_ON_ERROR(error);

	// get format

	if (SHARE_MODE == AUDCLNT_SHAREMODE_EXCLUSIVE) {
		error = getStreamFormat(
			(WAVEFORMATEXTENSIBLE**) waveFormat,
			sampleRate,
			bitResolution,
			channelCount
		);
	} else {
		error = (*audioClient)->GetMixFormat((WAVEFORMATEX**) waveFormat);
	}
	EXIT_ON_ERROR(error);

	// check if our format is supported, useless..
	// error = (*audioClient)->IsFormatSupported(SHARE_MODE, (WAVEFORMATEX*)*waveFormat, NULL);

	// init audio client

	// get minimal possible buffer duration
	REFERENCE_TIME requestedDuration; // 100-nanosecond units

	error = (*audioClient)->GetDevicePeriod(NULL, &requestedDuration);
	EXIT_ON_ERROR(error);

	error = (*audioClient)->Initialize(
		SHARE_MODE,
		AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
		requestedDuration, // 100-nanosecond units
		requestedDuration,
		*waveFormat,
		sessionId
	);

	if (error == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED) {
		// need to align buffer

		uint32_t frameCount = 0;
		error = (*audioClient)->GetBufferSize(&frameCount);
		EXIT_ON_ERROR(error);

		requestedDuration = (REFERENCE_TIME)((double)REFERENCE_TIME_PER_SECOND /
			(*waveFormat)->nSamplesPerSec * frameCount + 0.5
			);

		error = (*audioClient)->Initialize(
			SHARE_MODE,
			AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
			requestedDuration,
			requestedDuration,
			(*waveFormat),
			NULL
		);
		EXIT_ON_ERROR(error);

	} else {
		EXIT_ON_ERROR(error);
	}

	// create an event handle for buffer-event notifications
	*captureEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (*captureEvent == NULL) {
		error = -1;
		goto cleanupAndExit;
	}

	error = (*audioClient)->SetEventHandle(*captureEvent);
	EXIT_ON_ERROR(error);

	// get the actual size of buffers
	error = (*audioClient)->GetBufferSize(&(*bufferFrameCount));
	EXIT_ON_ERROR(error);

	// get capture client

	error = (*audioClient)->GetService(
		IID_IAudioCaptureClient,
		(void**) captureClient
	);
	EXIT_ON_ERROR(error);

	return S_OK;

cleanupAndExit:

	CoTaskMemFree(*waveFormat);
	SAFE_RELEASE(deviceEnumerator)
	SAFE_RELEASE(device)
	SAFE_RELEASE(*captureClient)
	SAFE_RELEASE(captureService)

	return error;

}

int AudioIOWASAPI::getStreamFormat(
	WAVEFORMATEXTENSIBLE** waveFormatEx,
	unsigned int sampleRate,
	unsigned int bitResolution,
	int channelCount
) {

	const GUID pcmSubFormat = { STATIC_KSDATAFORMAT_SUBTYPE_PCM };
	
	WAVEFORMATEXTENSIBLE* waveFormat = (WAVEFORMATEXTENSIBLE*) calloc(1, sizeof(WAVEFORMATEXTENSIBLE));
	if (waveFormat == NULL) {
		*waveFormatEx = NULL;
		return S_FALSE;
	}
	*waveFormatEx = waveFormat;
	
	waveFormat->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	waveFormat->Format.nChannels = channelCount;
	waveFormat->Format.nSamplesPerSec = sampleRate;
	waveFormat->Format.cbSize = 22;
	waveFormat->Format.nBlockAlign = 2 * (bitResolution / 8);
	waveFormat->Samples.wValidBitsPerSample = bitResolution;
	waveFormat->Format.wBitsPerSample = bitResolution;
	/*
	if (bitResolution == 24) {
		waveFormat->Format.wBitsPerSample = 32;
		waveFormat->Format.nBlockAlign = 2 * (32 / 8);
	}
	*/
	memcpy(&waveFormat->SubFormat, &pcmSubFormat, sizeof(GUID));
	waveFormat->Format.nAvgBytesPerSec = sampleRate * waveFormat->Format.nBlockAlign;

	return S_OK;

}

void AudioIOWASAPI::toggleMode() {

	const int playing = this->run;
	
	this->init(this->driverInfo);

	if (run) {
		this->start();
	}

}

AudioIOWASAPI::~AudioIOWASAPI() {

	exit();

};

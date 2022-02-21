#pragma once

#include "AudioIOFile.h"
#include "WAVE.c"
#include "Config.h"
#include "Utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define MY_FILE_EXT ".RAW"
#define WAVE_FILE_EXT ".WAV"

#define BASIC_CLIP(in, out, max){\
	const double val = (in);\
	if (val >= 1) { out = (max); }\
	else if (val <= -1) { out = -(max); }\
	else { out = val * (max); }\
}

// best programming practices, thats whats keeps this code alive
#define BASIC_OUTPUT(outBuff, outData, buffSize, OUT_MAX, outLeft, outRight){\
	if ((outLeft) && (outRight)) {\
		for (int i = 0; i < (buffSize); i++) {\
			const int off = stepOut * (offset + i);\
			BASIC_CLIP((outBuff)[i], (outData)[off], (OUT_MAX));\
			BASIC_CLIP((outBuff)[i], (outData)[off + 1], (OUT_MAX));\
		}\
	} else if ((outLeft) || (outRight)) {\
		for (int i = 0; i < (buffSize); i++) {\
			BASIC_CLIP((outBuff)[i], (outData)[offset + i], (OUT_MAX));\
		}\
	} else {\
		memset(outData, 0, (buffSize) * sizeof(*(outData)));\
	}\
}



AudioIOFile::AudioIOFile() {
	
}

int AudioIOFile::init(AudioDriver::DriverInfo* info) {
	
	fileType = info->device->id;

	const int buffSize = 64;
	this->buffSize = buffSize;

	inDataLen = 0;
	inData = NULL;

	info->maxBufferLength = buffSize;
	driverInfo = info;

	return 0;

}

int load(AudioIOFile* const driver) {

	if (driver->inData) free(driver->inData);
	
	const int fileType = driver->fileType;

	if (fileType == 0) {
		// my format

		char flname[256];
		strcpy(flname, Config::inFileName);
		strcpy(flname + strlen(Config::inFileName), MY_FILE_EXT);

		FILE* file = fopen(flname, "rb");
		if (file == NULL) return 1;

		fseek(file, 0, SEEK_END);
		const int fileSize = ftell(file);
		fseek(file, 0, SEEK_SET);

		const int numberOfSamples = fileSize / 4;
		if (numberOfSamples < 1) {
			fclose(file);
			return 1;
		}

		driver->inData = (void*) malloc(sizeof(float) * numberOfSamples);
		if (!(driver->inData)) {
			fclose(file);
			return 1;
		}

		const int samplesRead = fread(driver->inData, sizeof(float), numberOfSamples, file);
		if (samplesRead <= 0) {
			fclose(file);
			return 1;
		}

		driver->inDataLen = samplesRead;

		fclose(file);

	} else if (fileType == 1) {
		// wave format

		char flname[256];
		strcpy(flname, Config::inFileName);
		strcpy(flname + strlen(Config::inFileName), WAVE_FILE_EXT);

		const int len = WAVEOpenFile(flname, (WAVE**) &driver->inData);
		if (len < 0) {
			return 1;
		} else {
			driver->inDataLen = len;
		}

		WAVE* const wav = (WAVE*) driver->inData;

		driver->audioFormat = wav->audioFormat;
		driver->sampleRate = wav->sampleRate;
		driver->bitsPerSample = wav->bitsPerSample;
		driver->bytesPerFrame = wav->blockAlign;
		driver->channelCount = wav->channelCount;

	} else {
		return 1;
	}

	driver->driverInfo->sampleRate = driver->sampleRate;
	driver->driverInfo->maxBufferLength = driver->buffSize;

	return 0;

}

template <typename InData, typename OutData>
void AudioIOFile::iProcessWAVE(
	InData inData, 
	OutData outData, 
	double* const inBuff,
	double* const outBuff,
	const int sgn
) {

	const int buffSize = this->buffSize;
	const int buffCount = inDataLen / buffSize;
	const int remLen = inDataLen - buffCount * buffSize;

	// will it work allways? is it pre compiled or something?
	const unsigned int inDataSize = sizeof(*inData) * 8;
	const unsigned int outDataSize = sizeof(*outData) * 8;

	// dunno about capslock, but why not)
	const long int IN_MAX = (1 << (inDataSize - 1)) - 1;
	const long int OUT_MAX = (1 << (outDataSize - 1)) - 1;

	const int OFFSET = (sgn) ? 0 : -IN_MAX - 1;

	const int channNum = this->channelCount;
	const int inLeft = this->leftChannelIn;
	const int inRight = this->rightChannelIn;
	const int outLeft = this->leftChannelOut;
	const int outRight = this->rightChannelOut;

	const int stepIn = bytesPerFrame / (inDataSize / 8);
	const int stepOut = (outLeft && outRight) ? 2 : 1;

	for (int i = 0; i < buffCount; i++) {

		const int offset = i * buffSize;

		if (channNum > 2 && inLeft && inRight) {

			for (int i = 0; i < buffSize; i++) {
				const int off = stepIn * (offset + i);
				inBuff[i] = ((double) (inData[off] + OFFSET) + (double) (inData[off + 1] + OFFSET)) / (double)IN_MAX;
			}

		} else if (!inLeft && !inRight) {

			// dont shure about mem set for floats, if it will work allways..
			for (int i = 0; i < buffSize; i++) {
				inBuff[i] = 0;
			}
		
		} else {
			
			for (int i = 0; i < buffSize; i++) {
				inBuff[i] = (inData[stepIn * (offset + i)] + OFFSET) / (double) IN_MAX;
			}
		
		}

		this->processInput((void*) inBuff, (void*) outBuff, buffSize);

		BASIC_OUTPUT(outBuff, outData, buffSize, OUT_MAX, outLeft, outRight);

	}

	if (remLen < 1) return;

	// and the overflow

	const int offset = buffCount * buffSize;

	if (channNum > 2 && inLeft && inRight) {

		for (int i = 0; i < remLen; i++) {
			const int off = stepIn * (offset + i);
			inBuff[i] = ((double) (inData[off] + OFFSET) + (double) (inData[off + 1] + OFFSET)) / (double) IN_MAX;
		}

	} else if (!inLeft && !inRight) {

		// dont shure about mem set for floats, if it will work allways..
		for (int i = 0; i < remLen; i++) {
			inBuff[i] = 0;
		}

	} else {

		for (int i = 0; i < remLen; i++) {
			inBuff[i] = (inData[stepIn * (offset + i)] + OFFSET) / (double) IN_MAX;
		}

	}

	for (int i = remLen; i < buffSize; i++) {
		inBuff[i] = 0;
	}

	this->processInput((void*) inBuff, (void*) outBuff, buffSize);

	BASIC_OUTPUT(outBuff, outData, buffSize, OUT_MAX, outLeft, outRight);

}

template <typename InData, typename OutData>
void AudioIOFile::fProcessWAVE(
	InData inData,
	OutData outData,
	double* const inBuff,
	double* const outBuff
) {

	const int buffSize = this->buffSize;
	const int buffCount = inDataLen / buffSize;
	const int remLen = inDataLen - buffCount * buffSize;

	const long int OUT_MAX = (1 << (sizeof(*outData) * 8 - 1)) - 1;

	const int channNum = this->channelCount;
	const int inLeft = this->leftChannelIn;
	const int inRight = this->rightChannelIn;
	const int outLeft = this->leftChannelOut;
	const int outRight = this->rightChannelOut;

	const int stepIn = bytesPerFrame / sizeof(*inData);
	const int stepOut = (outLeft && outRight) ? 2 : 1;

	for (int i = 0; i < buffCount; i++) {

		const int offset = i * buffSize;

		if (channNum > 2 && inLeft && inRight) {

			for (int i = 0; i < buffSize; i++) {
				const int off = stepIn * (offset + i);
				inBuff[i] = (double) (inData[off]) + (double) (inData[off + 1]);
			}

		} else if (!inLeft && !inRight) {

			// dont shure about mem set for floats, if it will work allways..
			for (int i = 0; i < buffSize; i++) {
				inBuff[i] = 0;
			}

		} else {

			for (int i = 0; i < buffSize; i++) {
				inBuff[i] = inData[stepIn * (offset + i)];
			}

		}

		this->processInput((void*)inBuff, (void*)outBuff, buffSize);

		BASIC_OUTPUT(outBuff, outData, buffSize, OUT_MAX, outLeft, outRight);

	}

	if (remLen < 1) return;

	// and the overflow

	const int offset = buffCount * buffSize;

	if (channNum > 2 && inLeft && inRight) {

		for (int i = 0; i < remLen; i++) {
			const int off = stepIn * (offset + i);
			inBuff[i] = (double)(inData[off]) + (double)(inData[off + 1]);
		}

	} else if (!inLeft && !inRight) {

		// dont shure about mem set for floats, if it will work allways..
		for (int i = 0; i < remLen; i++) {
			inBuff[i] = 0;
		}

	} else {

		for (int i = 0; i < remLen; i++) {
			inBuff[i] = inData[stepIn * (offset + i)];
		}

	}

	for (int i = remLen; i < buffSize; i++) {
		inBuff[i] = 0;
	}

	this->processInput((void*)inBuff, (void*)outBuff, buffSize);

	BASIC_OUTPUT(outBuff, outData, buffSize, OUT_MAX, outLeft, outRight);

}

int AudioIOFile::start() {

	const int buffSize = this->buffSize;
	double* inBuff = NULL;
	double* outBuff = NULL;

	leftChannelIn = driverInfo->channelIn & AudioDriver::CHANNEL_1;
	rightChannelIn = (driverInfo->channelIn & AudioDriver::CHANNEL_2) >> 1;
	leftChannelOut = driverInfo->channelOut & AudioDriver::CHANNEL_1;
	rightChannelOut = (driverInfo->channelOut & AudioDriver::CHANNEL_2) >> 1;

	if (load(this)) goto errExit;
	
	inBuff = (double*) malloc(buffSize * sizeof(double));
	if (!inBuff) goto errExit;

	outBuff = (double*) malloc(buffSize * sizeof(double));
	if (!outBuff) {
		free(inBuff);
		goto errExit;
	}

	if (fileType == 0) {

		const int buffCount = inDataLen / buffSize;
		const int remLen = inDataLen - buffCount * buffSize;

		float* data = (float*) inData;
		float* outData = (float*) malloc(inDataLen * sizeof(float));
		if (!outData) {
			free(inBuff);
			free(outBuff);
			return 1;
		}

		for (int i = 0; i < buffCount; i++) {
			
			const int offset = i * buffSize;

			for (int i = 0; i < buffSize; i++) {
				inBuff[i] = data[offset + i];
			}

			this->processInput((void*) inBuff, (void*) outBuff, buffSize);

			for (int i = 0; i < buffSize; i++) {
				outData[offset + i] = outBuff[i];
			}

		}

		if (remLen > 0) {

			const int offset = buffCount * buffSize;

			for (int i = 0; i < remLen; i++) {
				inBuff[i] = data[offset + i];
			}

			for (int i = remLen; i < buffSize; i++) {
				inBuff[i] = 0;
			}

			this->processInput((void*)inBuff, (void*)outBuff, buffSize);

			for (int i = 0; i < remLen; i++) {
				outData[offset + i] = outBuff[i];
			}

		}

		char flname[256];
		strcpy(flname, Config::outFileName);
		strcpy(flname + strlen(Config::outFileName), MY_FILE_EXT);

		FILE* file = fopen(flname, "wb");
		if (file) {
			fwrite(outData, 4, inDataLen, file);
		}

		fclose(file);

		free(inBuff);
		free(outBuff);
		free(outData);

		if (!file) goto errExit;

	} else if (fileType == 1) {
	
		const int outChannelCount = (leftChannelOut && rightChannelOut) ? 2 : 1;

		const int outDataLen = inDataLen;
		WAVE* outWave;
		if (WAVEPrepare(&outWave, outDataLen, sampleRate, outChannelCount)) {
			free(inBuff);
			free(outBuff);
			return 1;
		}

		char* const inData = (char*) ((WAVE*) this->inData)->data;
		int32_t* const outData = (int32_t*) ((WAVE*) outWave)->data;

		switch (bytesPerFrame * 8 / channelCount) {
				
			case 8 : {
				// unsigned for some reason...

				iProcessWAVE((uint8_t*) inData, outData, inBuff, outBuff, 0);
				break;

			}

			case 16 : {
				
				iProcessWAVE((int16_t*) inData, outData, inBuff, outBuff);
				break;

			}

			case 24 : {
				// bytes in reverse order, the microsoft is used to, or whatever happenes i dont 
				// know actualy, but it seems to works this way, at least on windows...

				const int maxVal = INT_MAX - 256;
				const int buffCount = inDataLen / buffSize;
				const int remLen = inDataLen - buffCount * buffSize;

				const int stepIn = bytesPerFrame;
				const int stepOut = outChannelCount;

				unsigned char* const data = (unsigned char*) inData;

				for (int i = 0; i < buffCount; i++) {

					const int offset = i * buffSize;

					if (channelCount > 2 && leftChannelIn && rightChannelIn) {

						for (int i = 0; i < buffSize; i++) {
							unsigned char* const frame = data + stepIn * (offset + i);
							const int32_t valA = *(frame + 2) << 24 | *(frame + 1) << 16 | (*frame) << 8;
							const int32_t valB = *(frame + 5) << 24 | *(frame + 4) << 16 | (*frame + 3) << 8;
							inBuff[i] = ((double) valA + (double) valB) / (double) maxVal;
						}

					} else if (!leftChannelIn && !rightChannelIn) {

						// dont shure about mem set for floats, if it will work allways..
						for (int i = 0; i < buffSize; i++) {
							inBuff[i] = 0;
						}

					} else {

						for (int i = 0; i < buffSize; i++) {
							unsigned char* const frame = data + stepIn * (offset + i);
							const int32_t val = *(frame + 2) << 24 | *(frame + 1) << 16 | (*frame) << 8;
							inBuff[i] = val / (double) maxVal;
						}

					}

					this->processInput((void*) inBuff, (void*) outBuff, buffSize);

					BASIC_OUTPUT(outBuff, outData, buffSize, INT_MAX, leftChannelOut, rightChannelOut);

				}

				if (remLen < 1) break;

				const int offset = buffCount * buffSize;

				for (int i = 0; i < remLen; i++) {
					unsigned char* const frame = data + stepIn * (offset + i);
					const int32_t val = *(frame + 2) << 24 | *(frame + 1) << 16 | *frame << 8;
					inBuff[i] = val / (double) maxVal;
				}

				for (int i = remLen; i < buffSize; i++) {
					inData[offset + i] = 0;
				}

				this->processInput((void*) inBuff, (void*) outBuff, buffSize);

				BASIC_OUTPUT(outBuff, outData, remLen, INT_MAX, leftChannelOut, rightChannelOut);
				
				break;

			}

			case 32 : {
				
				if (audioFormat == 1) iProcessWAVE((int32_t*) inData, outData, inBuff, outBuff);
				else fProcessWAVE((float*) inData, outData, inBuff, outBuff);
				break;

			}

			case 64 : {

				if (audioFormat == 1) iProcessWAVE((int64_t*) inData, outData, inBuff, outBuff);
				else fProcessWAVE((double*) inData, outData, inBuff, outBuff);
				break;

			}
			
		}

		char flname[256];
		strcpy(flname, Config::outFileName);
		strcpy(flname + strlen(Config::outFileName), WAVE_FILE_EXT);

		const int err = (bool) WAVEToFile(flname, outWave);

		free(inBuff);
		free(outBuff);
		WAVEFree(outWave);
		
		if (err) goto errExit;

	}

	// too lazy for goto stuff, copy - paste is my friend tonight

	Utils::showMessage("Output file is ready.\nTo start again, toggle two more times power button.");
	return 0;

errExit:
	Utils::showMessage("Something went wrong!");
	return 1;

}

int AudioIOFile::stop() {

	return 0;

}

int AudioIOFile::exit() {

	return 0;

}

void AudioIOFile::setChannels(AudioDriver::DriverInfo* info) {

}

void AudioIOFile::openExternalConfig() {

}

AudioDriver::Device** AudioIOFile::getDevices(int* deviceCount) {
	
	const int dCount = 2;

	AudioDriver::Device** devices;

	devices = (AudioDriver::Device**) malloc(dCount * sizeof(AudioDriver::Device*));

	devices[0] = (AudioDriver::Device*) malloc(sizeof(AudioDriver::Device));
	devices[0]->name = (char*) "RAW DATA";
	devices[0]->id = 0;

	devices[1] = (AudioDriver::Device*) malloc(sizeof(AudioDriver::Device));
	devices[1]->name = (char*) "WAVE FILE";
	devices[1]->id = 1;

	*deviceCount = dCount;

	return devices;

}

AudioIOFile::~AudioIOFile() {

	if (inData) free(inData);

}
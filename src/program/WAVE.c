// http://soundfile.sapp.org/doc/WaveFormat/
// http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html

#pragma once

#include "WAVE.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define RIFF_CHUNK_ID 0x46464952
#define WAVE_CHUNK_ID 0x45564157
#define FMT_CHUNK_ID 0x20746d66
#define DATA_CHUNK_ID 0x61746164

const int DF_SAMPLE_SIZE = 4; // in bytes

int WAVEPrepare(WAVE** wave, int len, int sampleRate, int channelCount) {

	*wave = (WAVE*) malloc(sizeof(WAVE));
	if (!*wave) return 1;

	WAVE* const wav = *wave;

	wav->audioFormat = 1;

	wav->sampleRate = sampleRate;
	wav->byteRate = DF_SAMPLE_SIZE * channelCount * sampleRate;
	wav->blockAlign = DF_SAMPLE_SIZE * channelCount;
	wav->bitsPerSample = DF_SAMPLE_SIZE * 8;
	wav->channelCount = channelCount;

	wav->dataSize = len;
	wav->data = (unsigned char*) malloc(WAVE_HEADER_SIZE + len * channelCount * DF_SAMPLE_SIZE);
	
	if (!wav->data) {
		free(*wave);
		*wave = NULL;
		return 1;
	}

	return 0;

}

#define RETURN_AND_FCLOSE(file) { fclose(file); return -1; }
int WAVEToFile(char* flname, WAVE* wave) {

	FILE* file = fopen(flname, "wb");
	if (!file) return -1;



	// 1 - 4 bytes, RIFF id
	uint32_t riffId = RIFF_CHUNK_ID;
	if (fwrite(&riffId, 4, 1, file) < 1) RETURN_AND_FCLOSE(file);

	// 5 - 8 bytes, ChunkSize
	uint32_t chunkSize = WAVE_HEADER_SIZE - 8 + wave->dataSize * wave->blockAlign;
	if (fwrite(&chunkSize, 4, 1, file) < 1) RETURN_AND_FCLOSE(file);

	// 9 - 12 bytes, Format
	uint32_t waveId = WAVE_CHUNK_ID;
	if (fwrite(&waveId, 4, 1, file) < 1) RETURN_AND_FCLOSE(file);



	// 13 - 16 bytes, Subchunk1ID
	uint32_t fmtId = FMT_CHUNK_ID;
	if (fwrite(&fmtId, 4, 1, file) < 1) RETURN_AND_FCLOSE(file);

	// 17 - 20 bytes, Subchunk1Size
	uint32_t pcmFormatBasicSize = 16;
	if (fwrite(&pcmFormatBasicSize, 4, 1, file) < 1) RETURN_AND_FCLOSE(file);

	// 21 - 22 bytes, AudioFormat
	uint16_t audioFormat = WAVE_FORMAT_PCM;
	if (fwrite(&audioFormat, 2, 1, file) < 1) RETURN_AND_FCLOSE(file);

	// 23 - 24 bytes, NumChannels
	uint16_t channNum = wave->channelCount;
	if (fwrite(&channNum, 2, 1, file) < 1) RETURN_AND_FCLOSE(file);

	// 25 - 28 bytes, SampleRate
	uint32_t sampleRate = wave->sampleRate;
	if (fwrite(&sampleRate, 4, 1, file) < 1) RETURN_AND_FCLOSE(file);

	// 29 - 32 bytes, ByteRate
	uint32_t byteRate = wave->byteRate;
	if (fwrite(&byteRate, 4, 1, file) < 1) RETURN_AND_FCLOSE(file);

	// 33 - 34 bytes, BlockAlign
	uint16_t blockAlign = wave->blockAlign;
	if (fwrite(&blockAlign, 2, 1, file) < 1) RETURN_AND_FCLOSE(file);

	// 35 - 36 bytes, BitsPerSample
	uint16_t bitsPerSample = wave->bitsPerSample;
	if (fwrite(&bitsPerSample, 2, 1, file) < 1) RETURN_AND_FCLOSE(file);

	// here can be extra params, but we dont give a fuck for now, as PCM only for now (and forever)



	// 37 - 40 bytes, Subchunk2ID
	uint32_t dataId = DATA_CHUNK_ID;
	if (fwrite(&dataId, 4, 1, file) < 1) RETURN_AND_FCLOSE(file);

	// 41 - 44 bytes Subchunk2Size
	uint32_t dataSize = wave->dataSize * wave->blockAlign;
	if (fwrite(&dataSize, 4, 1, file) < 1) RETURN_AND_FCLOSE(file);

	// 45 - whatever bytes, actual data
	if (fwrite(wave->data, dataSize, 1, file) < 1) RETURN_AND_FCLOSE(file);

	fclose(file);

	return 0;

}

int WAVEOpenFile(char* flname, WAVE** wave) {
	
	uint32_t fileSize = 0;
	uint32_t sizeRead = 4;

	uint16_t format;
	uint16_t channNum;
	uint32_t sampleRate;
	uint32_t bytesPerSample;
	uint16_t blockAlign;
	uint16_t bitsPerSample;
	uint16_t extSize;
	uint16_t validBitsPerSample;
	uint32_t channelMask;
	char subFormat[16];

	uint32_t dataSize = 0;
	unsigned char* data = NULL;


	FILE* file = fopen(flname, "rb");
	if (!file) return -1;

	// skip RIFF header
	if (fseek(file, 4, SEEK_CUR)) goto exit;

	// read basicly the size of the rest of the file
	if (fread(&fileSize, 4, 1, file) < 1) goto exit;

	// skip WAVE header or whatever it is
	if (fseek(file, 4, SEEK_CUR)) goto exit;

	while (sizeRead < fileSize) {
	
		uint32_t chunkId;
		uint32_t chunkSize;

		// read 4byte chunk id
		if (fread(&chunkId, 4, 1, file) < 1) {
			if (!feof(file)) goto exit;
			else break;
		}

		// read 4byte size of chunk, only consider the next bytes
		if (fread(&chunkSize, 4, 1, file) < 1) goto exit;

		sizeRead += 8 + chunkSize;

		switch (chunkId) {
			
			case FMT_CHUNK_ID : {
		
				// basic format
				if (fread(&format, 2, 1, file) < 1) goto exit;
				if (fread(&channNum, 2, 1, file) < 1) goto exit;
				if (fread(&sampleRate, 4, 1, file) < 1) goto exit;
				if (fread(&bytesPerSample, 4, 1, file) < 1) goto exit;
				if (fread(&blockAlign, 2, 1, file) < 1) goto exit;
				if (fread(&bitsPerSample, 2, 1, file) < 1) goto exit;

				if (chunkSize <= 16) break;

				// check for extension size
				if (fread(&extSize, 2, 1, file) < 1) goto exit;
				if (extSize < 22) break;

				// read the extension
				if (fread(&validBitsPerSample, 2, 1, file) < 1) goto exit;
				if (fread(&channelMask, 4, 1, file) < 1) goto exit;
				if (fread(subFormat, 16, 1, file) < 1) goto exit;

				break;

			}

			case DATA_CHUNK_ID: {

				// basicly just raw data, nothing special
				data = (unsigned char*) malloc(chunkSize);
				if (!data || fread(data, chunkSize, 1, file) < 1) goto exit;
				dataSize = chunkSize;

				break;

			}
			
			default: {

				// read size to skip
				if (fseek(file, chunkSize, SEEK_CUR)) goto exit;

			}

		}	
	
	}

	*wave = (WAVE*) malloc(sizeof(WAVE));
	if (!*wave) goto exit;

	(*wave)->audioFormat = format;
	(*wave)->sampleRate = sampleRate;
	(*wave)->byteRate = bytesPerSample;
	(*wave)->blockAlign = blockAlign;
	(*wave)->bitsPerSample = bitsPerSample;
	(*wave)->channelCount = channNum;

	(*wave)->dataSize = dataSize / blockAlign;
	(*wave)->data = data;

	return dataSize / blockAlign;

exit:
	fclose(file);
	if (data != NULL) free(data);
	return -1;

}

void WAVEFree(WAVE* wave) {
	
	if (wave != NULL) {
		if (wave->data != NULL) {
			free(wave->data);
			wave->data = NULL;
		}
		free(wave);
		wave = NULL;
	}

}
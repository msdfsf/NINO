#pragma once

// in bytes
#define WAVE_HEADER_SIZE 44

typedef enum {

	WAVE_FORMAT_PCM = 1,
	WAVE_FORMAT_IEEE_FLOAT = 3,

} WaveFormat;

// 'WAVE', is this the right name, or at least not that bad? 
typedef struct WAVE {

	int audioFormat;

	int sampleRate;
	int byteRate;
	int blockAlign;
	int bitsPerSample;
	int channelCount;

	unsigned int dataSize;
	unsigned char* data;

} WAVE;

int WAVEPrepare(WAVE** wave, int len, int sampleRate);
int WAVEOpenFile(char* flname, WAVE* wave);
int WAVEToFile(char* flname, WAVE* wave);
void WAVEFree(WAVE* wave);
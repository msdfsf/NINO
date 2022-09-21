#pragma once

#include "Plugin.h"
#include "TheSimpleTheBadAndTheUglyDSPLibrary.h"

#include <stdlib.h>
#include <math.h>
#include <corecrt_math_defines.h>

#define TEXT_COLOR 0xFF000000

#define WND_SIZE	(256 * 2)
#define FFT_SIZE	(512 * 2)
#define STEP		64
#define OVERLAP	(WND_SIZE - STEP)

#define MIN_MIX -1
#define MAX_MIX 1
#define DF_MIX 0

#define MAX_RATIO 2
#define MIN_RATIO 0.5

#define OCTAVE_ONE_UP 1
#define OCTAVE_ONE_DOWN 0

#define SWAP_POINTERS(a, b) {\
	void* tmp = (void*) (a);\
	a = b;\
	(void*) b = tmp;\
}

#define FILL_OUT_BUFFER(outDataIdx, outIdx) {\
	const double mixRatio = spc->outMixRatio;\
	for (int i = (outDataIdx); i >= 0; i--) {\
		const int offset = STEP * ((outDataIdx)-i);\
		for (int j = 0; j < bufferLength; j++) {\
			outBuff[j] += mixRatio * outData[i][(outIdx) + offset + j];\
		}\
	}\
	for (int i = outDataCount - 1; i > (outDataIdx); i--) {\
		const int offset = STEP * ((outDataIdx) + outDataCount - i);\
		for (int j = 0; j < bufferLength; j++) {\
			outBuff[j] += mixRatio * outData[i][(outIdx) + offset + j];\
		}\
	}\
}

typedef struct Space {

	int sampleRate;

	int dataIdx;
	double* data;

	int outIdx;
	int outSize;
	int outDataIdx;
	int outDataLen;
	int outDataCount;
	double** outData;

	int firstIteration;

	double* fftBuff;

	double* mag;
	double* phase;
	double* prevPhase;
	double* phaseDiff;
	double* prevPhaseDiff;
	double* newPhase;
	double* prevNewPhase;

	int octave;
	double mixRatio;

	double inMixRatio;
	double outMixRatio;
	double pitchRatio;

} Space;

double wndFrame[WND_SIZE];
double wndFFT[FFT_SIZE];
double phaseBin[FFT_SIZE];
fComplex expWn[FFT_SIZE];

int init(IPluginInfo* info, void** space) {

	const int outDataMaxLen = FFT_SIZE * MAX_RATIO;
	const int outDataMaxCount = outDataMaxLen / STEP;

	((Space*) *space) = (Space*) malloc(sizeof(Space));
	if (!((Space*)*space)) return 1;

	Space* const spc = (Space*) *space;

	spc->data = (double*) malloc(WND_SIZE * sizeof(double));
	if (!spc->data) return 1;

	spc->outData = (double**) malloc(outDataMaxCount * sizeof(double*));
	if (!spc->outData) return 1;

	for (int i = 0; i < outDataMaxCount; i++) {
		(spc->outData)[i] = (double*) malloc(outDataMaxLen * sizeof(double));
		double* const tmp = (spc->outData)[i];
		for (int i = 0; i < outDataMaxLen; i++) {
			tmp[i] = 0.0;
		}
	}

	spc->fftBuff = (double*) calloc((2 * FFT_SIZE), sizeof(double));
	if (!spc->fftBuff) return 1;

	spc->mag = (double*) calloc((FFT_SIZE + 1), sizeof(double));
	if (!spc->mag) return 1;

	spc->phase = (double*) calloc((FFT_SIZE + 1), sizeof(double));
	if (!spc->phase) return 1;

	spc->prevPhase = (double*) calloc((FFT_SIZE + 1), sizeof(double));
	if (!spc->prevPhase) return 1;

	spc->phaseDiff = (double*) calloc((FFT_SIZE + 1), sizeof(double));
	if (!spc->phaseDiff) return 1;

	spc->prevPhaseDiff = (double*) calloc((FFT_SIZE + 1), sizeof(double));
	if (!spc->prevPhaseDiff) return 1;

	spc->newPhase = (double*) calloc((FFT_SIZE + 1), sizeof(double));
	if (!spc->newPhase) return 1;

	spc->prevNewPhase = (double*) calloc((FFT_SIZE + 1), sizeof(double));
	if (!spc->prevNewPhase) return 1;

	spc->dataIdx = 0;
	spc->outIdx = 64;
	spc->outDataIdx = outDataMaxCount - 1;
	spc->outDataLen = outDataMaxLen;
	spc->outDataCount = outDataMaxCount;
	spc->firstIteration = 1;

	spc->pitchRatio = 2;
	spc->firstIteration = 1;

	spc->octave = 0;
	spc->mixRatio = 0;

	spc->inMixRatio = 1 - (1 + DF_MIX) / (double) 2;
	spc->outMixRatio = (1 + DF_MIX) / (double) 2;

	for (int i = 0; i < FFT_SIZE; i++) {
		phaseBin[i] = i * (2 * M_PI * (double) WND_SIZE / (double) FFT_SIZE);
	}

	for (int i = 0; i < WND_SIZE; i++) {
		wndFrame[i] = pow(sin(M_PI * i / (double) WND_SIZE), 2);
	}

	for (int i = 0; i < FFT_SIZE; i++) {
		wndFFT[i] = pow(sin(M_PI * i / (double) FFT_SIZE), 2);
	}

	getDFTMatrix(expWn, FFT_SIZE);

	return 0;
}

void process(void* inBuffer, void* outBuffer, int bufferLength, void* space) {

	Space* const spc = (Space*)space;

	int dataIdx = spc->dataIdx;
	double* const data = spc->data;

	const int octave = spc->octave;

	const int outIdx = spc->outIdx;
	const int outSize = spc->outSize;
	const int outDataIdx = spc->outDataIdx;
	const int outDataCount = spc->outDataCount;
	double** outData = spc->outData;

	const int firstIteration = spc->firstIteration;

	double* const  fftBuff = spc->fftBuff;

	double* const mag = spc->mag;
	double* const phase = spc->phase;
	double* const prevPhase = spc->prevPhase;
	double* const phaseDiff = spc->phaseDiff;
	double* const prevPhaseDiff = spc->prevPhaseDiff;
	double* newPhase = spc->newPhase;
	double* const prevNewPhase = spc->prevNewPhase;

	const double pitchRatio = spc->pitchRatio;

	double* const inBuff = inBuffer;
	double* const outBuff = outBuffer;

	{
		const double mixRatio = spc->inMixRatio;
		for (int i = 0; i < bufferLength; i++) {
			outBuff[i] = mixRatio * inBuff[i];
		}
	}

	// (outIdx - bufferLength) may replace with new variable dataRead or something, as its not valid
	// in general( if data that we get after interpolation are shorter than bufferLength)
	const int remDelta = ((firstIteration ? WND_SIZE : STEP) - (outIdx - bufferLength) - bufferLength);

	if (remDelta > 0) {
		// buffer wiil not be filled yet
		// we are safe to record all data

		// fill the circle buffer
		if (dataIdx + bufferLength > WND_SIZE) {

			for (int i = dataIdx; i < WND_SIZE; i++) {
				data[i] = inBuff[i - dataIdx];
			}

			int i = 0;
			for (; i < dataIdx + bufferLength - WND_SIZE; i++) {
				data[i] = inBuff[WND_SIZE - dataIdx + i];
			}

			spc->dataIdx = i;

		} else {

			for (int i = 0; i < bufferLength; i++) {
				data[dataIdx + i] = inBuff[i];
			}

			spc->dataIdx += bufferLength;

		}

		// !!!!
		// bufferLength = min(bufferLength, len(outData[]));
		// !!!!
		
		// pass data from the output buffers to the output
		FILL_OUT_BUFFER(outDataIdx, outIdx);
		/*
		{
			const double mixRatio = spc->outMixRatio;

			for (int i = outDataIdx; i >= 0; i--) {
				const int offset = STEP * (outDataIdx - i);
				for (int j = 0; j < bufferLength; j++) {
					outBuff[j] += mixRatio * outData[i][outIdx + offset + j];
				}
			}

			for (int i = outDataCount - 1; i > outDataIdx; i--) {
				const int offset = STEP * (outDataIdx + outDataCount - i);
				for (int j = 0; j < bufferLength; j++) {
					outBuff[j] += mixRatio * outData[i][outIdx + offset + j];
				}
			}
		}
		*/
		spc->outIdx += bufferLength;

		return;
	
	} else {
		// buffer will be filled this turn
		// we have to record only data to fill window
		// and fill te rest after copying to the process buffer
	
		// fill the circle buffer
		if (dataIdx + bufferLength + remDelta >= WND_SIZE) {

			for (int i = dataIdx; i < WND_SIZE; i++) {
				data[i] = inBuff[i - dataIdx];
			}

			int i = 0;
			for (; i < dataIdx + bufferLength + remDelta - WND_SIZE; i++) {
				data[i] = inBuff[WND_SIZE - dataIdx + i];
			}

			spc->dataIdx = i;

		} else {

			for (int i = 0; i < bufferLength + remDelta; i++) {
				data[dataIdx + i] = inBuff[i];
			}

			spc->dataIdx += bufferLength + remDelta;

		}

	}

	// fill the fft buffer
	{
		int j = 0;
		
		for (int i = spc->dataIdx; i < WND_SIZE; i++) {
			fftBuff[j] = data[i];
			j++;
		}

		for (int i = 0; i < spc->dataIdx; i++) {
			fftBuff[j] = data[i];
			j++;
		}

	}

	// record the rest of the input data
	if (spc->dataIdx - remDelta >= WND_SIZE) {

		for (int i = dataIdx; i < WND_SIZE; i++) {
			data[i] = inBuff[i - dataIdx];
		}

		int i = 0;
		for (; i < dataIdx - remDelta - WND_SIZE; i++) {
			data[i] = inBuff[WND_SIZE - dataIdx + i];
		}

		spc->dataIdx = i;

	} else {

		for (int i = 0; i < -remDelta; i++) {
			data[dataIdx + i] = inBuff[i];
		}

		spc->dataIdx -= remDelta;

	}




	// ANALYSIS STAGE
	//

	// window the signal
	for (int i = 0; i < WND_SIZE; i++) {
		fftBuff[i] *= wndFrame[i];
	}

	// zero fill the end
	for (int i = WND_SIZE; i < FFT_SIZE; i++) {
		fftBuff[i] = 0;
	}

	// compute fft
	fftFloat(fftBuff, fftBuff + FFT_SIZE, expWn, FFT_SIZE);

	// get mag
	mag[0] = sqrt(pow((fftBuff + FFT_SIZE)[0], 2) + 0);
	for (int i = 2; i < FFT_SIZE; i += 2) {
		mag[i / 2] = sqrt(pow((fftBuff + FFT_SIZE)[i], 2) + pow((fftBuff + FFT_SIZE)[i + 1], 2));
	}
	mag[FFT_SIZE / 2] = (fftBuff + FFT_SIZE)[1];

	// get phase
	phase[0] = atan2(0, (fftBuff + FFT_SIZE)[0]);
	for (int i = 2; i < FFT_SIZE; i += 2) {
		phase[i / 2] = atan2((fftBuff + FFT_SIZE)[i + 1], (fftBuff + FFT_SIZE)[i]);
	}
	phase[FFT_SIZE / 2] = atan2(0, (fftBuff + FFT_SIZE)[1]);



	// PROCESSING STAGE
	//

	if (firstIteration) {
		// just copy

		// newPhase = phase;
		memcpy(spc->newPhase, phase, sizeof(double) * (FFT_SIZE + 1));
		spc->firstIteration = 0;

	} else {
		// shift phase

		for (int i = 0; i < FFT_SIZE / 2 + 1; i++) {
			const double val = prevNewPhase[i] + prevPhaseDiff[i] * pitchRatio;
			newPhase[i] = val - round(val / (2 * M_PI)) * 2 * M_PI;
		}

	}

	// compute phase deviation to use in the next iteration
	for (int i = 0; i < FFT_SIZE / 2 + 1; i++) {
		const double val = phase[i] - prevPhase[i] - phaseBin[i];
		phaseDiff[i] = val - round(val / (2 * M_PI)) * 2 * M_PI + phaseBin[i];
	}



	// RESYNTHESIS STAGE
	//

	// real-synthese
	fftBuff[0] = mag[0] * cos(newPhase[0]);
	for (int i = 1; i < FFT_SIZE / 2; i++) {
		fftBuff[i] = fftBuff[FFT_SIZE - i] = mag[i] * cos(newPhase[i]);
	}
	fftBuff[FFT_SIZE / 2] = mag[FFT_SIZE / 2] * cos(newPhase[FFT_SIZE / 2]);

	// compute ifft using fft, as we are lazy to write ifft fcn for now
	// (1 / N) * (real(fft(Xr)) + imag(fft(Xi)))
	 
	double ifftOut[2 * FFT_SIZE];

	// compute real-fft
	fftFloat(fftBuff, fftBuff + FFT_SIZE, expWn, FFT_SIZE);
	
	// fill output buffer
	for (int i = 0; i < FFT_SIZE / 2; i++) {
		ifftOut[i] = ifftOut[FFT_SIZE - i] = (fftBuff + FFT_SIZE)[2 * i];
	}
	ifftOut[FFT_SIZE / 2] = (fftBuff + FFT_SIZE)[1];

	// imag-synthese
	fftBuff[0] = mag[0] * sin(newPhase[0]);
	for (int i = 0; i < FFT_SIZE / 2; i++) {
		const double val = mag[i] * sin(newPhase[i]);
		fftBuff[i] = val;
		fftBuff[FFT_SIZE - i] = -val;
	}
	fftBuff[FFT_SIZE / 2] = mag[FFT_SIZE / 2] * sin(newPhase[FFT_SIZE / 2]);

	// compute imag-fft
	fftFloat(fftBuff, fftBuff + FFT_SIZE, expWn, FFT_SIZE);

	// !!!!!!! ifftOut is 2x lower than suppose to be
	// add to output buffer and divide by N
	ifftOut[0] /= (double) FFT_SIZE;
	for (int i = 1; i < FFT_SIZE / 2; i++) {
		const double val = (fftBuff + FFT_SIZE)[2 * i + 1];
		ifftOut[i] = (ifftOut[i] + val) / (double) FFT_SIZE;
		ifftOut[FFT_SIZE - i] = (ifftOut[FFT_SIZE - i] - val) / FFT_SIZE;
	}
	ifftOut[FFT_SIZE / 2] /= (double) FFT_SIZE;

	// window
	for (int i = 0; i < FFT_SIZE; i++) {
		ifftOut[i] *= wndFFT[i];
	}

	const int actualOutDataIdx = (outDataIdx + 1 >= outDataCount) ? 0 : outDataIdx + 1;
	spc->outDataIdx = actualOutDataIdx;
	spc->outIdx = 0;

	// scale and interpolate, assuming only octaves
	if (octave == OCTAVE_ONE_DOWN) {
		// going down

		for (int i = 1; i < FFT_SIZE; i++) {

			const double y1 = ifftOut[i - 1];
			const double y2 = ifftOut[i];

			outData[actualOutDataIdx][2 * (i - 1)] = y1;
			outData[actualOutDataIdx][2 * (i - 1) + 1] = y1 + (y2 - y1) / 2;

		}

		// spc->outDataLen = 2 * FFT_SIZE;
	
	} else {
		// up

		const int step = (int) pitchRatio;

		for (int i = 0; i < FFT_SIZE / 2; i++) {
			outData[actualOutDataIdx][i] = ifftOut[2 * i];
		}

		// spc->outDataLen = FFT_SIZE / 2;

	}

	// swap buffer pointers
	SWAP_POINTERS(spc->phase, spc->prevPhase);
	SWAP_POINTERS(spc->newPhase, spc->prevNewPhase);
	SWAP_POINTERS(spc->phaseDiff, spc->prevPhaseDiff);

	// frame can be shorter than bufferLength!!!!!!
	// 	   !!!!!!
	// 	   !!!
	// TODO
	
	// pass the beginning to the output
	{
		const int outIdx = spc->outIdx;
		FILL_OUT_BUFFER(actualOutDataIdx, outIdx);
	}
	/*
	{
		const int outIdx = spc->outIdx;
		const double mixRatio = spc->outMixRatio;

		for (int i = actualOutDataIdx; i >= 0; i--) {
			const int offset = STEP * (actualOutDataIdx - i);
			for (int j = 0; j < bufferLength; j++) {
				outBuff[j] += mixRatio * outData[i][outIdx + offset + j];
			}
		}

		for (int i = outDataCount - 1; i > actualOutDataIdx; i--) {
			const int offset = STEP * (actualOutDataIdx + outDataCount - i);
			for (int j = 0; j < bufferLength; j++) {
				outBuff[j] += mixRatio * outData[i][outIdx + offset + j];
			}
		}
	}
	*/

	spc->outIdx += bufferLength;

}

void mixChange(PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	Space* const spc = (Space*) source->plugin->space;

	const double mixRatio = (source->value + 1) / 2;
	spc->outMixRatio = mixRatio;
	spc->inMixRatio = 1.0 - mixRatio;

	spc->mixRatio = source->value;

}

void octaveChange(PluginControl* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	Space* const spc = (Space*) source->plugin->space;
	
	// spc->outDataLen = (source->value == OCTAVE_ONE_UP) ? FFT_SIZE / 2 : FFT_SIZE * 2;
	spc->firstIteration = 1;
	spc->pitchRatio = (source->value == OCTAVE_ONE_UP) ? 2 : 0.5;
	spc->octave = source->value;

}

IPlugin* getPlugin() {
	
	IPlugin* plugin = (IPlugin*) malloc(sizeof(IPlugin));
	if (plugin == NULL) return NULL;

	// ui
	PluginUIHandler* uihnd = buildPluginUIHandler();
	uihnd->controls[0]->backgroundColor = 0xFF7952B3;

	PluginControl* mixKnob = addControl(uihnd, PCT_KNOB);
	mixKnob->color = TEXT_COLOR;
	mixKnob->MIN_VALUE = MIN_MIX;
	mixKnob->MAX_VALUE = MAX_MIX;
	mixKnob->value = DF_MIX;
	mixKnob->label = "DRY/WET";
	mixKnob->eChange = &mixChange;

	PluginControl* octaveKnob = addControl(uihnd, PCT_STEP_KNOB);
	octaveKnob->color = TEXT_COLOR;
	octaveKnob->MIN_VALUE = 0;
	octaveKnob->MAX_VALUE = 1;
	octaveKnob->step = 1;
	octaveKnob->minValue = 0;
	octaveKnob->maxValue = 1;
	octaveKnob->value = 1;
	octaveKnob->label = "Octave";
	octaveKnob->eChange = &octaveChange;

	// plugin itself
	plugin->name = L"Oktaver";
	*(PluginUIHandler**) &plugin->uihnd = uihnd;
	plugin->init = &init;
	plugin->process = &process;
	plugin->free = NULL;

	return plugin;

}
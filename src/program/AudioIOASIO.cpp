#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "AudioIOASIO.h"

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <mmsystem.h>
#include "ASIO/asiosys.h"
#include "ASIO/asio.h"
#include "ASIO/asiodrivers.h"
#include <cstdint>

#pragma comment(lib, "winmm")

#define MAX_UINT32 0xFFFFFFFF
#define MAX_UINT24 0x00FFFFFF

#define MAX_IN_BUFFERS 2
#define MAX_OUT_BUFFERS 2

#define BASIC_CLIP(in, out, m){\
	const double val = (in);\
	if (val >= 1) { out = (m); }\
	else if (val <= -1) { out = -(m); }\
	else { out = val * (m); }\
}

// number of input and outputs supported by the host application
enum {
	kMaxInputChannels = 2,
	kMaxOutputChannels = 2
};

// internal data storage
typedef struct DriverInfo {

	// ASIOInit()
	ASIODriverInfo driverInfo;

	// ASIOGetChannels()
	long           inputChannels;
	long           outputChannels;

	// ASIOGetBufferSize()
	long           minSize;
	long           maxSize;
	long           preferredSize;
	long           granularity;

	// ASIOGetSampleRate()
	ASIOSampleRate sampleRate;

	// ASIOOutputReady()
	bool           postOutput;

	// ASIOGetLatencies ()
	long           inputLatency;
	long           outputLatency;

	// ASIOCreateBuffers ()
	long inputBuffers;	// becomes number of actual created input buffers
	long outputBuffers;	// becomes number of actual created output buffers
	ASIOBufferInfo bufferInfos[kMaxInputChannels + kMaxOutputChannels]; // buffer info's

	// ASIOGetChannelInfo()
	ASIOChannelInfo channelInfos[kMaxInputChannels + kMaxOutputChannels]; // channel info's
	// The above two arrays share the same indexing, as the data in them are linked together

	// Information from ASIOGetSamplePosition()
	// data is converted to double floats for easier use, however 64 bit integer can be used, too
	double         nanoSeconds;
	double         samples;
	double         tcSamples;	// time code samples

	// bufferSwitchTimeInfo()
	ASIOTime       tInfo;			// time info state
	unsigned long  sysRefTime;      // system reference time, when bufferSwitch() was called

	// Signal the end of processing in this example
	bool           stopped;

} DriverInfo;

DriverInfo asioDriverInfo = { 0 };
ASIOCallbacks asioCallbacks;

int initialized = 0;

// meh, but anyways
int (*process) (void* inBuffer, void* outBuffer, int bufferLength);

int leftChannelIn = 0;
int rightChannelIn = 0;
int leftChannelOut = 0;
int rightChannelOut = 0;

// buffers to store input data
// [ [in buff 1] [in buff 2] ... [in buff n] [out buff 1] [out buff 2] ... [out buff m] ]
double* dataBuffers = NULL;
int dataBufferInCount = 0;
int dataBufferOutCount = 0;
int dataBufferLength = 0;

int allocDataBuffers(const int inCount, const int outCount, int length);
void freeASIO(AudioIOASIO* asio);

//----------------------------------------------------------------------------------
// some external references
extern AsioDrivers* asioDrivers;
bool loadAsioDriver(char* name);

// internal prototypes (required for the Metrowerks CodeWarrior compiler)
int main(int argc, char* argv[]);
long init_asio_static_data(DriverInfo* asioDriverInfo);
ASIOError create_asio_buffers(DriverInfo* asioDriverInfo);
unsigned long get_sys_reference_time();


// callback prototypes
void bufferSwitch(long index, ASIOBool processNow);
ASIOTime* bufferSwitchTimeInfo(ASIOTime* timeInfo, long index, ASIOBool processNow);
void sampleRateChanged(ASIOSampleRate sRate);
long asioMessages(long selector, long value, void* message, double* opt);

AudioIOASIO::AudioIOASIO() {

	process = processInput;

};

int AudioIOASIO::init(AudioDriver::DriverInfo* info) {

	char* driverName = info->device->name;
	setChannels(info);

	const ASIOSampleRate sampleRate = 48000.0; // double

	if (initialized) {

		ASIOStop();
		freeASIO(this);
		asioDrivers->removeCurrentDriver();
		
		initialized = 0;
	
	}

	// load the driver, this will setup all the necessary internal data structures
	if (loadAsioDriver(driverName)) {

		// initialize the driver
		if (ASIOInit(&asioDriverInfo.driverInfo) != ASE_OK) {
			asioDrivers->removeCurrentDriver();
			return 1;
		}

		initialized = 1;

		// set requaried params
		ASIOSetSampleRate(sampleRate);

		if (init_asio_static_data(&asioDriverInfo) != 0) {
			ASIOExit();
			return 2;
		}

		//ASIOControlPanel();// you might want to check wether the ASIOControlPanel() can open

		// set up the asioCallback structure and create the ASIO data buffer
		asioCallbacks.bufferSwitch = &bufferSwitch;
		asioCallbacks.sampleRateDidChange = &sampleRateChanged;
		asioCallbacks.asioMessage = &asioMessages;
		asioCallbacks.bufferSwitchTimeInfo = &bufferSwitchTimeInfo;

		// alloc internal data buffers to store and process asio input
		if (allocDataBuffers(2, 2, asioDriverInfo.preferredSize)) {
			ASIOExit();
			return 3;
		}

		// asign process input function to a global pointer, so it could be reachen by callbacks
		process = this->processInput;

		if (create_asio_buffers(&asioDriverInfo) != ASE_OK) {
			ASIOExit();
			return 4;
		}

	}

	AudioDriver::info.sampleRate = (int) asioDriverInfo.sampleRate;
	AudioDriver::info.maxBufferLength = (int) asioDriverInfo.preferredSize;

	return 0;

};

// 0 success start
// 1 error
int AudioIOASIO::start() {

	if (ASIOStart() == ASE_OK)
		return 0;
	
	return 1;

};

int AudioIOASIO::stop() {

	if (ASIOStop() == ASE_OK)
		return 0;

	return 1;

};

int AudioIOASIO::exit() {

	ASIOStop();
	return ASIOExit() == ASE_OK;

};

void AudioIOASIO::setChannels(AudioDriver::DriverInfo* info) {

	leftChannelIn = info->channelIn & AudioDriver::CHANNEL_1;
	rightChannelIn = (info->channelIn & AudioDriver::CHANNEL_2) >> 1;
	leftChannelOut = info->channelOut & AudioDriver::CHANNEL_1;
	rightChannelOut = (info->channelOut & AudioDriver::CHANNEL_2) >> 1;

}

void AudioIOASIO::openExternalConfig() {
	ASIOControlPanel();
}

AudioDriver::Device** AudioIOASIO::getDevices(int* deviceCount) {

	AsioDriverList driverList = AsioDriverList();
	LPASIODRVSTRUCT devicesList = driverList.lpdrvlist;
	const int driversNum = driverList.numdrv;

	AudioDriver::Device** devices = (AudioDriver::Device**) malloc(sizeof(AudioDriver::Device*) * driversNum);
	if (devices == NULL) return NULL;

	LPASIODRVSTRUCT device = devicesList;
	for (int i = 0; i < driversNum; i++) {
		
		devices[i] = (AudioDriver::Device*) malloc(sizeof(AudioDriver::Device));
		if (devices[i] == NULL) {
			*deviceCount = i;
			return devices;
		};

		devices[i]->name = (char*) malloc(sizeof(char) * strlen(device->drvname));
		if (devices[i]->name == NULL) {
			*deviceCount = i;
			return devices;
		}
		strcpy(devices[i]->name, device->drvname);
		devices[i]->id = device->drvID;
	
		device = devicesList->next;

	}

	*deviceCount = driversNum;

	return devices;

};

AudioIOASIO::~AudioIOASIO() {

	freeASIO(this);
	exit();

};

void freeASIO(AudioIOASIO* asio) {

	if (dataBufferInCount + dataBufferOutCount > 0) {
		free(dataBuffers);
	}

	dataBufferLength = 0;
	dataBufferInCount = 0;
	dataBufferOutCount = 0;

}

// all needed variables are in global space
int allocDataBuffers(const int inCount, const int outCount, const int length) {

	const int count = inCount + outCount;

	dataBuffers = (double*) malloc(count * length * sizeof(double)); // 2 for double buffering
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

//----------------------------------------------------------------------------------
long init_asio_static_data(DriverInfo* asioDriverInfo) {	
	// collect the informational data of the driver
	// get the number of available channels
	if (ASIOGetChannels(&asioDriverInfo->inputChannels, &asioDriverInfo->outputChannels) == ASE_OK) {

		// get the usable buffer sizes
		if (ASIOGetBufferSize(&asioDriverInfo->minSize, &asioDriverInfo->maxSize, &asioDriverInfo->preferredSize, &asioDriverInfo->granularity) == ASE_OK) {

			// get the currently selected sample rate
			if (ASIOGetSampleRate(&asioDriverInfo->sampleRate) == ASE_OK) {

				if (asioDriverInfo->sampleRate <= 0.0 || asioDriverInfo->sampleRate > 96000.0) {
					// Driver does not store it's internal sample rate, so set it to a know one.
					// Usually you should check beforehand, that the selected sample rate is valid
					// with ASIOCanSampleRate().
					if (ASIOSetSampleRate(44100.0) == ASE_OK) {
						if (ASIOGetSampleRate(&asioDriverInfo->sampleRate) == ASE_OK)
							printf("ASIOGetSampleRate (sampleRate: %f);\n", asioDriverInfo->sampleRate);
						else
							return -6;
					} else {
						return -5;
					}
				}

				// check wether the driver requires the ASIOOutputReady() optimization
				// (can be used by the driver to reduce output latency by one block)
				if (ASIOOutputReady() == ASE_OK)
					asioDriverInfo->postOutput = true;
				else
					asioDriverInfo->postOutput = false;
				printf("ASIOOutputReady(); - %s\n", asioDriverInfo->postOutput ? "Supported" : "Not supported");

				return 0;
			
			}
			
			return -3;
		
		}

		return -2;
	
	}

	return -1;

}


//----------------------------------------------------------------------------------
// conversion from 64 bit ASIOSample/ASIOTimeStamp to double float
#if NATIVE_INT64
#define ASIO64toDouble(a)  (a)
#else
const double twoRaisedTo32 = 4294967296.;
#define ASIO64toDouble(a)  ((a).lo + (a).hi * twoRaisedTo32)
#endif

ASIOTime* bufferSwitchTimeInfo(ASIOTime* timeInfo, long index, ASIOBool processNow)
{	// the actual processing callback.
	// Beware that this is normally in a seperate thread, hence be sure that you take care
	// about thread synchronization. This is omitted here for simplicity.
	static long processedSamples = 0;

	// store the timeInfo for later use
	asioDriverInfo.tInfo = *timeInfo;

	// get the time stamp of the buffer, not necessary if no
	// synchronization to other media is required
	if (timeInfo->timeInfo.flags & kSystemTimeValid)
		asioDriverInfo.nanoSeconds = ASIO64toDouble(timeInfo->timeInfo.systemTime);
	else
		asioDriverInfo.nanoSeconds = 0;

	if (timeInfo->timeInfo.flags & kSamplePositionValid)
		asioDriverInfo.samples = ASIO64toDouble(timeInfo->timeInfo.samplePosition);
	else
		asioDriverInfo.samples = 0;

	if (timeInfo->timeCode.flags & kTcValid)
		asioDriverInfo.tcSamples = ASIO64toDouble(timeInfo->timeCode.timeCodeSamples);
	else
		asioDriverInfo.tcSamples = 0;

	// get the system reference time
	asioDriverInfo.sysRefTime = get_sys_reference_time();

	// buffer size in samples
	const long buffSize = asioDriverInfo.preferredSize;
	
	// count of input buffers, they go first
	const int inBuffCount = (MAX_IN_BUFFERS < asioDriverInfo.inputBuffers) ? MAX_IN_BUFFERS : asioDriverInfo.inputBuffers;

	// count of output buffers, they go after input ones
	const int outBuffCount = (MAX_OUT_BUFFERS < asioDriverInfo.outputBuffers) ? MAX_OUT_BUFFERS : asioDriverInfo.inputBuffers;

	// get input
	int i = 0;
	int wasAssigned = 0;
	double* const dataBuffer = dataBuffers;// = dataBuffers + dataBufferLength * (1 - index);
	for (i = 0; i < inBuffCount; i++) {

		switch (asioDriverInfo.channelInfos[i].type) {
			// here only process supported types
			// assuming that at least one buffer exists
			// all input will bi summed into one

			case ASIOSTInt32LSB: {

				if ((i == 0 && !leftChannelIn) || (i == 1 && !rightChannelIn)) break;
				//if (i >= dataBufferCount) break;

				const int* inBuffer = (int*) (asioDriverInfo.bufferInfos[i].buffers[index]);

				if (!wasAssigned) {

					for (int i = 0; i < buffSize; i++) {
						dataBuffer[i] = (double) (inBuffer[i] / (double) INT_MAX);
					}
					wasAssigned = 1;

				} else {

					for (int i = 0; i < buffSize; i++) {
						dataBuffer[i] += (double) (inBuffer[i] / (double) INT_MAX);
					}
				
				}

				break;

			}

		}

	}

	// process
	// const int outOffset = inOffset + (dataBufferInCount * 2) * dataBufferLength;

	double* const inBuff = dataBuffer;
	double* const outBuff = dataBuffer + dataBufferInCount * dataBufferLength;

	const int mix = process((void*) inBuff, (void*) outBuff, dataBufferLength);

	// pass to output
	int computedIdx = -1;
	for (i; i < inBuffCount + outBuffCount; i++) {
		// output buffers processing
		// assuming only max 2 buffers, so maybe has to be rewriten

		switch (asioDriverInfo.channelInfos[i].type) {

			case ASIOSTInt16LSB: {

				memset(asioDriverInfo.bufferInfos[i].buffers[index], 0, buffSize * 2);
				break;
			
			}

			case ASIOSTInt24LSB: {		// used for 20 bits as well
				
				memset(asioDriverInfo.bufferInfos[i].buffers[index], 0, buffSize * 3);
				break;
			
			}

			case ASIOSTInt32LSB: {

				int32_t* const outBuffer = (int*) (asioDriverInfo.bufferInfos[i].buffers[index]);

				if ((i == inBuffCount && !leftChannelOut) || (i == inBuffCount + 1 && !rightChannelOut)) {
					
					memset(outBuffer, 0, buffSize * 4);
					break;

				}

				if (computedIdx >= 0) {

					int32_t* const outBufferComputed = (int*) (asioDriverInfo.bufferInfos[computedIdx].buffers[index]);
					if (mix) {
						double* const buff = outBuff + dataBufferLength;
						for (int j = 0; j < buffSize; j++) {
							BASIC_CLIP(buff[j], outBuffer[j], INT_MAX);
						}
					} else {
						memcpy(outBuffer, outBufferComputed, 4 * buffSize);
					}
				
				} else {
				
					for (int j = 0; j < buffSize; j++) {
						BASIC_CLIP(outBuff[j], outBuffer[j], INT_MAX);
					}

					computedIdx = i;
				
				}
				
				break;
			}

			case ASIOSTFloat32LSB: {	
				// IEEE 754 32 bit float, as found on Intel x86 architecture

				memset(asioDriverInfo.bufferInfos[i].buffers[index], 0, buffSize * 4);
				break;
			
			}

			case ASIOSTFloat64LSB: {
				// IEEE 754 64 bit double float, as found on Intel x86 architecture

				memset(asioDriverInfo.bufferInfos[i].buffers[index], 0, buffSize * 8);
				break;
			
			}

				// these are used for 32 bit data buffer, with different alignment of the data inside
				// 32 bit PCI bus systems can more easily used with these
			case ASIOSTInt32LSB16:		// 32 bit data with 18 bit alignment
			case ASIOSTInt32LSB18:		// 32 bit data with 18 bit alignment
			case ASIOSTInt32LSB20:		// 32 bit data with 20 bit alignment
			case ASIOSTInt32LSB24: {
				// 32 bit data with 24 bit alignment

				memset(asioDriverInfo.bufferInfos[i].buffers[index], 0, buffSize * 4);
				break;
			
			}

			case ASIOSTInt16MSB: {

				memset(asioDriverInfo.bufferInfos[i].buffers[index], 0, buffSize * 2);
				break;

			}

			case ASIOSTInt24MSB: {
				// used for 20 bits as well

				memset(asioDriverInfo.bufferInfos[i].buffers[index], 0, buffSize * 3);
				break;

			}

			case ASIOSTInt32MSB: {

				memset(asioDriverInfo.bufferInfos[i].buffers[index], 0, buffSize * 4);
				break;

			}

			case ASIOSTFloat32MSB: {
				// IEEE 754 32 bit float, as found on Intel x86 architecture
				
				memset(asioDriverInfo.bufferInfos[i].buffers[index], 0, buffSize * 4);
				break;
			
			}
			
			case ASIOSTFloat64MSB: {
				// IEEE 754 64 bit double float, as found on Intel x86 architecture

				memset(asioDriverInfo.bufferInfos[i].buffers[index], 0, buffSize * 8);
				break;
			
			}

				// these are used for 32 bit data buffer, with different alignment of the data inside
				// 32 bit PCI bus systems can more easily used with these
			case ASIOSTInt32MSB16:		// 32 bit data with 18 bit alignment
			case ASIOSTInt32MSB18:		// 32 bit data with 18 bit alignment
			case ASIOSTInt32MSB20:		// 32 bit data with 20 bit alignment
			case ASIOSTInt32MSB24: {
				// 32 bit data with 24 bit alignment

				memset(asioDriverInfo.bufferInfos[i].buffers[index], 0, buffSize * 4);
				break;
			
			}
		}

	}

	// finally if the driver supports the ASIOOutputReady() optimization, do it here, all data are in place
	if (asioDriverInfo.postOutput)
		ASIOOutputReady();

	return 0L;
}

//----------------------------------------------------------------------------------
void bufferSwitch(long index, ASIOBool processNow)
{	// the actual processing callback.
	// Beware that this is normally in a seperate thread, hence be sure that you take care
	// about thread synchronization. This is omitted here for simplicity.

	// as this is a "back door" into the bufferSwitchTimeInfo a timeInfo needs to be created
	// though it will only set the timeInfo.samplePosition and timeInfo.systemTime fields and the according flags
	ASIOTime  timeInfo;
	memset(&timeInfo, 0, sizeof(timeInfo));

	// get the time stamp of the buffer, not necessary if no
	// synchronization to other media is required
	if (ASIOGetSamplePosition(&timeInfo.timeInfo.samplePosition, &timeInfo.timeInfo.systemTime) == ASE_OK)
		timeInfo.timeInfo.flags = kSystemTimeValid | kSamplePositionValid;

	bufferSwitchTimeInfo(&timeInfo, index, processNow);
}


//----------------------------------------------------------------------------------
void sampleRateChanged(ASIOSampleRate sRate)
{
	// do whatever you need to do if the sample rate changed
	// usually this only happens during external sync.
	// Audio processing is not stopped by the driver, actual sample rate
	// might not have even changed, maybe only the sample rate status of an
	// AES/EBU or S/PDIF digital input at the audio device.
	// You might have to update time/sample related conversion routines, etc.

	int x = 0;
	int y = x + 2;
}

//----------------------------------------------------------------------------------
long asioMessages(long selector, long value, void* message, double* opt)
{
	// currently the parameters "value", "message" and "opt" are not used.
	long ret = 0;
	switch (selector)
	{
	case kAsioSelectorSupported:
		if (value == kAsioResetRequest
			|| value == kAsioEngineVersion
			|| value == kAsioResyncRequest
			|| value == kAsioLatenciesChanged
			// the following three were added for ASIO 2.0, you don't necessarily have to support them
			|| value == kAsioSupportsTimeInfo
			|| value == kAsioSupportsTimeCode
			|| value == kAsioSupportsInputMonitor)
			ret = 1L;
		break;
	case kAsioResetRequest:
		// defer the task and perform the reset of the driver during the next "safe" situation
		// You cannot reset the driver right now, as this code is called from the driver.
		// Reset the driver is done by completely destruct is. I.e. ASIOStop(), ASIODisposeBuffers(), Destruction
		// Afterwards you initialize the driver again.
		asioDriverInfo.stopped;  // In this sample the processing will just stop
		ret = 1L;
		break;
	case kAsioResyncRequest:
		// This informs the application, that the driver encountered some non fatal data loss.
		// It is used for synchronization purposes of different media.
		// Added mainly to work around the Win16Mutex problems in Windows 95/98 with the
		// Windows Multimedia system, which could loose data because the Mutex was hold too long
		// by another thread.
		// However a driver can issue it in other situations, too.
		ret = 1L;
		break;
	case kAsioLatenciesChanged:
		// This will inform the host application that the drivers were latencies changed.
		// Beware, it this does not mean that the buffer sizes have changed!
		// You might need to update internal delay data.
		ret = 1L;
		break;
	case kAsioEngineVersion:
		// return the supported ASIO version of the host application
		// If a host applications does not implement this selector, ASIO 1.0 is assumed
		// by the driver
		ret = 2L;
		break;
	case kAsioSupportsTimeInfo:
		// informs the driver wether the asioCallbacks.bufferSwitchTimeInfo() callback
		// is supported.
		// For compatibility with ASIO 1.0 drivers the host application should always support
		// the "old" bufferSwitch method, too.
		ret = 1;
		break;
	case kAsioSupportsTimeCode:
		// informs the driver wether application is interested in time code info.
		// If an application does not need to know about time code, the driver has less work
		// to do.
		ret = 0;
		break;
	}
	return ret;
}


//----------------------------------------------------------------------------------
ASIOError create_asio_buffers(DriverInfo* asioDriverInfo)
{	// create buffers for all inputs and outputs of the card with the 
	// preferredSize from ASIOGetBufferSize() as buffer size
	long i;
	ASIOError result;

	// fill the bufferInfos from the start without a gap
	ASIOBufferInfo* info = asioDriverInfo->bufferInfos;

	// prepare inputs (Though this is not necessaily required, no opened inputs will work, too
	if (asioDriverInfo->inputChannels > kMaxInputChannels)
		asioDriverInfo->inputBuffers = kMaxInputChannels;
	else
		asioDriverInfo->inputBuffers = asioDriverInfo->inputChannels;
	for (i = 0; i < asioDriverInfo->inputBuffers; i++, info++)
	{
		info->isInput = ASIOTrue;
		info->channelNum = i;
		info->buffers[0] = info->buffers[1] = 0;
	}

	// prepare outputs
	if (asioDriverInfo->outputChannels > kMaxOutputChannels)
		asioDriverInfo->outputBuffers = kMaxOutputChannels;
	else
		asioDriverInfo->outputBuffers = asioDriverInfo->outputChannels;
	for (i = 0; i < asioDriverInfo->outputBuffers; i++, info++)
	{
		info->isInput = ASIOFalse;
		info->channelNum = i;
		info->buffers[0] = info->buffers[1] = 0;
	}

	// create and activate buffers
	result = ASIOCreateBuffers(asioDriverInfo->bufferInfos,
		asioDriverInfo->inputBuffers + asioDriverInfo->outputBuffers,
		asioDriverInfo->preferredSize, &asioCallbacks);
	if (result == ASE_OK)
	{
		// now get all the buffer details, sample word length, name, word clock group and activation
		for (i = 0; i < asioDriverInfo->inputBuffers + asioDriverInfo->outputBuffers; i++)
		{
			asioDriverInfo->channelInfos[i].channel = asioDriverInfo->bufferInfos[i].channelNum;
			asioDriverInfo->channelInfos[i].isInput = asioDriverInfo->bufferInfos[i].isInput;
			result = ASIOGetChannelInfo(&asioDriverInfo->channelInfos[i]);
			if (result != ASE_OK)
				break;
		}

		if (result == ASE_OK)
		{
			// get the input and output latencies
			// Latencies often are only valid after ASIOCreateBuffers()
			// (input latency is the age of the first sample in the currently returned audio block)
			// (output latency is the time the first sample in the currently returned audio block requires to get to the output)
			result = ASIOGetLatencies(&asioDriverInfo->inputLatency, &asioDriverInfo->outputLatency);
			if (result == ASE_OK)
				printf("ASIOGetLatencies (input: %d, output: %d);\n", asioDriverInfo->inputLatency, asioDriverInfo->outputLatency);
		}
	}
	return result;
}

unsigned long get_sys_reference_time()
{	// get the system reference time
#if WINDOWS
	return timeGetTime();
#elif MAC
	static const double twoRaisedTo32 = 4294967296.;
	UnsignedWide ys;
	Microseconds(&ys);
	double r = ((double)ys.hi * twoRaisedTo32 + (double)ys.lo);
	return (unsigned long)(r / 1000.);
#endif
}

#include "ps3mic.h"
#include <assert.h>

void PS3EYEMic::init(libusb_device * _device, const AudioCallback * _audioCallback)
{
	assert(device == nullptr);
	assert(audioCallback == nullptr);

	device = _device;
	audioCallback = _audioCallback;

	// todo : find and claim interface

	// todo : on Macos and Windows : suggest to unload kernel driver when interface claim fails
}

void PS3EYEMic::shut()
{

}

void PS3EYEMic::beginTransfers(const int packetSize, const int numPackets, const int numTransfers)
{

}

void PS3EYEMic::cancelTransfersBegin()
{

}

void PS3EYEMic::cancelTransfersWait()
{

}

void PS3EYEMic::freeTransfers()
{
	
}

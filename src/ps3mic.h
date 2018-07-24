/*

PS3 eye routines for capturing audio input from the four-channel microphone array.

Support and donate at,
https://www.patreon.com/marcelsmit

Code origionally written by Marcel Smit,
Author of the Framework creative coding library,
https://github.com/marcel303/framework

Donated to the community to hack away with.

If you want to support my efforts building a new creative coding library,
an audio-visual patching system, tutorials and examples and ofcourse
projects like these, please become a patron. :-)

*/

#pragma once

#include <atomic>
#include <vector>

struct libusb_device;
struct libusb_device_handle;
struct libusb_transfer;

namespace ps3eye {

struct AudioFrame
{
	int16_t channel[4];
};

struct AudioCallback
{
	// numSamples = numFrames * 4, as the PS3 eye contains a four-channel microphone array

	virtual void handleAudioData(const AudioFrame * frames, const int numFrames) = 0;
};

struct PS3EYEMic
{
	libusb_device * device = nullptr;
	libusb_device_handle * deviceHandle = nullptr;

	AudioCallback * audioCallback = nullptr;
	
	uint8_t * transferData = nullptr;
	
	std::vector<libusb_transfer*> transfers;
	std::atomic<int> numActiveTransfers;
	std::atomic<bool> endTransfers;

	PS3EYEMic();
	~PS3EYEMic();

	bool init(libusb_device * device, AudioCallback * audioCallback);
	bool initImpl(libusb_device * device, AudioCallback * audioCallback);
	void shut();

	bool beginTransfers(const int packetSize, const int numPackets, const int numTransfers);
	void endTransfersBegin();
	void endTransfersWait();
	void freeTransfers();
};

}

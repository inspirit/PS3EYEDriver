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

todo : 48kHz sampling rate
	Marketing materials, Wikipedia etc all consistently say the PS3 Eye camera supports a 48kHz sampling rate. However, the USB audio class descriptor only mentions support for up to 16kHz, and this is what I get. Perhaps it's possible to switch modes? There's a mysterious vendor specific interface in the USB interface descriptor list. Perhaps this is the key?

*/

#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
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

class PS3EYEMic
{
private:
	libusb_device * device = nullptr;
	libusb_device_handle * deviceHandle = nullptr;
	bool started = false;

	AudioCallback * audioCallback = nullptr;
	
	uint8_t * transferData = nullptr;
	
	std::vector<libusb_transfer*> transfers;
	int numActiveTransfers = 0;
	std::mutex numActiveTransfersMutex;
	std::condition_variable numActiveTransfersCondition;
	std::atomic<bool> endTransfers;
	
	static void handleTransfer(struct libusb_transfer * transfer);

public:
	static const int kSampleRate;
	
	PS3EYEMic();
	~PS3EYEMic();

	bool init(libusb_device * device, AudioCallback * audioCallback); // Initialize and begin capturing microphone data
	void shut(); // End capturing microphone data and shutdown
	
	bool getIsInitialized() const;
	
private:
	bool initImpl(libusb_device * device, AudioCallback * audioCallback);
	void shutImpl();

	bool beginTransfers(const int packetSize, const int numPackets, const int numTransfers);
	void endTransfersBegin();
	void endTransfersWait();
	void freeTransfers();
};

}

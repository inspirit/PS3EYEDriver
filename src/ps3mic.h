#pragma once

#include <atomic>
#include <vector>

struct libusb_device;
struct libusb_device_handle;
struct libusb_transfer;

struct PS3EYEMic
{
	struct AudioCallback
	{
		// numSamples = numFrames * 4, as the PS3 eye contains a microphone array of four capsules

		virtual void handleAudioData(const int16_t * frames, const int numFrames) = 0;
	};

	libusb_device * device = nullptr;
	libusb_device_handle * deviceHandle = nullptr;

	AudioCallback * audioCallback = nullptr;
	
	uint8_t * transferData = nullptr;
	
	std::vector<libusb_transfer*> transfers;
	std::atomic<int> numActiveTransfers;
	std::atomic<bool> cancelTransfers;

	PS3EYEMic();
	~PS3EYEMic();

	bool init(libusb_device * device, AudioCallback * audioCallback);
	bool initImpl(libusb_device * device, AudioCallback * audioCallback);
	void shut();

	bool beginTransfers(const int packetSize, const int numPackets, const int numTransfers);
	void cancelTransfersBegin();
	void cancelTransfersWait();
	void freeTransfers();
};

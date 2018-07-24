#pragma once

struct libusb_device;
struct libusb_device_handle;

struct PS3EYEMic
{
	struct AudioCallback
	{
		// numSamples = numFrames * 4, as the PS3 eye contains a microphone array of four capsules

		virtual void handleAudioData(const int16_t * frames, const int numFrames) = 0;
	};

	libusb_device * device = nullptr;
	libusb_device_handle * deviceHandle = nullptr;

	const AudioCallback * audioCallback = nullptr;

	PS3EYEMic();
	~PS3EYEMic();

	bool init(libusb_device * device, const AudioCallback * audioCallback);
	void shut();

	void beginTransfers(const int packetSize, const int numPackets, const int numTransfers);
	void cancelTransfersBegin();
	void cancelTransfersWait();
	void freeTransfers();
};

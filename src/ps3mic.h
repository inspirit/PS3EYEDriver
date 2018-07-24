#pragma once

struct PS3EYEMic
{
	struct AudioCallback
	{
		// numSamples = numFrames * 4, as the PS3 eye contains a microphone array of four capsules

		virtual void handleAudioData(const int16_t * frames, const int numFrames);
	};

	libusb_device * device = nullptr;
	libusb_device_handle * deviceHandle = nullptr

	AudioCallback * audioCallback = nullptr;

	PS3EYEMic();
	~PS3EYEMic();

	void init(libusb_device * device, const AudioCallback * audioCallback);
	void shut();

	void beginTransfers(const int packetSize, const int numPackets, const int numTransfers);
	void cancelTransfersBegin();
	void cancelTransfersWait();
	void freeTransfers();
};

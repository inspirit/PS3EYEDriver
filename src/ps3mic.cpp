#include "ps3eye.h" // for vendor and product IDs
#include "ps3mic.h"

#include "libusb.h"

#include <assert.h>
#include <stdio.h>

/*

The PS3 eye camera exposes its microphone array as a regular USB audio device with four channels

Each USB device can expose one or more 'interface'. The PS3 eye camera exposes its video
capabilities on interface 0. Its USB audio interface is exposed on interface 2 (INTERFACE_NUMBER).

We set the audio interface to alt setting 1, which means 'mono' output, which actually means to
output one or more mono streams. For reference: alt setting 0 is energy savings mode (disabled) and
alt setting 2 is stereo mode, which always produces a stream of stereo frames, even if the mic
only has one capsule.

*/

#if defined(DEBUG)
	#define debug(...) do { fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n"); } while (false)
#else
	#define debug(...)
#endif

#define INTERFACE_NUMBER 2

// for streaming we set up a bunch of transfers, which will each transfer a number of packets of a certain size
// I have no idea what the ideal values are here.. I guess there's a sweet spot between still having robust transfers
// while still maintaining an acceptable latency.
// todo : find good values for these numbers
#define PACKET_SIZE 128
#define NUM_PACKETS 8
#define NUM_TRANSFERS 2

PS3EYEMic::PS3EYEMic()
{
}

PS3EYEMic::~PS3EYEMic()
{
}

bool PS3EYEMic::init(libusb_device * _device, const AudioCallback * _audioCallback)
{
	assert(device == nullptr);
	assert(audioCallback == nullptr);

	device = _device;
	audioCallback = _audioCallback;

	// open the USB device

	int res = libusb_open(device, &deviceHandle);
	if (res != 0)
	{
		debug("device open error: %d: %s", res, libusb_error_name(res));
		return false;
	}

	res = libusb_set_configuration(deviceHandle, 1);
	if (res != 0)
	{
		debug("failed to set device configuration: %d: %s", res, libusb_error_name(res));
		return false;
	}

	// before we claim the interface, we must first detach any kernel drivers that may use it

    if (libusb_kernel_driver_active(deviceHandle, INTERFACE_NUMBER))
    {
        res = libusb_detach_kernel_driver(deviceHandle, INTERFACE_NUMBER);
        if (res < 0)
        {
            debug("failed to detach kernel driver: %d: %s", res, libusb_error_name(res));
            return false;
        }
    }

	// claim interface

	res = libusb_claim_interface(deviceHandle, INTERFACE_NUMBER);
	if (res < 0)
	{
		// todo : on Macos and Windows : suggest to unload kernel driver when interface claim fails

		debug("failed to claim interface: %d: %s", res, libusb_error_name(res));
        return false;
    }

    // set the interface alt mode. 1 = (multi-channel) mono

	res = libusb_set_interface_alt_setting(deviceHandle, INTERFACE_NUMBER, 1);
	if (res < 0)
	{
		debug("failed to set interface alt setting: %d: %sn", res, libusb_error_name(res));
        return false;
	}
	
	beginTransfers(PACKET_SIZE, NUM_PACKETS, NUM_TRANSFERS);

	return true;
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

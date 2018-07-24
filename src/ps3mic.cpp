#include "ps3eye.h" // for vendor and product IDs
#include "ps3mic.h"
#include "libusb.h"
#include <assert.h>

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
	#include <stdio.h>
	#define debugMessage(...) do { fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n"); } while (false)
#else
	#define debugMessage(...)
#endif

namespace ps3eye {

static const int INTERFACE_NUMBER = 2;

static const int ISO_ENDPOINT_ADDRESS = 0x84;

/*
For streaming we set up a bunch of transfers, which will each transfer a number of packets of a certain size. I have no idea what the ideal values are here. I guess there's a sweet spot between still having robust transfers while still maintaining an acceptable latency.

todo : find good values for these numbers
*/
static const int PACKET_SIZE = 512;
static const int NUM_PACKETS = 2;
static const int NUM_TRANSFERS = 2;

//

extern void micStarted();
extern void micStopped();

//

void PS3EYEMic::handleTransfer(struct libusb_transfer * transfer)
{
	PS3EYEMic * mic = (PS3EYEMic*)transfer->user_data;
	
	assert(mic->numActiveTransfers > 0);
	
	bool endTransfer = false;
	
	if (mic->endTransfers)
	{
		endTransfer = true;
	}
	else if (transfer->status == LIBUSB_TRANSFER_COMPLETED)
	{
		for (int i = 0; i < transfer->num_iso_packets; ++i)
		{
			const libusb_iso_packet_descriptor & packet = transfer->iso_packet_desc[i];
		
			if (packet.status != LIBUSB_TRANSFER_COMPLETED)
			{
				debugMessage("packet status != LIBUSB_TRANSFER_COMPLETED");
				continue;
			}
			
			const int frameSize = 4 * sizeof(int16_t);
			
			const int numBytes = packet.actual_length;
			assert((numBytes % frameSize) == 0);
			const int numFrames = numBytes / frameSize;
			
			if (numFrames > 0)
			{
				const AudioFrame * frames = (AudioFrame*)libusb_get_iso_packet_buffer_simple(transfer, i);
				
				mic->audioCallback->handleAudioData(frames, numFrames);
			}
		}
		
		const int res = libusb_submit_transfer(transfer);
		
		if (res < 0)
		{
			debugMessage("failed to submit transfer: %d: %s", res, libusb_error_name(res));
			endTransfer = true;
		}
	}
	else
	{
		debugMessage("transfer ended but status is not COMPLETED: status=%d", transfer->status);
		
		const int res = libusb_submit_transfer(transfer);
		
		if (res < 0)
		{
			debugMessage("failed to submit transfer: %d: %s", res, libusb_error_name(res));
			endTransfer = true;
		}
	}
	
	if (endTransfer)
	{
		std::unique_lock<std::mutex> lock(mic->numActiveTransfersMutex);
		mic->numActiveTransfers--;
		mic->numActiveTransfersCondition.notify_one();
	}
}

PS3EYEMic::PS3EYEMic()
	: endTransfers(false)
{
}

PS3EYEMic::~PS3EYEMic()
{
}

bool PS3EYEMic::init(libusb_device * device, AudioCallback * audioCallback)
{
	const bool result = initImpl(device, audioCallback);
	
	if (result == false)
		shut();
	
	return result;
}

void PS3EYEMic::shut()
{
	shutImpl();
}

bool PS3EYEMic::initImpl(libusb_device * _device, AudioCallback * _audioCallback)
{
	assert(device == nullptr);
	assert(audioCallback == nullptr);

	device = _device;
	audioCallback = _audioCallback;
	
	assert(device != nullptr);
	assert(audioCallback != nullptr);

	// open the USB device

	int res;
	
	res = libusb_open(device, &deviceHandle);
	if (res != LIBUSB_SUCCESS)
	{
		debugMessage("device open error: %d: %s", res, libusb_error_name(res));
		return false;
	}

	res = libusb_set_configuration(deviceHandle, 1);
	if (res != LIBUSB_SUCCESS)
	{
		debugMessage("failed to set device configuration: %d: %s", res, libusb_error_name(res));
		return false;
	}

	// before we claim the interface, we must first detach any kernel drivers that may use it

    if (libusb_kernel_driver_active(deviceHandle, INTERFACE_NUMBER))
    {
        res = libusb_detach_kernel_driver(deviceHandle, INTERFACE_NUMBER);
        if (res != LIBUSB_SUCCESS)
        {
            debugMessage("failed to detach kernel driver: %d: %s", res, libusb_error_name(res));
            return false;
        }
    }

	// claim interface

	res = libusb_claim_interface(deviceHandle, INTERFACE_NUMBER);
	if (res != LIBUSB_SUCCESS)
	{
	#ifdef __APPLE__
		printf(
			"-- Failed to claim interface. On Macos this may be due to the OS already having loaded the Apple USB Audio kernel driver for your Eye camera. LibUSB cannot unload this driver for us, so you'll have to run the following command from the terminal to unload it manually. Note this will kill audio to/from ALL USB audio devices!!:\n"
			"sudo kextunload -b com.apple.driver.AppleUSBAudio\n"
			"--\n"
			);
	#endif
		// todo : add Windows help text here in case claiming the interface fails

		debugMessage("failed to claim interface: %d: %s", res, libusb_error_name(res));
        return false;
    }

    // set the interface alt mode. 1 = (multi-channel) mono

	res = libusb_set_interface_alt_setting(deviceHandle, INTERFACE_NUMBER, 1);
	if (res != LIBUSB_SUCCESS)
	{
		debugMessage("failed to set interface alt setting: %d: %sn", res, libusb_error_name(res));
        return false;
	}
	
	//
	
	micStarted();
	started = true;
	
	//
	
	if (!beginTransfers(PACKET_SIZE, NUM_PACKETS, NUM_TRANSFERS))
	{
		return false;
	}

	return true;
}

void PS3EYEMic::shutImpl()
{
	if (numActiveTransfers > 0)
	{
		endTransfersBegin();
		
		endTransfersWait();
	}
	
	freeTransfers();
	
	//
	
	if (started)
	{
		micStopped();
		started = false;
	}
	
	//
	
	if (deviceHandle != nullptr)
	{
		int res = libusb_release_interface(deviceHandle, INTERFACE_NUMBER);
		if (res != LIBUSB_SUCCESS)
			debugMessage("failed to release interface. %d: %s", res, libusb_error_name(res));
		
		res = libusb_attach_kernel_driver(deviceHandle, INTERFACE_NUMBER);
		if (res != LIBUSB_SUCCESS)
			debugMessage("failed to attach kernel driver for interface. %d: %s", res, libusb_error_name(res));
		
		libusb_close(deviceHandle);
		deviceHandle = nullptr;
	}
	
	//
	
	device = nullptr;
	audioCallback = nullptr;
}

bool PS3EYEMic::beginTransfers(const int packetSize, const int numPackets, const int numTransfers)
{
	const int transferSize = packetSize * numPackets;
	
	assert(transferData == nullptr);
	transferData = new uint8_t[transferSize];
	
	//
	
	assert(transfers.empty());
	transfers.resize(numTransfers);
	
	for (int i = 0; i < numTransfers; ++i)
	{
		auto & transfer = transfers[i];
		
		transfer = libusb_alloc_transfer(numPackets);
		
		if (transfer == nullptr)
		{
			debugMessage("failed to allocate transfer");
			return false;
		}
		
		libusb_fill_iso_transfer(
			transfer,
			deviceHandle,
			ISO_ENDPOINT_ADDRESS,
			transferData,
			transferSize,
			numPackets,
			handleTransfer,
			this,
			1000);
		
		libusb_set_iso_packet_lengths(transfer, packetSize);
		
		const int res = libusb_submit_transfer(transfer);
		
		if (res != LIBUSB_SUCCESS)
		{
			debugMessage("failed to submit transfer. %d: %s", res, libusb_error_name(res));
			return false;
		}
		
		std::unique_lock<std::mutex> lock(numActiveTransfersMutex);
		numActiveTransfers++;
	}
	
	return true;
}

void PS3EYEMic::endTransfersBegin()
{
	endTransfers = true;
}

void PS3EYEMic::endTransfersWait()
{
	std::unique_lock<std::mutex> lock(numActiveTransfersMutex);
	numActiveTransfersCondition.wait(lock, [this]() { return numActiveTransfers == 0; });
	
	endTransfers = false;
}

void PS3EYEMic::freeTransfers()
{
	for (auto & transfer : transfers)
	{
		libusb_free_transfer(transfer);
		transfer = nullptr;
	}
	
	transfers.clear();
	
	//
	
	delete [] transferData;
	transferData = nullptr;
}

#undef debugMessage

}

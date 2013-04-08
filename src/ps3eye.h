#ifndef PS3EYECAM_H
#define PS3EYECAM_H

#include <cstdio>
#include <cstdlib>
#include <vector>

// define shared_ptr in std 

#if defined( _MSC_VER ) && ( _MSC_VER >= 1600 )
    #include <memory>
#else
    #include <tr1/memory>
    namespace std {
        using std::tr1::shared_ptr;
        using std::tr1::weak_ptr;        
        using std::tr1::static_pointer_cast;
        using std::tr1::dynamic_pointer_cast;
        using std::tr1::const_pointer_cast;
        using std::tr1::enable_shared_from_this;
    }
#endif

#include "libusb.h"

#ifndef __STDC_CONSTANT_MACROS
#  define __STDC_CONSTANT_MACROS
#endif

#include <stdint.h>

#if defined(DEBUG)
#define debug(x...) fprintf(stdout,x)
#else
#define debug(x...) 
#endif


namespace ps3eye {

class PS3EYECam
{
public:
	typedef std::shared_ptr<PS3EYECam> PS3EYERef;

	static const uint16_t VENDOR_ID;
	static const uint16_t PRODUCT_ID;

	PS3EYECam(libusb_device *device);
	~PS3EYECam();

	bool init(uint32_t width = 0, uint32_t height = 0, uint8_t desiredFrameRate = 30);
	void start();
	void stop();

	// Controls

	bool getAutogain() const { return autogain; }
	void setAutogain(bool val) {
	    autogain = val;
	    if (val) {
			sccb_reg_write(0x13, 0xf7); //AGC,AEC,AWB ON
			sccb_reg_write(0x64, sccb_reg_read(0x64)|0x03);
	    } else {
			sccb_reg_write(0x13, 0xf0); //AGC,AEC,AWB OFF
			sccb_reg_write(0x64, sccb_reg_read(0x64)&0xFC);

			setGain(gain);
			setExposure(exposure);
	    }
	}
	bool getAutoWhiteBalance() const { return awb; }
	void setAutoWhiteBalance(bool val) {
	    awb = val;
	    if (val) {
			sccb_reg_write(0x63, 0xe0); //AWB ON
	    }else{
			sccb_reg_write(0x63, 0xAA); //AWB OFF
	    }
	}
	uint8_t getGain() const { return gain; }
	void setGain(uint8_t val) {
	    gain = val;
	    switch(val & 0x30){
		case 0x00:
		    val &=0x0F;
		    break;
		case 0x10:
		    val &=0x0F;
		    val |=0x30;
		    break;
		case 0x20:
		    val &=0x0F;
		    val |=0x70;
		    break;
		case 0x30:
		    val &=0x0F;
		    val |=0xF0;
		    break;
	    }
	    sccb_reg_write(0x00, val);
	}
	uint8_t getExposure() const { return exposure; }
	void setExposure(uint8_t val) {
	    exposure = val;
	    sccb_reg_write(0x08, val>>7);
    	sccb_reg_write(0x10, val<<1);
	}
	uint8_t getSharpness() const { return sharpness; }
	void setSharpness(uint8_t val) {
	    sharpness = val;
	    sccb_reg_write(0x91, val); //vga noise
    	sccb_reg_write(0x8E, val); //qvga noise
	}
	uint8_t getContrast() const { return contrast; }
	void setContrast(uint8_t val) {
	    contrast = val;
	    sccb_reg_write(0x9C, val);
	}
	uint8_t getBrightness() const { return brightness; }
	void setBrightness(uint8_t val) {
	    brightness = val;
	    sccb_reg_write(0x9B, val);
	}
	uint8_t getHue() const { return hue; }
	void setHue(uint8_t val) {
		hue = val;
		sccb_reg_write(0x01, val);
	}
	uint8_t getRedBalance() const { return redblc; }
	void setRedBalance(uint8_t val) {
		redblc = val;
		sccb_reg_write(0x43, val);
	}
	uint8_t getBlueBalance() const { return blueblc; }
	void setBlueBalance(uint8_t val) {
		blueblc = val;
		sccb_reg_write(0x42, val);
	}
	void setFlip(bool horizontal = false, bool vertical = false) {
        flip_h = horizontal;
        flip_v = vertical;
		uint8_t val = sccb_reg_read(0x0c);
        val &= ~0xc0;
        if (!horizontal) val |= 0x40;
        if (!vertical) val |= 0x80;
        sccb_reg_write(0x0c, val);
	}
    

    bool isStreaming() const { return is_streaming; }
	bool isNewFrame() const;
	const uint8_t* getLastFramePointer();

	uint32_t getWidth() const { return frame_width; }
	uint32_t getHeight() const { return frame_height; }
	uint8_t getFrameRate() const { return frame_rate; }
	uint32_t getRowBytes() const { return frame_stride; }

	//
	static const std::vector<PS3EYERef>& getDevices( bool forceRefresh = false );
	static bool updateDevices();

private:
	PS3EYECam(const PS3EYECam&);
    void operator=(const PS3EYECam&);

	void release();

	// usb ops
	void ov534_set_frame_rate(uint8_t frame_rate);
	void ov534_set_led(int status);
	void ov534_reg_write(uint16_t reg, uint8_t val);
	uint8_t ov534_reg_read(uint16_t reg);
	int sccb_check_status();
	void sccb_reg_write(uint8_t reg, uint8_t val);
	uint8_t sccb_reg_read(uint16_t reg);
	void reg_w_array(const uint8_t (*data)[2], int len);
	void sccb_w_array(const uint8_t (*data)[2], int len);

	// controls
	bool autogain;
	uint8_t gain; // 0 <-> 63
	uint8_t exposure; // 0 <-> 255
	uint8_t sharpness; // 0 <-> 63
	uint8_t hue; // 0 <-> 255
	bool awb;
	uint8_t brightness; // 0 <-> 255
	uint8_t contrast; // 0 <-> 255
	uint8_t blueblc; // 0 <-> 255
	uint8_t redblc; // 0 <-> 255
    bool flip_h;
    bool flip_v;
	//
    bool is_streaming;

	std::shared_ptr<class USBMgr> mgrPtr;

	static bool devicesEnumerated;
    static std::vector<PS3EYERef> devices;

	uint32_t frame_width;
	uint32_t frame_height;
	uint32_t frame_stride;
	uint8_t frame_rate;

	double last_qued_frame_time;

	//usb stuff
	libusb_device *device_;
	libusb_device_handle *handle_;
	uint8_t *usb_buf;

	std::shared_ptr<class URBDesc> urb;

	bool open_usb();
	void close_usb();

};

} // namespace


#endif

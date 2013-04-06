#ifndef PS3EYEDRIVER_H
#define PS3EYEDRIVER_H

#include "libusb.h"

#ifdef __cplusplus
extern "C" {
#endif

/* used to list framerates supported by a camera mode (resolution) */
struct framerates {
    const uint8_t *rates;
    int nrates;
};

struct ps3eye_mode
{
    uint32_t width;
    uint32_t height;
    uint32_t bytesperline;
    uint32_t sizeimage;
};

static const struct ps3eye_mode ps3eye_modes[] = {
    {320, 240, 
      /*.bytesperline = */320 * 2,
      /*.sizeimage = */320 * 240 * 2
    },
    {640, 480, 
      /*.bytesperline = */640 * 2,
      /*.sizeimage = */640 * 480 * 2
    },
 };

static const uint8_t qvga_rates[] = {125, 100, 75, 60, 50, 40, 30};
static const uint8_t vga_rates[] = {60, 50, 40, 30, 15};
 
static const struct framerates ps3eye_framerates[] = {
    { /* 320x240 */
        /*.rates = */qvga_rates,
        /*.nrates = */7,
    },
    { /* 640x480 */
        /*.rates = */vga_rates,
        /*.nrates = */5,
    },
};


struct cam_frame
{
    uint8_t *data;          /* frame buffer */
    uint8_t *data_end;      /* end of frame while filling */
    int length;
};


struct ps3eye_cam
{
    libusb_device *device;
    libusb_device_handle *handle;
    const struct ps3eye_mode *mode;
    uint8_t frame_rate;
    uint8_t *usb_buf;
    struct libusb_transfer *xfr[2];
    // frame buffer
    uint8_t *frame_buff;
    struct cam_frame frame[16];
    uint8_t last_packet_type;
    uint8_t frame_ind;
    int8_t last_ready_ind;
    int8_t last_request_ind;
    uint32_t last_pts; // used for frame parsing
    uint16_t last_fid;
    // status
    uint8_t streaming;
    // controls
    uint8_t autogain; // 0 <-> 1
	uint8_t awb; // 0 <-> 1
    uint8_t gain; // 0 <-> 63
	uint8_t sharpness; // 0 <-> 63
    uint8_t exposure; // 0 <-> 255
    uint8_t hue; // 0 <-> 255
    uint8_t blueblc; // 0 <-> 255
	uint8_t redblc; // 0 <-> 255
    uint8_t brightness; // 0 <-> 255
    uint8_t contrast; // 0 <-> 255
};

int ps3eye_list_devices(struct ps3eye_cam **list);
void ps3eye_free_devices(struct ps3eye_cam **list, int count);
int ps3eye_update();

int ps3eye_init(struct ps3eye_cam *cam, const struct ps3eye_mode* mode, uint8_t fps);
int ps3eye_start(struct ps3eye_cam *cam);
void ps3eye_stop(struct ps3eye_cam *cam);
int ps3eye_request_frame(struct ps3eye_cam *cam, const uint8_t **data);

// CONTROLS
void ps3eye_set_blue_balance(struct ps3eye_cam *cam, uint8_t val);
void ps3eye_set_red_balance(struct ps3eye_cam *cam, uint8_t val);
void ps3eye_set_hue(struct ps3eye_cam *cam, uint8_t val);
void ps3eye_set_brightness(struct ps3eye_cam *cam, uint8_t val);
void ps3eye_set_contrast(struct ps3eye_cam *cam, uint8_t val);
void ps3eye_set_sharpness(struct ps3eye_cam *cam, uint8_t val);
void ps3eye_set_exposure(struct ps3eye_cam *cam, uint8_t val);
void ps3eye_set_gain(struct ps3eye_cam *cam, uint8_t val);
void ps3eye_set_auto_white_balance(struct ps3eye_cam *cam, uint8_t val);
void ps3eye_set_auto_gain(struct ps3eye_cam *cam, uint8_t val);
//


#ifdef __cplusplus
}
#endif

#endif
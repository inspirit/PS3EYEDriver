
#include "ps3eye_driver.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

//#include <pthread.h>


#ifndef ARRAY_SIZE
#define ARRAY_SIZE(_A) (sizeof(_A) / sizeof((_A)[0]))
#endif

#define debug(x...) //fprintf(stdout,x);fprintf(stdout,"\n")
//#define debug(x,...) fprintf(stdout,x);fprintf(stdout,"\n")


#define PS3EYE_VENDOR_ID  0x1415
#define PS3EYE_PRODUCT_ID  0x2000

#define OV534_REG_ADDRESS   0xf1    /* sensor address */
#define OV534_REG_SUBADDR   0xf2
#define OV534_REG_WRITE     0xf3
#define OV534_REG_READ      0xf4
#define OV534_REG_OPERATION 0xf5
#define OV534_REG_STATUS    0xf6

#define OV534_OP_WRITE_3    0x37
#define OV534_OP_WRITE_2    0x33
#define OV534_OP_READ_2     0xf9

#define CTRL_TIMEOUT 500
#define BULK_SIZE 16384


/* packet types when moving from iso buf to frame buf */
enum gspca_packet_type {
    DISCARD_PACKET,
    FIRST_PACKET,
    INTER_PACKET,
    LAST_PACKET
};


static const uint8_t ov534_reg_initdata[][2] = {
    { 0xe7, 0x3a },

    { OV534_REG_ADDRESS, 0x42 }, /* select OV772x sensor */

    { 0xc2, 0x0c },
    { 0x88, 0xf8 },
    { 0xc3, 0x69 },
    { 0x89, 0xff },
    { 0x76, 0x03 },
    { 0x92, 0x01 },
    { 0x93, 0x18 },
    { 0x94, 0x10 },
    { 0x95, 0x10 },
    { 0xe2, 0x00 },
    { 0xe7, 0x3e },

    { 0x96, 0x00 },

    { 0x97, 0x20 },
    { 0x97, 0x20 },
    { 0x97, 0x20 },
    { 0x97, 0x0a },
    { 0x97, 0x3f },
    { 0x97, 0x4a },
    { 0x97, 0x20 },
    { 0x97, 0x15 },
    { 0x97, 0x0b },

    { 0x8e, 0x40 },
    { 0x1f, 0x81 },
    { 0x34, 0x05 },
    { 0xe3, 0x04 },
    { 0x88, 0x00 },
    { 0x89, 0x00 },
    { 0x76, 0x00 },
    { 0xe7, 0x2e },
    { 0x31, 0xf9 },
    { 0x25, 0x42 },
    { 0x21, 0xf0 },

    { 0x1c, 0x00 },
    { 0x1d, 0x40 },
    { 0x1d, 0x02 }, /* payload size 0x0200 * 4 = 2048 bytes */
    { 0x1d, 0x00 }, /* payload size */

// -------------

//  { 0x1d, 0x01 },/* frame size */     // kwasy
//  { 0x1d, 0x4b },/* frame size */
//  { 0x1d, 0x00 }, /* frame size */


//  { 0x1d, 0x02 },/* frame size */     // macam
//  { 0x1d, 0x57 },/* frame size */
//  { 0x1d, 0xff }, /* frame size */

    { 0x1d, 0x02 },/* frame size */     // jfrancois / linuxtv.org/hg/v4l-dvb
    { 0x1d, 0x58 },/* frame size */
    { 0x1d, 0x00 }, /* frame size */

// ---------

    { 0x1c, 0x0a },
    { 0x1d, 0x08 }, /* turn on UVC header */
    { 0x1d, 0x0e }, /* .. */

    { 0x8d, 0x1c },
    { 0x8e, 0x80 },
    { 0xe5, 0x04 },

// ----------------
//  { 0xc0, 0x28 },//   kwasy / macam
//  { 0xc1, 0x1e },//

    { 0xc0, 0x50 },     // jfrancois
    { 0xc1, 0x3c },
    { 0xc2, 0x0c }, 


    
};

static const uint8_t ov772x_reg_initdata[][2] = {

    {0x12, 0x80 },
    {0x11, 0x01 },
    {0x11, 0x01 },
    {0x11, 0x01 },
    {0x11, 0x01 },
    {0x11, 0x01 },
    {0x11, 0x01 },
    {0x11, 0x01 },
    {0x11, 0x01 },
    {0x11, 0x01 },
    {0x11, 0x01 },
    {0x11, 0x01 },

    {0x3d, 0x03 },
    {0x17, 0x26 },
    {0x18, 0xa0 },
    {0x19, 0x07 },
    {0x1a, 0xf0 },
    {0x32, 0x00 },
    {0x29, 0xa0 },
    {0x2c, 0xf0 },
    {0x65, 0x20 },
    {0x11, 0x01 },
    {0x42, 0x7f },
    {0x63, 0xAA },  // AWB
    {0x64, 0xff },
    {0x66, 0x00 },
    {0x13, 0xf0 },  // COM8  - jfrancois 0xf0   orig x0f7
    {0x0d, 0x41 },
    {0x0f, 0xc5 },
    {0x14, 0x11 },

    {0x22, 0x7f },
    {0x23, 0x03 },
    {0x24, 0x40 },
    {0x25, 0x30 },
    {0x26, 0xa1 },
    {0x2a, 0x00 },
    {0x2b, 0x00 }, 
    {0x6b, 0xaa },
    {0x13, 0xff },  // COM8 - jfrancois 0xff orig 0xf7

    {0x90, 0x05 },
    {0x91, 0x01 },
    {0x92, 0x03 },
    {0x93, 0x00 },
    {0x94, 0x60 },
    {0x95, 0x3c },
    {0x96, 0x24 },
    {0x97, 0x1e },
    {0x98, 0x62 },
    {0x99, 0x80 },
    {0x9a, 0x1e },
    {0x9b, 0x08 },
    {0x9c, 0x20 },
    {0x9e, 0x81 },

    {0xa6, 0x04 },
    {0x7e, 0x0c },
    {0x7f, 0x16 },
    {0x80, 0x2a },
    {0x81, 0x4e },
        {0x82, 0x61 },
    {0x83, 0x6f },
    {0x84, 0x7b },
    {0x85, 0x86 },
    {0x86, 0x8e },
    {0x87, 0x97 },
    {0x88, 0xa4 },
    {0x89, 0xaf },
    {0x8a, 0xc5 },
    {0x8b, 0xd7 },
    {0x8c, 0xe8 },
    {0x8d, 0x20 },

    {0x0c, 0x90 },

    {0x2b, 0x00 }, 
    {0x22, 0x7f },
    {0x23, 0x03 },
    {0x11, 0x01 },
    {0x0c, 0xd0 },
    {0x64, 0xff },
    {0x0d, 0x41 },

    {0x14, 0x41 },
    {0x0e, 0xcd },
    {0xac, 0xbf },
    {0x8e, 0x00 },  // De-noise threshold - jfrancois 0x00 - orig 0x04
    {0x0c, 0xd0 }

};

static const uint8_t bridge_start_vga[][2] = {
    {0x1c, 0x00},
    {0x1d, 0x40},
    {0x1d, 0x02},
    {0x1d, 0x00},
    {0x1d, 0x02},
    {0x1d, 0x58},
    {0x1d, 0x00},
    {0xc0, 0x50},
    {0xc1, 0x3c},
};
static const uint8_t sensor_start_vga[][2] = {
    {0x12, 0x00},
    {0x17, 0x26},
    {0x18, 0xa0},
    {0x19, 0x07},
    {0x1a, 0xf0},
    {0x29, 0xa0},
    {0x2c, 0xf0},
    {0x65, 0x20},
};
static const uint8_t bridge_start_qvga[][2] = {
    {0x1c, 0x00},
    {0x1d, 0x40},
    {0x1d, 0x02},
    {0x1d, 0x00},
    {0x1d, 0x01},
    {0x1d, 0x4b},
    {0x1d, 0x00},
    {0xc0, 0x28},
    {0xc1, 0x1e},
};
static const uint8_t sensor_start_qvga[][2] = {
    {0x12, 0x40},
    {0x17, 0x3f},
    {0x18, 0x50},
    {0x19, 0x03},
    {0x1a, 0x78},
    {0x29, 0x50},
    {0x2c, 0x78},
    {0x65, 0x2f},
};


static void ov534_reg_write(struct ps3eye_cam *cam, uint16_t reg, uint8_t val)
{
    int ret;

    //debug("reg=0x%04x, val=0%02x", reg, val);
    cam->usb_buf[0] = val;

    ret = libusb_control_transfer(cam->handle,
                            LIBUSB_ENDPOINT_OUT | 
                            LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, 
                            0x01, 0x00, reg,
                            cam->usb_buf, 1, 500);
    if (ret < 0) {
        debug("write failed");
    }
}

static uint8_t ov534_reg_read(struct ps3eye_cam *cam, uint16_t reg)
{
    int ret;

    ret = libusb_control_transfer(cam->handle,
                            LIBUSB_ENDPOINT_IN|LIBUSB_REQUEST_TYPE_VENDOR|LIBUSB_RECIPIENT_DEVICE, 
                            0x01, 0x00, reg,
                            cam->usb_buf, 1, 500);

    //debug("reg=0x%04x, data=0x%02x", reg, cam->usb_buf[0]);
    if (ret < 0) {
        debug("read failed");
    
    }
    return cam->usb_buf[0];
}

/* Two bits control LED: 0x21 bit 7 and 0x23 bit 7.
 * (direction and output)? */
static void ov534_set_led(struct ps3eye_cam *cam, int status)
{
    uint8_t data;

    debug("led status: %d", status);

    data = ov534_reg_read(cam, 0x21);
    data |= 0x80;
    ov534_reg_write(cam, 0x21, data);

    data = ov534_reg_read(cam, 0x23);
    if (status)
        data |= 0x80;
    else
        data &= ~0x80;

    ov534_reg_write(cam, 0x23, data);
    
    if (!status) {
        data = ov534_reg_read(cam, 0x21);
        data &= ~0x80;
        ov534_reg_write(cam, 0x21, data);
    }
}

static int sccb_check_status(struct ps3eye_cam *cam)
{
    uint8_t data;
    int i;

    for (i = 0; i < 5; i++) {
        data = ov534_reg_read(cam, OV534_REG_STATUS);

        switch (data) {
        case 0x00:
            return 1;
        case 0x04:
            return 0;
        case 0x03:
            break;
        default:
            debug("sccb status 0x%02x, attempt %d/5", data, i + 1);
        }
    }
    return 0;
}

static void sccb_reg_write(struct ps3eye_cam *cam, uint8_t reg, uint8_t val)
{
    debug("reg: 0x%02x, val: 0x%02x", reg, val);
    ov534_reg_write(cam, OV534_REG_SUBADDR, reg);
    ov534_reg_write(cam, OV534_REG_WRITE, val);
    ov534_reg_write(cam, OV534_REG_OPERATION, OV534_OP_WRITE_3);

    if (!sccb_check_status(cam))
        debug("sccb_reg_write failed");
}


static uint8_t sccb_reg_read(struct ps3eye_cam *cam, uint16_t reg)
{
    ov534_reg_write(cam, OV534_REG_SUBADDR, reg);
    ov534_reg_write(cam, OV534_REG_OPERATION, OV534_OP_WRITE_2);
    if (!sccb_check_status(cam))
        debug("sccb_reg_read failed 1");

    ov534_reg_write(cam, OV534_REG_OPERATION, OV534_OP_READ_2);
    if (!sccb_check_status(cam))
        debug( "sccb_reg_read failed 2");

    return ov534_reg_read(cam, OV534_REG_READ);
}
/* output a bridge sequence (reg - val) */
static void reg_w_array(struct ps3eye_cam *cam, const uint8_t (*data)[2], int len)
{
    while (--len >= 0) {
        ov534_reg_write(cam, (*data)[0], (*data)[1]);
        data++;
    }
}

/* output a sensor sequence (reg - val) */
static void sccb_w_array(struct ps3eye_cam *cam, const uint8_t (*data)[2], int len)
{
    while (--len >= 0) {
        if ((*data)[0] != 0xff) {
            sccb_reg_write(cam, (*data)[0], (*data)[1]);
        } else {
            sccb_reg_read(cam, (*data)[1]);
            sccb_reg_write(cam, 0xff, 0x00);
        }
        data++;
    }
}

/* set videomode */
/*
static void ov534_set_res(struct ps3eye_cam *cam, )
{
    if (res == QVGA) { //set to qvga
        ov534_reg_write(cam, 0x1c, 0x00);
        ov534_reg_write(cam, 0x1d, 0x40);
        ov534_reg_write(cam, 0x1d, 0x02);
        ov534_reg_write(cam, 0x1d, 0x00);
        ov534_reg_write(cam, 0x1d, 0x01);
        ov534_reg_write(cam, 0x1d, 0x4b);
        ov534_reg_write(cam, 0x1d, 0x00);
        ov534_reg_write(cam, 0xc0, 0x28);
        ov534_reg_write(cam, 0xc1, 0x1e);

        sccb_reg_write(cam, 0x12, 0x40);
        sccb_reg_write(cam, 0x17, 0x3f);
        sccb_reg_write(cam, 0x18, 0x50);
        sccb_reg_write(cam, 0x19, 0x03);
        sccb_reg_write(cam, 0x1a, 0x78);

        sccb_reg_write(cam, 0x29, 0x50);
        sccb_reg_write(cam, 0x2c, 0x78);
        sccb_reg_write(cam, 0x65, 0x2f);     // http://forums.ps2dev.org/viewtopic.php?p=74541#74541

    } 
    else 
    { //set to vga default
        ov534_reg_write(cam, 0x1c, 0x00);
        ov534_reg_write(cam, 0x1d, 0x40);
        ov534_reg_write(cam, 0x1d, 0x02);
        ov534_reg_write(cam, 0x1d, 0x00);
        ov534_reg_write(cam, 0x1d, 0x02);
        ov534_reg_write(cam, 0x1d, 0x58);
        ov534_reg_write(cam, 0x1d, 0x00);
        ov534_reg_write(cam, 0xc0, 0x50);
        ov534_reg_write(cam, 0xc1, 0x3c);

        sccb_reg_write(cam, 0x12, 0x00); 
        sccb_reg_write(cam, 0x17, 0x26);
        sccb_reg_write(cam, 0x18, 0xa0);
        sccb_reg_write(cam, 0x19, 0x07);
        sccb_reg_write(cam, 0x1a, 0xf0);

        sccb_reg_write(cam, 0x29, 0xa0);
        sccb_reg_write(cam, 0x2c, 0xf0);
        sccb_reg_write(cam, 0x65, 0x20);     // http://forums.ps2dev.org/viewtopic.php?p=74541#74541
    }
}
*/

/* set framerate */
static void ov534_set_frame_rate(struct ps3eye_cam *cam, uint8_t frame_rate)
{
     int i;
     struct rate_s {
             uint8_t fps;
             uint8_t r11;
             uint8_t r0d;
             uint8_t re5;
     };
     const struct rate_s *r;
     static const struct rate_s rate_0[] = { /* 640x480 */
             {60, 0x01, 0xc1, 0x04},
             {50, 0x01, 0x41, 0x02},
             {40, 0x02, 0xc1, 0x04},
             {30, 0x04, 0x81, 0x02},
             {15, 0x03, 0x41, 0x04},
     };
     static const struct rate_s rate_1[] = { /* 320x240 */
             {125, 0x02, 0x81, 0x02},
             {100, 0x02, 0xc1, 0x04},
             {75, 0x03, 0xc1, 0x04},
             {60, 0x04, 0xc1, 0x04},
             {50, 0x02, 0x41, 0x04},
             {40, 0x03, 0x41, 0x04},
             {30, 0x04, 0x41, 0x04},
     };

     if (cam->mode->width == 640) { // VGA
             r = rate_0;
             i = ARRAY_SIZE(rate_0);
     } else {
             r = rate_1;
             i = ARRAY_SIZE(rate_1);
     }
     while (--i > 0) {
             if (frame_rate >= r->fps)
                     break;
             r++;
     }
 
     
     sccb_reg_write(cam, 0x11, r->r11);
     sccb_reg_write(cam, 0x0d, r->r0d);
     ov534_reg_write(cam, 0xe5, r->re5);

     cam->frame_rate = r->fps;

     debug("frame_rate: %d", r->fps);
}
/////////////////////////////// CONTROLS
void ps3eye_set_auto_gain(struct ps3eye_cam *cam, uint8_t val) {
    cam->autogain = val;
    if (val) {
        sccb_reg_write(cam, 0x13, 0xf7); //AGC,AEC,AWB ON
        sccb_reg_write(cam, 0x64, sccb_reg_read(cam, 0x64)|0x03);
    } else {
        sccb_reg_write(cam, 0x13, 0xf0); //AGC,AEC,AWB OFF
        sccb_reg_write(cam, 0x64, sccb_reg_read(cam, 0x64)&0xFC); 

		ps3eye_set_gain(cam, cam->gain);
        ps3eye_set_exposure(cam, cam->exposure);
    }
}

void ps3eye_set_auto_white_balance(struct ps3eye_cam *cam, uint8_t val) {
    cam->awb = val;
    if (val) {
        sccb_reg_write(cam, 0x63, 0xe0); //AWB ON
    }else{
        sccb_reg_write(cam, 0x63, 0xAA); //AWB OFF
    }
}

void ps3eye_set_gain(struct ps3eye_cam *cam, uint8_t val) {
    cam->gain = val;
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
    sccb_reg_write(cam, 0x00, val);
}

void ps3eye_set_exposure(struct ps3eye_cam *cam, uint8_t val) {
    cam->exposure = val;
    sccb_reg_write(cam, 0x08, val>>7);
    sccb_reg_write(cam, 0x10, val<<1);
}

void ps3eye_set_sharpness(struct ps3eye_cam *cam, uint8_t val) {
    cam->sharpness = val;
    sccb_reg_write(cam, 0x91, val); //vga noise
    sccb_reg_write(cam, 0x8E, val); //qvga noise
}

void ps3eye_set_contrast(struct ps3eye_cam *cam, uint8_t val) {
    cam->contrast = val;
    sccb_reg_write(cam, 0x9C, val);
}

void ps3eye_set_brightness(struct ps3eye_cam *cam, uint8_t val) {
    cam->brightness = val;
    sccb_reg_write(cam, 0x9B, val);
}

void ps3eye_set_hue(struct ps3eye_cam *cam, uint8_t val)
{
    sccb_reg_write(cam, 0x01, val);
}

void ps3eye_set_blue_balance(struct ps3eye_cam *cam, uint8_t val) {
    cam->blueblc = val;
    sccb_reg_write(cam, 0x42, val);
}
void ps3eye_set_red_balance(struct ps3eye_cam *cam, uint8_t val) {
    cam->redblc = val;
    sccb_reg_write(cam, 0x43, val);
}
//////////////////////////////////////////


/*
 * add data to the current frame
 *
 * This function is called by the subdrivers at interrupt level.
 *
 * To build a frame, these ones must add
 *  - one FIRST_PACKET
 *  - 0 or many INTER_PACKETs
 *  - one LAST_PACKET
 * DISCARD_PACKET invalidates the whole frame.
 * On LAST_PACKET, a new frame is returned.
 */
struct cam_frame *cam_frame_add(struct ps3eye_cam *cam,
                    enum gspca_packet_type packet_type,
                    struct cam_frame *frame,
                    const uint8_t *data, int len)
{
    int i;

    /* when start of a new frame, if the current frame buffer
     * is not queued, discard the whole frame */
    if (packet_type == FIRST_PACKET) {
        /*if (cam->last_request_ind < cam->last_ready_ind) 
        {
            debug("frame discard");
            cam->last_packet_type = DISCARD_PACKET;
            return frame;
        }*/
        frame->data_end = frame->data;
    } 
    else if (cam->last_packet_type == DISCARD_PACKET) 
    {
        if (packet_type == LAST_PACKET)
            cam->last_packet_type = packet_type;
        return frame;
    }

    /* append the packet to the frame buffer */
    if (len > 0) {
        if (frame->data_end - frame->data + len
                         > frame->length) 
        {
            debug("frame overflow %zd > %d",
                frame->data_end - frame->data + len, frame->length);
            packet_type = DISCARD_PACKET;
        } else {
            memcpy(frame->data_end, data, len);
            frame->data_end += len;
        }
    }

    cam->last_packet_type = packet_type;

    if (packet_type == LAST_PACKET) 
    {        
        cam->last_ready_ind = cam->frame_ind;
        i = (cam->frame_ind + 1) & 15;
        cam->frame_ind = i;
		if(cam->last_ready_ind == 0) cam->last_request_ind = -1;
        frame = &cam->frame[i];
        debug("frame completed %d", cam->last_ready_ind);
    }
    return frame;
}


/* Values for bmHeaderInfo (Video and Still Image Payload Headers, 2.4.3.3) */
#define UVC_STREAM_EOH  (1 << 7)
#define UVC_STREAM_ERR  (1 << 6)
#define UVC_STREAM_STI  (1 << 5)
#define UVC_STREAM_RES  (1 << 4)
#define UVC_STREAM_SCR  (1 << 3)
#define UVC_STREAM_PTS  (1 << 2)
#define UVC_STREAM_EOF  (1 << 1)
#define UVC_STREAM_FID  (1 << 0)

static void sd_pkt_scan(struct ps3eye_cam *cam, uint8_t *data, int len)
{
    struct cam_frame *frame = &cam->frame[cam->frame_ind];
    uint32_t this_pts;
    uint16_t this_fid;
    int remaining_len = len;
    int payload_len;

    payload_len = 2048;
    do {
        len = remaining_len < payload_len ? remaining_len : payload_len;//min(remaining_len, payload_len);

        /* Payloads are prefixed with a UVC-style header.  We
           consider a frame to start when the FID toggles, or the PTS
           changes.  A frame ends when EOF is set, and we've received
           the correct number of bytes. */

        /* Verify UVC header.  Header length is always 12 */
        if (data[0] != 12 || len < 12) {
            debug("bad header");
            goto discard;
        }

        /* Check errors */
        if (data[1] & UVC_STREAM_ERR) {
            debug("payload error");
            goto discard;
        }

        /* Extract PTS and FID */
        if (!(data[1] & UVC_STREAM_PTS)) {
            debug("PTS not present");
            goto discard;
        }

        this_pts = (data[5] << 24) | (data[4] << 16)
                        | (data[3] << 8) | data[2];
        this_fid = (data[1] & UVC_STREAM_FID) ? 1 : 0;

        /* If PTS or FID has changed, start a new frame. */
        if (this_pts != cam->last_pts || this_fid != cam->last_fid) {
            if (cam->last_packet_type == INTER_PACKET)
            {
                frame = cam_frame_add(cam, LAST_PACKET, frame, NULL, 0);
            }
            cam->last_pts = this_pts;
            cam->last_fid = this_fid;
            cam_frame_add(cam, FIRST_PACKET, frame, data + 12, len - 12);
        } /* If this packet is marked as EOF, end the frame */
        else if (data[1] & UVC_STREAM_EOF) 
        {
            cam->last_pts = 0;
            frame = cam_frame_add(cam, LAST_PACKET, frame, data + 12, len - 12);
        } else {
            /* Add the data from this payload */
            cam_frame_add(cam, INTER_PACKET, frame, data + 12, len - 12);
        }


        /* Done this payload */
        goto scan_next;

discard:
        /* Discard data until a new frame starts. */
        cam_frame_add(cam, DISCARD_PACKET, frame, NULL, 0);
scan_next:
        remaining_len -= len;
        data += len;
    } while (remaining_len > 0);
}

/*
 * look for an input transfer endpoint in an alternate setting
 * libusb_endpoint_descriptor
 */
static uint8_t find_ep(struct ps3eye_cam *cam)
{
	const struct libusb_interface_descriptor *altsetting;
    const struct libusb_endpoint_descriptor *ep;
	struct libusb_config_descriptor *config;
    int i;
    uint8_t ep_addr = 0;

    libusb_get_active_config_descriptor(cam->device, &config);

    if (!config) return 0;

    for (i = 0; i < config->bNumInterfaces; i++) {
        altsetting = config->interface[i].altsetting;
        if (altsetting[0].bInterfaceNumber == 0) {
            break;
        }
    }

    for (i = 0; i < altsetting->bNumEndpoints; i++) {
        ep = &altsetting->endpoint[i];
        if ((ep->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) == LIBUSB_TRANSFER_TYPE_BULK 
            && ep->wMaxPacketSize != 0) 
        {
            ep_addr = ep->bEndpointAddress;
            debug("find ep addr(%d) packet(%d)", ep_addr, ep->wMaxPacketSize);
            break;
        }
    }

    libusb_free_config_descriptor(config);

    return ep_addr;
}

static void LIBUSB_CALL cb_xfr(struct libusb_transfer *xfr)
{
    struct ps3eye_cam *cam = (struct ps3eye_cam *)xfr->user_data;
	enum libusb_transfer_status status = xfr->status;

    if (status != LIBUSB_TRANSFER_COMPLETED) 
	{
        debug("transfer status %d", status);

		//if(xfr->buffer) free(xfr->buffer);
		//libusb_free_transfer(xfr);
		if(cam->xfr[0] == xfr){
			cam->xfr[0] = NULL;
		} else {
			cam->xfr[1] = NULL;
		}

		if(status != LIBUSB_TRANSFER_CANCELLED)
        {
            ps3eye_stop(cam);
        }
        return;
    }

    //debug("length:%u, actual_length:%u\n", xfr->length, xfr->actual_length);
    //
	//cam->last_ready_ind = xfr->actual_length;

    sd_pkt_scan(cam, xfr->buffer, xfr->actual_length);

    if (libusb_submit_transfer(xfr) < 0) {
        debug("error re-submitting URB");
        ps3eye_stop(cam);
    }
}
static int init_transfers(struct ps3eye_cam *cam)
{
    struct libusb_transfer *xfr0,*xfr1;
	uint8_t* buff, *buff1;
	uint8_t ep_addr;
	int res;
    const int bsize = BULK_SIZE;

    // bulk transfers
    xfr0 = libusb_alloc_transfer(0);
    xfr1 = libusb_alloc_transfer(0);

	buff = cam->usb_buf + 64;
	buff1 = buff + bsize;

    cam->xfr[0] = xfr0;
    cam->xfr[1] = xfr1;

    ep_addr = find_ep(cam);
//    debug("found ep: %d", ep_addr);

    libusb_clear_halt(cam->handle, ep_addr);

    libusb_fill_bulk_transfer(xfr0, cam->handle, ep_addr, buff, bsize, cb_xfr, cam, 0);
    libusb_fill_bulk_transfer(xfr1, cam->handle, ep_addr, buff1, bsize, cb_xfr, cam, 0);

	//xfr0->flags |= LIBUSB_TRANSFER_FREE_BUFFER;
	//xfr1->flags |= LIBUSB_TRANSFER_FREE_BUFFER;

	res = libusb_submit_transfer(xfr0);
	res |= libusb_submit_transfer(xfr1);

    return res;
}

static int sd_config(struct ps3eye_cam *cam, const struct ps3eye_mode* mode, uint8_t fps)
{
    // controls
    cam->autogain = 0;
    cam->gain = 20;
    cam->exposure = 120;
    cam->sharpness = 0;
    cam->hue = 143;
    cam->blueblc = 128;
	cam->redblc = 128;
    cam->awb = 0;
    cam->brightness =  20;
    cam->contrast =  37;

    cam->mode = mode;

    if(mode->width == 640)
    {
        cam->frame_rate = fps < 15 ? 15 : fps;
    } else {
        cam->frame_rate = fps < 30 ? 30 : fps;
    }


    return 0;
}

static void sd_init(struct ps3eye_cam *cam)
{
    uint16_t sensor_id;

    /* reset bridge */
    ov534_reg_write(cam, 0xe7, 0x3a);
    ov534_reg_write(cam, 0xe0, 0x08);

#ifdef _MSC_VER
	Sleep(100);
#else
    nanosleep((struct timespec[]){{0, 100000000}}, NULL);
#endif

    /* initialize the sensor address */
    ov534_reg_write(cam, OV534_REG_ADDRESS, 0x42);

    /* reset sensor */
    sccb_reg_write(cam, 0x12, 0x80);
#ifdef _MSC_VER
	Sleep(10);
#else    
    nanosleep((struct timespec[]){{0, 10000000}}, NULL);
#endif

    /* probe the sensor */
    sccb_reg_read(cam, 0x0a);
    sensor_id = sccb_reg_read(cam, 0x0a) << 8;
    sccb_reg_read(cam, 0x0b);
    sensor_id |= sccb_reg_read(cam, 0x0b);
    debug("Sensor ID: %04x", sensor_id);

    /* initialize */
    reg_w_array(cam, ov534_reg_initdata, ARRAY_SIZE(ov534_reg_initdata));
    ov534_set_led(cam, 1);
    sccb_w_array(cam, ov772x_reg_initdata, ARRAY_SIZE(ov772x_reg_initdata));
    ov534_reg_write(cam, 0xe0, 0x09);
    ov534_set_led(cam, 0);

    ov534_set_frame_rate(cam, cam->frame_rate);
}

static void sd_start(struct ps3eye_cam *cam)
{
    if (cam->mode->width == 320) {  /* 320x240 */
        reg_w_array(cam, bridge_start_qvga, ARRAY_SIZE(bridge_start_qvga));
        sccb_w_array(cam, sensor_start_qvga, ARRAY_SIZE(sensor_start_qvga));
    } else {        /* 640x480 */
        reg_w_array(cam, bridge_start_vga, ARRAY_SIZE(bridge_start_vga));
        sccb_w_array(cam, sensor_start_vga, ARRAY_SIZE(sensor_start_vga));
    }

    ov534_set_frame_rate(cam, cam->frame_rate);
	//
    ps3eye_set_auto_gain(cam, cam->autogain);
    ps3eye_set_auto_white_balance(cam, cam->awb);
    ps3eye_set_gain(cam, cam->gain);
    ps3eye_set_hue(cam, cam->hue);
    ps3eye_set_exposure(cam, cam->exposure);
    ps3eye_set_brightness(cam, cam->brightness);
    ps3eye_set_contrast(cam, cam->contrast);
    ps3eye_set_sharpness(cam, cam->sharpness);
    ps3eye_set_blue_balance(cam, cam->blueblc);
	ps3eye_set_red_balance(cam, cam->redblc);
	//

    ov534_set_led(cam, 1);
    ov534_reg_write(cam, 0xe0, 0x00);
}

static void sd_stop(struct ps3eye_cam *cam)
{
    /* stop streaming data */
    ov534_reg_write(cam, 0xe0, 0x09);
    ov534_set_led(cam, 0);
}

static void free_frame(struct ps3eye_cam *cam)
{
    if(cam->frame_buff) {
        free(cam->frame_buff);
        cam->frame_buff = NULL;
    }
    cam->last_pts = 0;
    cam->last_fid = 0;
}

static void alloc_frame(struct ps3eye_cam *cam)
{
	int i;
    if(cam->frame_buff) {
        free_frame(cam);
    }
    cam->frame_buff = (uint8_t*)malloc( 16 * cam->mode->sizeimage );
    
    for(i = 0; i < 16; ++i) {
        cam->frame[i].data = cam->frame[i].data_end = cam->frame_buff + (i * cam->mode->sizeimage);
        cam->frame[i].length = cam->mode->sizeimage;
    }
    
}

///////////////////////////////////////////////

static libusb_context* usb_context = NULL;

static void init_usb()
{
    if(usb_context == NULL)
    {
        libusb_init(&usb_context);
        libusb_set_debug(usb_context, 1);
    }
}

static void exit_usb()
{
    if(usb_context != NULL)
    {
        libusb_exit(usb_context);
        usb_context = NULL;
    }
}


void ps3eye_free_devices(struct ps3eye_cam **list, int count)
{
    // clean up
    int i, r;
    struct ps3eye_cam *cam;
    struct ps3eye_cam *cams = *list;
    for(i = 0; i < count; ++i)
    {
        cam = &cams[i];
        if(cam->handle != NULL)
        {
			if(cam->streaming == 1)
			{
				ps3eye_stop(cam);
			}            
            r = libusb_release_interface(cam->handle, 0);
            debug("release interface %d", r);
            libusb_close(cam->handle);
        }
        if(cam->usb_buf) {
            free(cam->usb_buf);
            cam->usb_buf = NULL;
        }
        free_frame(cam);
    }

    free(cams);

    exit_usb();
}


int ps3eye_list_devices( struct ps3eye_cam **list )
{
    libusb_device *dev;
    libusb_device **devs;
    struct libusb_device_descriptor desc;
	struct ps3eye_cam *cam;
    int r, i = 0;
    int cnt;

    init_usb();

    cnt = libusb_get_device_list(usb_context, &devs);

    if (cnt < 0) debug("Error Device scan");

    cnt = 0;
    i = 0;
    while ((dev = devs[i++]) != NULL) 
    {
        r = libusb_get_device_descriptor(dev, &desc);
        if (r < 0) {
            debug("failed to get device descriptor");
            break;
        }

        if(desc.idVendor == PS3EYE_VENDOR_ID && desc.idProduct == PS3EYE_PRODUCT_ID) 
        {
            cnt++;
        }
    }

    *list = (struct ps3eye_cam *)malloc( cnt * sizeof(struct ps3eye_cam) );

    memset(*list, 0, cnt * sizeof(struct ps3eye_cam));

    i = 0;
    cnt = 0;

    while ((dev = devs[i++]) != NULL) 
    {
        r = libusb_get_device_descriptor(dev, &desc);
        if (r < 0) {
            debug("failed to get device descriptor");
            break;
        }

        if(desc.idVendor == PS3EYE_VENDOR_ID && desc.idProduct == PS3EYE_PRODUCT_ID) 
        {
            cam = &((*list)[cnt]);

            cam->device = dev;
            cam->handle = NULL;
            cam->usb_buf = NULL;
            cam->frame_buff = NULL;

            cam->xfr[0] = NULL;
            cam->xfr[1] = NULL;

            libusb_ref_device(dev);

            cnt++;
        }
    }

    libusb_free_device_list(devs, 1);

    return cnt;
}

int ps3eye_update()
{
    return libusb_handle_events(usb_context);
}

int ps3eye_init(struct ps3eye_cam *cam, const struct ps3eye_mode* mode, uint8_t fps)
{
    int r;
    r = libusb_open(cam->device, &cam->handle);
    if(r < 0) {
        debug("error open");
        return -1;
    }

    // seems to be broken with my device at least
    //r = libusb_set_configuration(cam->handle, 0);
	//if(r != 0) return -1;
    //debug("set config %d", r);

    r = libusb_claim_interface(cam->handle, 0);
    if(r != 0) {
        debug("claim interface %d", r);
        return -1;
    }

    cam->usb_buf = (uint8_t*)malloc(64 + BULK_SIZE*2);

    sd_config(cam, mode, fps);
    sd_init(cam);

    alloc_frame(cam);

    return 0;
}

int ps3eye_start(struct ps3eye_cam *cam)
{
    int r = -1;
    sd_start(cam);
    r = init_transfers(cam);
    if(r != 0)
    {
        debug("failed to start transfers");
        ps3eye_stop(cam);
        return -1;
    }
    cam->streaming = 1;

	cam->frame_ind = 0;
    cam->last_ready_ind = 0;
    cam->last_request_ind = 0;
    cam->last_packet_type = DISCARD_PACKET;

    return r;
}

void ps3eye_stop(struct ps3eye_cam *cam)
{
	struct libusb_transfer *xfr0, *xfr1;
	xfr0 = cam->xfr[0];
	xfr1 = cam->xfr[1];
    if(xfr0) {
		libusb_cancel_transfer(xfr0);
	}
    if(xfr1) {
		libusb_cancel_transfer(xfr1);
	}
	while (cam->xfr[0] || cam->xfr[1]) {
		if (libusb_handle_events(usb_context) < 0)
			break;
	}
	libusb_free_transfer(xfr0);
	libusb_free_transfer(xfr1);
    //
    cam->xfr[0] = NULL;
    cam->xfr[1] = NULL;

	sd_stop(cam);
    
    cam->streaming = 0;
}

int ps3eye_request_frame(struct ps3eye_cam *cam, const uint8_t **data)
{
	int is_frame_new = 0;
	*data = 0;
    if(cam->streaming == 1)
	{
		is_frame_new = cam->last_request_ind < cam->last_ready_ind;
		if(is_frame_new)
		{
			cam->last_request_ind = cam->last_ready_ind;
			*data = (const uint8_t*)cam->frame[cam->last_ready_ind].data;
		}
	}

	return is_frame_new;
}

/**
 * PS3EYEDriver C API Interface for use with PS Move API
 * Copyright (c) 2014 Thomas Perl <m@thp.io>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 * 
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 **/

#ifndef PS3EYEDRIVER_H
#define PS3EYEDRIVER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ps3eye_t ps3eye_t;

/**
 * Initialize and enumerate connected cameras.
 * Needs to be called once before all other API functions.
 **/
void
ps3eye_init();

/**
 * De-initialize the library and free resources.
 * If a pseye_t * object is still opened, nothing happens.
 **/
void
ps3eye_uninit();

/**
 * Return the number of PSEye cameras connected via USB.
 **/
int
ps3eye_count_connected();

/**
 * Open a PSEye camera device by id.
 * The id is zero-based, and must be smaller than the count.
 * width and height should usually be 640x480 or 320x240
 * fps is the target frame rate, 60 usually works fine here
 **/
ps3eye_t *
ps3eye_open(int id, int width, int height, int fps);

/**
 * Grab the next frame as YUV422 blob.
 * A pointer to the buffer will be passed back. The buffer
 * will only be valid until the next call, or until the eye
 * is closed again with ps3eye_close(). If stride is not NULL,
 * the byte offset between two consecutive lines in the frame
 * will be written to *stride.
 **/
unsigned char *
ps3eye_grab_frame(ps3eye_t *eye, int *stride);

/**
 * Close a PSEye camera device and free allocated resources.
 * To really close the library, you should also call ps3eye_uninit().
 **/
void
ps3eye_close(ps3eye_t *eye);
    
int
ps3eye_set_parameters(ps3eye_t *eye, bool autogain, bool awb, uint8_t gain, uint8_t exposure,
                        uint8_t contrast, uint8_t brightness);
/**
 * Sets all ps3eye parameters on device.
 * gain: 0 <-> 63; exposure: 0 <-> 255; sharpness: 0 <-> 63; hue: 0 <-> 255;
 * brightness: 0 <-> 255; contrast: 0 <-> 255; blueblc: 0 <-> 255; redblc: 0 <-> 255
 **/

#ifdef __cplusplus
};
#endif

#endif /* PS3EYEDRIVER_H */

/**
 * PS3EYEDriver Simple SDL example (no GL rendering yet)
 * Thomas Perl <m@thp.io>; 2014-01-10
 **/

#include <SDL.h>

#include "ps3eyedriver.h"
#include "yuv422rgba.h"

#define CAM_WIDTH 640
#define CAM_HEIGHT 480

int
main(int argc, char *argv[])
{
    ps3eye_init();

    if (ps3eye_count_connected() == 0) {
        printf("No camera connected.\n");
        return 0;
    }

    ps3eye_t *eye = ps3eye_open(0, CAM_WIDTH, CAM_HEIGHT, 60);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Surface *surface = SDL_SetVideoMode(CAM_WIDTH, CAM_HEIGHT, 0, 0);

    unsigned char *rgba = (unsigned char *)malloc(CAM_WIDTH * CAM_HEIGHT * 4);

    SDL_Surface *rgba_surface = SDL_CreateRGBSurfaceFrom(rgba,
            CAM_WIDTH, CAM_HEIGHT, 32, CAM_WIDTH * 4,
            0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

    SDL_Event e;
    while (true) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                return 0;
            }
        }

        int stride = 0;
        unsigned char *pixels = ps3eye_grab_frame(eye, &stride);

        yuv422_to_rgba(pixels, stride, rgba, CAM_WIDTH, CAM_HEIGHT);
        // To just test raw read performance, use this:
        //memcpy(rgba, pixels, stride * CAM_HEIGHT);

        SDL_BlitSurface(rgba_surface, NULL, surface, NULL);
        SDL_Flip(surface);
    }

    ps3eye_close(eye);
    ps3eye_uninit();

    return 0;
}

/**
 * PS3EYEDriver Simple SDL example (no GL rendering yet)
 * Thomas Perl <m@thp.io>; 2014-01-10
 **/

#include <SDL.h>
#include "ps3eye.h"
#include "yuv422rgba.h"


struct yuv422_buffer_t {
    void update(const unsigned char *pixels, int stride, int width, int height)
    {
        size_t size = stride * height;

        if (this->size != size) {
            this->pixels = (unsigned char *)realloc(this->pixels, size);
            this->size = size;
        }

        memcpy(this->pixels, pixels, size);
        this->stride = stride;
        this->width = width;
        this->height = height;
    }
    
    unsigned char *pixels;
    size_t size;

    int stride;
    int width;
    int height;
};

struct yuv422_buffers_t {
    yuv422_buffers_t(int count)
        : current(0)
        , count(count)
        , buffers((yuv422_buffer_t *)calloc(sizeof(yuv422_buffer_t), count))
    {
    }

    ~yuv422_buffers_t()
    {
        for (int i=0; i<count; i++) {
            free(buffers[i].pixels);
        }
        free(buffers);
    }

    yuv422_buffer_t *next()
    {
        // TODO: Proper buffer queueing and locking
        current = (current + 1) % count;
        return buffers + current;
    }

    yuv422_buffer_t *read()
    {
        // TODO: Proper buffer dequeueing and locking
        int last = current - 1;
        if (last < 0) last += count;
        return buffers + last;
    }

    int current;
    int count;
    yuv422_buffer_t *buffers;
};

struct ps3eye_context {
    ps3eye_context(int width, int height, int fps)
        : buffers(3 /* number of buffers */)
        , eye(0)
        , devices(ps3eye::PS3EYECam::getDevices())
        , running(true)
        , last_ticks(0)
        , last_frames(0)
    {
        if (hasDevices()) {
            eye = devices[0];
            eye->init(width, height, fps);
        }
    }

    bool
    hasDevices()
    {
        return (devices.size() > 0);
    }

    yuv422_buffers_t buffers;
    std::vector<ps3eye::PS3EYECam::PS3EYERef> devices;
    ps3eye::PS3EYECam::PS3EYERef eye;

    bool running;
    Uint32 last_ticks;
    Uint32 last_frames;
};

int
ps3eye_cam_thread(void *user_data)
{
    ps3eye_context *ctx = (ps3eye_context *)user_data;

    ctx->eye->start();

    printf("w: %d, h: %d\n", ctx->eye->getWidth(), ctx->eye->getHeight());

    while (ctx->running) {
        ps3eye::PS3EYECam::updateDevices();

        if (ctx->eye->isNewFrame()) {
            ctx->buffers.next()->update(ctx->eye->getLastFramePointer(),
                    ctx->eye->getRowBytes(), ctx->eye->getWidth(),
                    ctx->eye->getHeight());
            ctx->last_frames++;
        }

        Uint32 now_ticks = SDL_GetTicks();
        if (now_ticks - ctx->last_ticks > 1000) {
            printf("FPS: %.2f\n", 1000 * ctx->last_frames / (float(now_ticks - ctx->last_ticks)));
            ctx->last_ticks = now_ticks;
            ctx->last_frames = 0;
        }
    }

    return 0;
}

int
main(int argc, char *argv[])
{
    ps3eye_context ctx(640, 480, 60);

    if (!ctx.hasDevices()) {
        printf("No devices connected.\n");
        return 1;
    }

    SDL_Thread *thread = SDL_CreateThread(ps3eye_cam_thread, (void *)&ctx);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Surface *surface = SDL_SetVideoMode(ctx.eye->getWidth(), ctx.eye->getHeight(), 0, 0);

    unsigned char *rgba = (unsigned char *)malloc(ctx.eye->getWidth() * ctx.eye->getHeight() * 4);

    SDL_Surface *rgba_surface = SDL_CreateRGBSurfaceFrom(rgba,
            ctx.eye->getWidth(), ctx.eye->getHeight(), 32,
            ctx.eye->getWidth() * 4,
            0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

    SDL_Event e;
    while (true) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                return 0;
            }
        }

        // TODO: proper thread signalling to wait for next available buffer
        SDL_Delay(10);
        yuv422_buffer_t *last = ctx.buffers.read();

        // TODO: Use OpenGL rendering instead of sw-based blitting, possibly
        // decoding the yuv422 format directly in the fragment shader, or if
        // we want to process the frame in the CPU, we could also have a
        // separate thread for converting the pixel data (and an additional
        // layer of buffering there)
        yuv422_to_rgba(last->pixels, last->stride, rgba, last->width, last->height);
        // TODO: Use RGB surface instead of RGBA surface
        SDL_BlitSurface(rgba_surface, NULL, surface, NULL);
        SDL_Flip(surface);
    }

    ctx.running = true;
    SDL_WaitThread(thread, NULL);

    return 0;
}

/**
 * PS3EYEDriver Simple SDL 2 example, using OpenGL where available.
 * Thomas Perl <m@thp.io>; 2014-01-10
 * Joseph Howse <josephhowse@nummist.com>; 2014-12-26
 **/

#include <SDL2/SDL.h>
#include "ps3eye.h"


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

    printf("Camera mode: %dx%d@%d\n", ctx->eye->getWidth(),
           ctx->eye->getHeight(), ctx->eye->getFrameRate());

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
            printf("FPS: %.2f\n", 1000 * ctx->last_frames /
                                  (float(now_ticks - ctx->last_ticks)));
            ctx->last_ticks = now_ticks;
            ctx->last_frames = 0;
        }
    }

    return EXIT_SUCCESS;
}

void
print_renderer_info(SDL_Renderer *renderer)
{
    SDL_RendererInfo renderer_info;
    SDL_GetRendererInfo(renderer, &renderer_info);
    printf("Renderer: %s\n", renderer_info.name);
}

int
main(int argc, char *argv[])
{
    ps3eye_context ctx(320, 240, 187);
    if (!ctx.hasDevices()) {
        printf("No PS3 Eye camera connected\n");
        return EXIT_FAILURE;
    }
    ctx.eye->setFlip(true); /* mirrored left-right */

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Window *window = SDL_CreateWindow(
            "PS3 Eye - SDL 2", SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
    if (window == NULL) {
        printf("Failed to create window: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
                                                SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        return EXIT_FAILURE;
    }
    SDL_RenderSetLogicalSize(renderer, ctx.eye->getWidth(),
                             ctx.eye->getHeight());
    print_renderer_info(renderer);

    SDL_Texture *video_tex = SDL_CreateTexture(
            renderer, SDL_PIXELFORMAT_YUY2, SDL_TEXTUREACCESS_STREAMING,
            ctx.eye->getWidth(), ctx.eye->getHeight());
    if (video_tex == NULL) {
        printf("Failed to create video texture: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return EXIT_FAILURE;
    }

    SDL_Thread *thread = SDL_CreateThread(ps3eye_cam_thread, "ps3eye_cam",
                                          (void *)&ctx);

    SDL_Event e;
    yuv422_buffer_t *last;
    void *video_tex_pixels;
    int pitch;
    while (ctx.running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                ctx.running = false;
            }
        }

        // TODO: Proper thread signalling to wait for next available buffer
        SDL_Delay(10);
        last = ctx.buffers.read();

        SDL_LockTexture(video_tex, NULL, &video_tex_pixels, &pitch);
        memcpy(video_tex_pixels, last->pixels, last->size);
        SDL_UnlockTexture(video_tex);

        SDL_RenderCopy(renderer, video_tex, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    SDL_WaitThread(thread, NULL);

    SDL_DestroyTexture(video_tex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return EXIT_SUCCESS;
}

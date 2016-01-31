/**
 * PS3EYEDriver Simple SDL 2 example, using OpenGL where available.
 * Thomas Perl <m@thp.io>; 2014-01-10
 * Joseph Howse <josephhowse@nummist.com>; 2014-12-26
 **/

#include <SDL.h>
#include "ps3eye.h"

struct ps3eye_context {
    ps3eye_context(int width, int height, int fps) :
          eye(0)
        , devices(ps3eye::PS3EYECam::getDevices())
        , running(true)
        , last_ticks(0)
        , last_frames(0)
    {
        if (hasDevices()) {
            eye = devices[0];
            eye->init(width, height, (uint8_t)fps);
        }
    }

    bool hasDevices()
    {
        return (devices.size() > 0);
    }

    std::vector<ps3eye::PS3EYECam::PS3EYERef> devices;
    ps3eye::PS3EYECam::PS3EYERef eye;

    bool running;
    Uint32 last_ticks;
    Uint32 last_frames;
};

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
    ps3eye_context ctx(640, 480, 60);
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

	ctx.eye->start();

	printf("Camera mode: %dx%d@%d\n", ctx.eye->getWidth(), ctx.eye->getHeight(), ctx.eye->getFrameRate());

    SDL_Event e;
    
    while (ctx.running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                ctx.running = false;
            }
        }

		uint8_t* new_pixels = ctx.eye->getFrame();

		void *video_tex_pixels;
		int pitch;
        SDL_LockTexture(video_tex, NULL, &video_tex_pixels, &pitch);
		memcpy(video_tex_pixels, new_pixels, ctx.eye->getRowBytes() * ctx.eye->getHeight());
        SDL_UnlockTexture(video_tex);

		free(new_pixels);

        SDL_RenderCopy(renderer, video_tex, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

	ctx.eye->stop();

    SDL_DestroyTexture(video_tex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return EXIT_SUCCESS;
}

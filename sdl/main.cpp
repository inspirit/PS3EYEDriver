/**
 * PS3EYEDriver Simple SDL 2 example, using OpenGL where available.
 * Thomas Perl <m@thp.io>; 2014-01-10
 * Joseph Howse <josephhowse@nummist.com>; 2014-12-26
 **/
#include <sstream>
#include <iostream>
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

void run_camera(int width, int height, int fps, Uint32 duration)
{
	ps3eye_context ctx(width, height, fps);
	if (!ctx.hasDevices()) {
		printf("No PS3 Eye camera connected\n");
		return;
	}
	ctx.eye->setFlip(true); /* mirrored left-right */

	char title[256];
	sprintf(title, "%dx%d@%d\n", ctx.eye->getWidth(), ctx.eye->getHeight(), ctx.eye->getFrameRate());

	SDL_Window *window = SDL_CreateWindow(
		title, SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, width, height, 0);
	if (window == NULL) {
		printf("Failed to create window: %s\n", SDL_GetError());
		return;
	}

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
		SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		printf("Failed to create renderer: %s\n", SDL_GetError());
		SDL_DestroyWindow(window);
		return;
	}
	SDL_RenderSetLogicalSize(renderer, ctx.eye->getWidth(), ctx.eye->getHeight());
	print_renderer_info(renderer);

	SDL_Texture *video_tex = SDL_CreateTexture(
		renderer, SDL_PIXELFORMAT_BGR24, SDL_TEXTUREACCESS_STREAMING,
		ctx.eye->getWidth(), ctx.eye->getHeight());

	if (video_tex == NULL)
	{
		printf("Failed to create video texture: %s\n", SDL_GetError());
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		return;
	}

	ctx.eye->start();

	printf("Camera mode: %dx%d@%d\n", ctx.eye->getWidth(), ctx.eye->getHeight(), ctx.eye->getFrameRate());

	SDL_Event e;

	Uint32 start_ticks = SDL_GetTicks();
	while (ctx.running) {
		if (duration != 0 && (SDL_GetTicks() - start_ticks) / 1000 >= duration)
			break;

		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT || (e.type == SDL_KEYUP && e.key.keysym.scancode == SDL_SCANCODE_ESCAPE)) {
				ctx.running = false;
			}			
		}

		{
			Uint32 now_ticks = SDL_GetTicks();

			ctx.last_frames++;

			if (now_ticks - ctx.last_ticks > 1000)
			{
				printf("FPS: %.2f\n", 1000 * ctx.last_frames / (float(now_ticks - ctx.last_ticks)));
				ctx.last_ticks = now_ticks;
				ctx.last_frames = 0;
			}
		}

		void *video_tex_pixels;
		int pitch;
		SDL_LockTexture(video_tex, NULL, &video_tex_pixels, &pitch);

		ctx.eye->getFrame((uint8_t*)video_tex_pixels);

		SDL_UnlockTexture(video_tex);

		SDL_RenderCopy(renderer, video_tex, NULL, NULL);
		SDL_RenderPresent(renderer);
	}

	ctx.eye->stop();

	SDL_DestroyTexture(video_tex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}

int
main(int argc, char *argv[])
{
	bool mode_test = false;
    int width = 640;
    int height = 480;
    int fps = 60;
    if (argc > 1)
    {
        bool good_arg = false;
        for (int arg_ix = 1; arg_ix < argc; ++arg_ix)
        {
            if (std::string(argv[arg_ix]) == "--qvga")
            {
                width = 320;
                height = 240;
                good_arg = true;
            }

            if ((std::string(argv[arg_ix]) == "--fps") && argc > arg_ix)
            {
                std::istringstream new_fps_ss( argv[arg_ix+1] );
                if (new_fps_ss >> fps)
                {
                    good_arg = true;
                }
            }

			if (std::string(argv[arg_ix]) == "--mode_test")
			{
				mode_test = true;
				good_arg = true;
			}
        }
        if (!good_arg)
        {
            std::cerr << "Usage: " << argv[0] << " [--fps XX] [--qvga] [--mode_test]" << std::endl;
        }
    }

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Failed to initialize SDL: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	if (mode_test)
	{
		int rates_qvga[] = { 15, 20, 30, 40, 50, 60, 75, 90, 100, 125, 137, 150, 187 };
		int num_rates_qvga = sizeof(rates_qvga) / sizeof(int);

		int rates_vga[] = { 15, 20, 30, 40, 50, 60, 75 };
		int num_rates_vga = sizeof(rates_vga) / sizeof(int);

		for (int index = 0; index < num_rates_qvga; ++index)
			run_camera(320, 240, rates_qvga[index], 5);

		for (int index = 0; index < num_rates_vga; ++index)
			run_camera(640, 480, rates_vga[index], 5);
	}
	else
	{
		run_camera(width, height, fps, 0);
	}

    return EXIT_SUCCESS;
}

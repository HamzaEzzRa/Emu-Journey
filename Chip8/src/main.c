#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32) || defined(WIN32)
    #include "../SDL2/include/SDL.h"
#elif defined __unix__ || defined __APPLE__
    #include <SDL2/SDL.h>
#endif

#include "../include/cpu.h"
#include "../include/gui.h"
#include "../include/sound.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: ./CHIP-8 <filename>");
        return EXIT_FAILURE;
    }

    if (!load_rom(argv[1])) {
        printf("Failed to find ROM at path %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    int ops_per_sec = 480;
    int target_fps = 60;
    int frames = ops_per_sec / target_fps;
    float interval = 1000 / target_fps;

    if (!init_gui() || !init_sound())
        return EXIT_FAILURE;

    int quit = 0;
    SDL_Event event;
    unsigned int _time = SDL_GetTicks();

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            handle_input(event);

            if (event.type == SDL_QUIT)
                quit = 1;
        }

        unsigned int current = SDL_GetTicks();

		if (_time + interval < current) {
			decrease_timers();
			for (int i = 0 ; i < frames; i++)
                execute_opcode(next_opcode());

			_time = current;
			render_frame();
		}
        SDL_Delay(1);
    }

    clear_gui();
    clear_audio();
    SDL_Quit();
    return EXIT_SUCCESS;
}

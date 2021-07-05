#include <stdio.h>

#if defined(_WIN32) || defined(WIN32)
    #include "../SDL2/include/SDL.h"
    #include "../SDL2/include/SDL_opengl.h"
#elif defined __unix__ || defined __APPLE__
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_opengl.h>
#endif

#include "../include/cpu.h"
#include "../include/gui.h"

SDL_Window *g_window = NULL;
SDL_Renderer *g_renderer = NULL;
SDL_Texture *g_texture = NULL;

int init_gui() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		return 0;
	}

    g_window = SDL_CreateWindow("Chip8 - Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (g_window == NULL) {
        printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
        return 0;
    }
	g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
	if (g_renderer == NULL) {
		printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        return 0;
	}
	g_texture = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, SCREEN_WIDTH, SCREEN_HEIGHT);
	if (g_texture == NULL) {
		printf("Texture could not be created! SDL Error: %s\n", SDL_GetError());
        return 0;
	}

    return 1;
}

void render_frame() {
	SDL_UpdateTexture(g_texture, NULL, screen_data, SCREEN_WIDTH * 3 * sizeof(*screen_data));
	SDL_RenderCopy(g_renderer, g_texture, NULL, NULL);
	SDL_RenderPresent(g_renderer);
}

void handle_input(SDL_Event event) {
	int keycode = -1;
	switch (event.key.keysym.sym) {
		case SDLK_1: { keycode = 0; break; }
		case SDLK_2: { keycode = 1; break; }
		case SDLK_3: { keycode = 2; break; }
		case SDLK_c: { keycode = 3; break; }
		case SDLK_4: { keycode = 4; break; }
		case SDLK_5: { keycode = 5; break; }
		case SDLK_6: { keycode = 6; break; }
		case SDLK_d: { keycode = 7; break; }
		case SDLK_7: { keycode = 8; break; }
		case SDLK_8: { keycode = 9; break; }
		case SDLK_9: { keycode = 10; break; }
		case SDLK_e: { keycode = 11; break; }
		case SDLK_a: { keycode = 12; break; }
		case SDLK_0: { keycode = 13; break; }
		case SDLK_b: { keycode = 14; break; }
		case SDLK_f: { keycode = 15; break; }
		default: break;
	}
	if (keycode != -1) {
		if (event.type == SDL_KEYDOWN)
			key_press(keycode);
		else if (event.type == SDL_KEYUP)
			key_release(keycode);
	}
}

void clear_gui() {
	SDL_DestroyTexture(g_texture);
  	g_texture = NULL;
	SDL_DestroyRenderer(g_renderer);
	g_renderer = NULL;
	SDL_DestroyWindow(g_window);
	g_window = NULL;
}

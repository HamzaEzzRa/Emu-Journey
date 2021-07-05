#ifndef SOUND_H
#define SOUND_H

#if defined(_WIN32) || defined(WIN32)
    #include "../SDL2/include/SDL.h"
#elif defined __unix__ || defined __APPLE__
    #include <SDL2/SDL.h>
#endif

int init_sound();
void play_sound(double volume, double pan, double phase, double freq_pitch, double wavelength_pitch);
void pause_device();
void clear_audio();

#endif // SOUND_H
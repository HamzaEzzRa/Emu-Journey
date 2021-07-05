#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#if defined(_WIN32) || defined(WIN32)
    #include "../SDL2/include/SDL.h"
#elif defined __unix__ || defined __APPLE__
    #include <SDL2/SDL.h>
#endif

#include "../include/memory.h"
#include "../include/sound.h"

const double CHROMATIC_RATIO = 1.059463094359295264562;
const double TAO = 6.283185307179586476925;
const Uint32 FRAME_RATE = 60;
const double IS_SILENT = 0.001;

Uint32 sample_rate = 44100;
Uint32 samples_per_frame;
Uint32 float_stream_length = 1024;
Uint32 ms_per_frame;
Uint32 audio_buffer_length = 44100;
float *audio_buffer;

SDL_atomic_t audio_callback_left_off;
Sint32 audio_main_left_off;
Uint8 audio_main_accumulator;

SDL_AudioDeviceID audio_device;
SDL_AudioSpec audio_spec;

SDL_bool running;

typedef struct {
    float *waveform;
    Uint32 waveform_length;
    double volume;
    double pan;
    double frequency;
    double phase;
} Sound;

void prepare(Sound *s) {
    float sample;
    Uint32 sourceIndex;
    double phaseIncrement = s->frequency / sample_rate;
    Uint32 i;
    if (s->volume > IS_SILENT) {
        for (i = 0; i + 1 < samples_per_frame; i += 2) {
            s->phase += phaseIncrement;
            if (s->phase > 1)
                s->phase -= 1;

            sourceIndex = s->phase * s->waveform_length;
            sample = s->waveform[sourceIndex] * s->volume;

            audio_buffer[audio_main_left_off + i] += sample*(1 - s->pan);
            audio_buffer[audio_main_left_off + i + 1] += sample * s->pan;
        }
    }
    else {
        for (i = 0; i < samples_per_frame; i += 1)
            audio_buffer[audio_main_left_off + i] = 0;
    }
    audio_main_accumulator++;
}

double get_frequency(double pitch) {
    return pow(CHROMATIC_RATIO, pitch - 57) * 440;
}

int get_waveform_length(double pitch) {
    return sample_rate / get_frequency(pitch) + 0.5f;
}

void build_sine_wave(float *data, Uint32 length) {
    Uint32 i;
    for (i = 0; i < length; i++)
        data[i] = sin(i * (TAO / length));
}

void audio_callback(void *unused, Uint8 *byte_stream, int byte_stream_length) {
    float *float_stream = (float*) byte_stream;
    Sint32 local_audio_callback_left_off = SDL_AtomicGet(&audio_callback_left_off);
    for (Uint32 i = 0; i < float_stream_length; i++) {
        float_stream[i] = audio_buffer[local_audio_callback_left_off];
        local_audio_callback_left_off++;
        if (local_audio_callback_left_off == audio_buffer_length)
            local_audio_callback_left_off = 0;
    }
    SDL_AtomicSet(&audio_callback_left_off, local_audio_callback_left_off);
    sound_timer--;
}

int init_sound() {
    SDL_Init(SDL_INIT_AUDIO);
    SDL_AudioSpec want;
    SDL_zero(want);

    want.freq = sample_rate;
    want.format = AUDIO_F32;
    want.channels = 1;
    want.samples = float_stream_length;
    want.callback = audio_callback;

    audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &audio_spec, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if (!audio_device) {
        printf("Failed to open audio: %s\n", SDL_GetError());
        return 0;
    }

    if (audio_spec.format != want.format) {
        printf("Couldn't get Float32 audio format.\n");
        return -1;
    }

    sample_rate = audio_spec.freq;
    float_stream_length = audio_spec.size / 4;
    samples_per_frame = sample_rate / FRAME_RATE;
    ms_per_frame = 1000 / FRAME_RATE;
    audio_main_left_off = samples_per_frame * 8;
    SDL_AtomicSet(&audio_callback_left_off, 0);

    if (audio_buffer_length % samples_per_frame)
        audio_buffer_length += samples_per_frame - (audio_buffer_length % samples_per_frame);
    audio_buffer = malloc(sizeof(float) * audio_buffer_length);

    return 1;
}

void play_sound(double volume, double pan, double phase, double freq_pitch, double wavelength_pitch) {
    Sint32 main_audio_lead;

    Sound sound;
    sound.volume = volume;
    sound.pan = pan;
    sound.phase = phase;
    sound.frequency = get_frequency(freq_pitch);
    Uint16 C0_waveform_length = get_waveform_length(wavelength_pitch);
    sound.waveform_length = C0_waveform_length;
    float *sine_wave = malloc(sizeof(sine_wave) * C0_waveform_length);
    build_sine_wave(sine_wave, C0_waveform_length);
    sound.waveform = sine_wave;

    running = SDL_TRUE;
    SDL_PauseAudioDevice(audio_device, 0);
    for (Uint32 i = 0; i < samples_per_frame; i++)
        audio_buffer[audio_main_left_off + i] = 0;
    prepare(&sound);
    if (audio_main_accumulator > 1) {
        for (Uint32 i = 0; i < samples_per_frame; i++)
            audio_buffer[audio_main_left_off + i] /= audio_main_accumulator;
    }
    audio_main_accumulator = 0;
    audio_main_left_off += samples_per_frame;
    if (audio_main_left_off == audio_buffer_length)
        audio_main_left_off = 0;
    main_audio_lead = audio_main_left_off - SDL_AtomicGet(&audio_callback_left_off);
    if (main_audio_lead < 0)
        main_audio_lead += audio_buffer_length;
    if (main_audio_lead < float_stream_length)
        pause_device();
}

void pause_device() {
    if (running) {
        SDL_PauseAudioDevice(audio_device, 1);
        running = SDL_FALSE;
    }
}

void clear_audio() {
    SDL_CloseAudioDevice(audio_device);
}

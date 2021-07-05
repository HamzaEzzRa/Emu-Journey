#ifndef CPU_H
#define CPU_H

#include "memory.h"

static const int SCREEN_WIDTH = 640;
static const int SCREEN_HEIGHT = 320;

BYTE *screen_data;

int load_rom(const char* const path_to_rom);

WORD next_opcode();

void execute_opcode(WORD opcode);

void key_press(int keycode);

void key_release(int keycode);

void decrease_timers();

#endif // CPU_H

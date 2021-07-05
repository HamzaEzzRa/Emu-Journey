#ifndef MEMORY_H
#define MEMORY_H

#include "stack.h"

static const int ROM_SIZE = 0xFFF;

typedef unsigned char BYTE;
typedef unsigned short int WORD;

BYTE game_memory[0xFFF]; // Total memory of 0xFFF bytes
BYTE data_registers[0x10]; // 16 8-bit registers (V0 to VF)
WORD address_register; // 16-bit address register (I)
WORD program_counter; // 16-bit PC
Stack *stack_ptr;

BYTE key_state[0x10];
BYTE delay_timer;
BYTE sound_timer;

#endif // MEMORY_H

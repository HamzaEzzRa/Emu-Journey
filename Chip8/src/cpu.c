#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/memory.h"
#include "../include/cpu.h"
#include "../include/stack.h"
#include "../include/sound.h"

unsigned char fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void reset_cpu() {
    srand(time(0));
    if (stack_ptr != NULL)
        free_stack(stack_ptr);
    
    stack_ptr = malloc(sizeof(Stack));
    init_stack(stack_ptr);

    address_register = 0;
    program_counter = 0x200;
    memset(data_registers, 0, sizeof(data_registers));
    memset(game_memory, 0, sizeof(game_memory));
    memset(key_state, 0, sizeof(key_state));
	delay_timer = 0;
	sound_timer = 0;

    int fontset_count = sizeof(fontset) / sizeof(fontset[0]);
    for (int i = 0; i < fontset_count; i++)
		game_memory[i] = fontset[i];
}

void clear_display() {
    for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH * 3; i++)
        screen_data[i] = 0x0;
}

int load_rom(const char* const path_to_rom) {
    reset_cpu();

    screen_data = malloc(sizeof(*screen_data) * SCREEN_HEIGHT * SCREEN_WIDTH * 3);

    clear_display();

    FILE *rom;
    if (!(rom = fopen(path_to_rom, "rb")))
        return 0;

    fread(game_memory + 0x200, ROM_SIZE, 1, rom);
    fclose(rom);

    return 1;
}

void key_press(int keycode) {
    key_state[keycode] = 1;
}

void key_release(int keycode) {
    key_state[keycode] = 0;
}

void decrease_timers() {
    if (delay_timer > 0)
        delay_timer--;

    if (sound_timer > 0)
        play_sound(1, 0.5, 0, 70.5, 16); // sound_timer is decreased inside the audio_callback
    if (sound_timer <= 0)
        pause_device();
}

// Clear the display screen
void op_00E0() {
    clear_display();
}

// Returns from a subroutine
void op_00EE() {
    WORD *address = (WORD*)stack_pop(stack_ptr);
    if (address != NULL)
        program_counter = *address;
}

// Jump to address NNN
void op_1NNN(WORD opcode) {
    program_counter = opcode & 0x0FFF;
}

// Call subroutine at NNN
void op_2NNN(WORD opcode) {
    stack_push(stack_ptr, &program_counter);
	program_counter = opcode & 0x0FFF;
}

 // Skip the next instruction if VX == NN
void op_3XNN(WORD opcode) {
    int nn = opcode & 0x00FF;
    int x = (opcode & 0x0F00) >> 8;

    if (data_registers[x] == nn)
        program_counter += 2;
}

// Skip the next instruction if VX != NN
void op_4XNN(WORD opcode) {
    int nn = opcode & 0x00FF;
    int x = (opcode & 0x0F00) >> 8;

    if (data_registers[x] != nn)
        program_counter += 2;
}

// Skip the next instruction if VX == VY
void op_5XY0(WORD opcode) {
    int x = (opcode & 0x0F00) >> 8;
    int y = (opcode & 0x00F0) >> 4;

    if (data_registers[x] == data_registers[y])
        program_counter += 2;
}

// Set VX to NN
void op_6XNN(WORD opcode) {
    int nn = opcode & 0x00FF;
    int x = (opcode & 0x0F00) >> 8;

    data_registers[x] = nn;
}

// Add NN to VX with no carry
void op_7XNN(WORD opcode) {
    int nn = opcode & 0x00FF;
    int x = (opcode & 0x0F00) >> 8;

    data_registers[x] += nn;
}

// Set VX = VY
void op_8XY0(WORD opcode) {
    int x = (opcode & 0x0F00) >> 8;
    int y = (opcode & 0x00F0) >> 4;

    data_registers[x] = data_registers[y];
}

// Set VX = VX | VY
void op_8XY1(WORD opcode) {
    int x = (opcode & 0x0F00) >> 8;
    int y = (opcode & 0x00F0) >> 4;

    data_registers[x] |= data_registers[y];
}

// Set VX = VX & VY
void op_8XY2(WORD opcode) {
    int x = (opcode & 0x0F00) >> 8;
    int y = (opcode & 0x00F0) >> 4;

    data_registers[x] &= data_registers[y];
}

// Set VX = VX ^ VY
void op_8XY3(WORD opcode) {
    int x = (opcode & 0x0F00) >> 8;
    int y = (opcode & 0x00F0) >> 4;

    data_registers[x] ^= data_registers[y];
}

// Add VY to VX with VF set to 1 if carry else 0
void op_8XY4(WORD opcode) {
    int x = (opcode & 0x0F00) >> 8;
    int y = (opcode & 0x00F0) >> 4;

    int value = data_registers[x] + data_registers[y];
    if (value > 0xFF)
        data_registers[0xF] = 1;

    data_registers[x] = value;
}

// Substract VY from VX with VF set to 0 if borrow else 1
void op_8XY5(WORD opcode) {
    int x = (opcode & 0x0F00) >> 8;
    int y = (opcode & 0x00F0) >> 4;

    data_registers[0xF] = 1;
    if (data_registers[x] < data_registers[y])
        data_registers[0xF] = 0;

    data_registers[x] -= data_registers[y];
}

// Store the least significant bit of VX in VF and shift VX by 1 to the right
void op_8XY6(WORD opcode) {
    int x = (opcode & 0x0F00) >> 8;

	data_registers[0xF] = data_registers[x] & 0x1;
	data_registers[x] >>= 1;
}

// Set VX to VY minus VX with VF set to 0 if borrow else 1
void op_8XY7(WORD opcode) {
    int x = (opcode & 0x0F00) >> 8;
    int y = (opcode & 0x00F0) >> 4;

    data_registers[0xF] = 1;
    if (data_registers[x] > data_registers[y])
        data_registers[0xF] = 0;

    data_registers[x] = data_registers[y] - data_registers[x];
}

// Store the most significant bit of VX in VF and shift VX by 1 to the left
void op_8XYE(WORD opcode) {
    int x = (opcode & 0x0F00) >> 8;

    data_registers[0xF] = data_registers[x] >> 7;
    data_registers[x] <<= 1;
}

 // Skip the next instruction if VX != VY
void op_9XY0(WORD opcode) {
    int x = (opcode & 0x0F00) >> 8;
    int y = (opcode & 0x00F0) >> 4;

    if (data_registers[x] != data_registers[y])
        program_counter += 2;
}

// Set address register to NNN
void op_ANNN(WORD opcode) {
    address_register = opcode & 0x0FFF;
}

// Jump to the address NNN + V0
void op_BNNN(WORD opcode) {
    program_counter = (opcode & 0x0FFF) + data_registers[0];
}

// Set VX = rnd(0, 255) & NN
void op_CXNN(WORD opcode) {
    int nn = opcode & 0x00FF;
	int x = (opcode & 0x0F00) >> 8;

	data_registers[x] = rand() & nn; // No need to modulo 255 because of the bitwise &
}

// Draw a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N+1 pixels
// Each row of 8 pixels is read as bit-coded starting from memory location (which value does not change after the execution of this instruction)
// VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, else VF is set to 0
void op_DXYN(WORD opcode) {
    const int SCALE = 10;

    int reg_x = (opcode & 0x0F00) >> 8;
    int reg_y = (opcode & 0x00F0) >> 4;

    int height = opcode & 0x000F;
    int coord_x = data_registers[reg_x];
    int coord_y = data_registers[reg_y];

    data_registers[0xF] = 0;

    for (int y_line = 0; y_line < height; y_line++) {
        BYTE data = game_memory[address_register + y_line];
        int x_pixel_inv = 7;
        int x_pixel = 0;
        for (x_pixel = 0; x_pixel < 8; x_pixel++, x_pixel_inv--) {
            int mask = 1 << x_pixel_inv;
            if (data & mask) {
                int x = (coord_x + x_pixel) * SCALE;
                int y = (coord_y + y_line) * SCALE;
                
                int color = 0xFF;
				if (screen_data[(y*SCREEN_WIDTH + x) * 3] == 0xFF) {
					color = 0;
					data_registers[0xF] = 1;
				}

				for (int i = 0; i < SCALE; i++) {
					for (int j = 0; j < SCALE; j++) {
						screen_data[((y+i)*SCREEN_WIDTH + x+j) * 3] = color;
						screen_data[((y+i)*SCREEN_WIDTH + x+j) * 3 + 1] = color;
						screen_data[((y+i)*SCREEN_WIDTH + x+j) * 3 + 2] = color;
					}
				}
            }
        }
    }
}

// Skip the next instruction if the key stored in VX is pressed
void op_EX9E(WORD opcode) {
    int x = (opcode & 0x0F00) >> 8;
    int key = data_registers[x];

    if (key_state[x])
        program_counter += 2;
}

// Skip the next instruction if the key stored in VK is not pressed
void op_EXA1(WORD opcode) {
	int x = (opcode & 0x0F00) >> 8;
	int key = data_registers[x];

	if (!key_state[key])
	    program_counter += 2;
}

// Set VX to the value of the delay timer
void op_FX07(WORD opcode) {
    int x = (opcode & 0x0F00) >> 8;
    data_registers[x] = delay_timer;
}

// Wait for key press and then store its value in VX. (All instruction halted until next key event)
void op_FX0A(WORD opcode) {
    int x = (opcode & 0x0F00) >> 8;

    for (int i = 0; i <= 0xF; i++) {
        if (key_state[i]) {
            data_registers[x] = i;
            return;
        }
    }

    program_counter -= 2;
}

// Set the delay timer to VX
void op_FX15(WORD opcode) {
	int x = (opcode & 0x0F00) >> 8;
	delay_timer = data_registers[x];
}

// Set the sound timer to VX
void op_FX18(WORD opcode) {
    int x = (opcode & 0x0F00) >> 8;
    sound_timer = data_registers[x];
}

// Add VX to address register value. VF not affected by this OP (only "Spacefight 2091!" depends on it being affected)
void op_FX1E(WORD opcode) {
    int x = (opcode & 0x0F00) >> 8;
    address_register += data_registers[x];
}

// Set address register to the location of sprite for the char in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font
void op_FX29(WORD opcode) {
    int x = (opcode & 0x0F00) >> 8;
    address_register = data_registers[x] * 5;
}

// Store the binary-coded decimal representation of VX, with the most significant of three digits at the address in address register,
// the middle digit at address register plus 1, and the least significant digit at address register plus 2
// (In other words, take the decimal representation of VX, place the hundreds digit in memory at location in address register,
// the tens digit at location address register + 1, and the ones digit at location address register + 2)
void op_FX33(WORD opcode) {
    int x = (opcode & 0x0F00) >> 8;
    
    game_memory[address_register] = data_registers[x] / 100;
    game_memory[address_register + 1] = (data_registers[x] % 100) / 10;
    game_memory[address_register + 2] = data_registers[x] % 10;
}

// Store V0 to VX (including VX) in memory starting at address register
// In the original CHIP-8 implementation, and also in CHIP-48, address register is left incremented after this instruction had been executed.
// In SCHIP, address register is left unmodified. (I choose to follow the SCHIP way)
void op_FX55(WORD opcode) {
    int x = (opcode & 0x0F00) >> 8;
    
    for (int i = 0; i <= x; i++)
        game_memory[address_register + i] = data_registers[i];
}

// Fills V0 to VX (including VX) with values from memory starting at address I.
// In the original CHIP-8 implementation, and also in CHIP-48, address register is left incremented after this instruction had been executed.
// In SCHIP, address register is left unmodified. (I choose to follow the SCHIP way)
void op_FX65(WORD opcode) {
    int x = (opcode & 0x0F00) >> 8;
    
    for (int i = 0; i <= x; i++)
        data_registers[i] = game_memory[address_register + i];
}

WORD next_opcode() {
    WORD next = game_memory[program_counter++]; // Get first byte and increase counter
    next <<= 8; // Make space for the second byte
    next |= game_memory[program_counter++]; // Combine second byte and increase counter

    return next;
}

void decode_op_0(WORD opcode) {
    switch (opcode & 0xF) {
        case 0x0:
            op_00E0();
            break;
        case 0xE:
            op_00EE();
            break ;
        default: break ;
	}
}

void decode_op_8(WORD opcode) {
	switch (opcode & 0xF) {
		case 0x0:
            op_8XY0(opcode);
            break;
		case 0x1:
            op_8XY1(opcode);
            break;
		case 0x2:
            op_8XY2(opcode);
            break;
		case 0x3:
            op_8XY3(opcode);
            break;
		case 0x4:
            op_8XY4(opcode);
            break;
		case 0x5:
            op_8XY5(opcode);
            break;
		case 0x6:
            op_8XY6(opcode);
            break;
		case 0x7:
            op_8XY7(opcode);
            break;
		case 0xE:
            op_8XYE(opcode);
            break;
		default:
            break;
	}
}

void decode_op_E(WORD opcode) {
	switch(opcode & 0xF) {
		case 0xE:
            op_EX9E(opcode);
            break;
		case 0x1:
            op_EXA1(opcode);
            break;
		default:
            break;
	}
}

void decode_op_F(WORD opcode) {
    switch(opcode & 0xFF) {
		case 0x07:
            op_FX07(opcode);
            break;
		case 0x0A:
            op_FX0A(opcode);
            break;
		case 0x15:
            op_FX15(opcode);
            break;
		case 0x18:
            op_FX18(opcode);
            break;
		case 0x1E:
            op_FX1E(opcode);
            break;
		case 0x29:
            op_FX29(opcode);
            break;
		case 0x33:
            op_FX33(opcode);
            break;
		case 0x55:
            op_FX55(opcode);
            break;
		case 0x65:
            op_FX65(opcode);
            break;
		default:
            break;
	}
}

void execute_opcode(WORD opcode) {
    // if (opcode != 0)
    //     printf("%X\n", opcode);
    switch (opcode & 0xF000) {
		case 0x0000:
            decode_op_0(opcode);
            break;
		case 0x1000:
            op_1NNN(opcode);
            break;
		case 0x2000:
            op_2NNN(opcode);
            break;
		case 0x3000:
            op_3XNN(opcode);
            break;
		case 0x4000:
            op_4XNN(opcode);
            break;
		case 0x5000: 
            op_5XY0(opcode); 
            break;
		case 0x6000: 
            op_6XNN(opcode); 
            break;
		case 0x7000: 
            op_7XNN(opcode); 
            break;
		case 0x8000: 
            decode_op_8(opcode); 
            break;
		case 0x9000: 
            op_9XY0(opcode); 
            break;
		case 0xA000: 
            op_ANNN(opcode); 
            break;
		case 0xB000: 
            op_BNNN(opcode); 
            break;
		case 0xC000: 
            op_CXNN(opcode); 
            break;
		case 0xD000: 
            op_DXYN(opcode); 
            break;
		case 0xE000: 
            decode_op_E(opcode); 
            break;
		case 0xF000: 
            decode_op_F(opcode); 
            break;
		default:
            break;
	}
}

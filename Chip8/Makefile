OBJS = ./src/main.c ./src/cpu.c ./src/gui.c ./src/stack.c ./src/sound.c

CC = gcc

# Windows Dependant Variables
ifeq ($(OS), Windows_NT)
	SDL_SYS_FLAGS := -IC:./SDL2/include -LC:./SDL2/lib
	SDL_LINKER_FLAGS := -lmingw32 -lSDL2main -lSDL2

# OS X/GNU/Linux Dependant Variables
else
	SDL_SYS_FLAGS := -w $(shell sdl2-config --cflags)
	SDL_LINKER_FLAGS := $(shell sdl2-config --libs) -lSDL2main -lSDL2
endif

OBJ_NAME = CHIP-8

all: $(OBJS)
	$(CC) $(OBJS) $(SDL_SYS_FLAGS) $(SDL_LINKER_FLAGS) -o $(OBJ_NAME)

clean:
	rm -r $(OBJ_NAME).exe

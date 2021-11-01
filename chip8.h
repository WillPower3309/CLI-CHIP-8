#include <stdint.h>

// hardware constants
#define DISPLAY_X 64
#define DISPLAY_Y 32
#define MEMORY_SIZE 4096 // CHIP-8 has up to 4 kilobytes of RAM
#define NUM_GENERAL_REGISTERS 16 // 16 registers - 0 through F
#define STACK_SIZE 16
#define MAX_GAME_SIZE (0x1000 - 0x200)

// emulator has 16 potential key inputs
#define NUM_KEYS 16

// hexadecimal macros
#define X(num) ((num >> 8) & 0x000F) // second nibble
#define Y(num) ((num >> 4) & 0x000F) // third nibble
#define N(num) (num & 0x000F) // fourth nibble
#define NN(num) (num & 0x00FF) // second byte (third and fourth nibbles)
#define NNN(num) (num & 0x0FFF) // second, third, and fourth nibbles

// print unknown opcode error message and exit
void unknownOpcode(uint16_t opcode);

// open and initialize a ROM given its path
void initRom(char *game);

// draw a sprite to the display variable
void draw(uint8_t xCoordinate, uint8_t yCoordinate, uint8_t spriteHeight);

// decode and execute the opcode instruction
void execute(uint16_t opcode);

void tick();

void initC8();
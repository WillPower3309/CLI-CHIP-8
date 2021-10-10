//   ██████╗██╗     ██╗     ██████╗██╗  ██╗██╗██████╗        █████╗
//  ██╔════╝██║     ██║    ██╔════╝██║  ██║██║██╔══██╗      ██╔══██╗
//  ██║     ██║     ██║    ██║     ███████║██║██████╔╝█████╗╚█████╔╝
//  ██║     ██║     ██║    ██║     ██╔══██║██║██╔═══╝ ╚════╝██╔══██╗
//  ╚██████╗███████╗██║    ╚██████╗██║  ██║██║██║           ╚█████╔╝
//   ╚═════╝╚══════╝╚═╝     ╚═════╝╚═╝  ╚═╝╚═╝╚═╝            ╚════╝

////////////////////////////////////////////////////////////////////
//  Imports & Definitions
////////////////////////////////////////////////////////////////////

// imports
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// hardware constants
#define DISPLAY_X 64
#define DISPLAY_Y 32
#define MEMORY_SIZE 4096 // CHIP-8 has up to 4 kilobytes of RAM
#define NUM_GENERAL_REGISTERS 16 // 16 registers - 0 through F
#define MAX_GAME_SIZE (0x1000 - 0x200)

// hexadecimal macros
#define X(num) ((num >> 8) & 0x000F) // second nibble
#define Y(num) ((num >> 4) & 0x000F) // third nibble
#define N(num) (num & 0x000F) // fourth nibble
#define NN(num) (num & 0x00FF) // second byte (third and fourth nibbles)
#define NNN(num) (num & 0x0FFF) // second, third, and fourth nibbles

////////////////////////////////////////////////////////////////////
//  Global Vars
////////////////////////////////////////////////////////////////////

// CHIP-8 Hardware 
uint8_t  memory[MEMORY_SIZE]; // emulated RAM
uint16_t PC; // program counter - points to current instruction in memory
uint16_t I; // register used to point at location in memory
uint8_t  V[NUM_GENERAL_REGISTERS]; // general purpose registers
bool     display[DISPLAY_X][DISPLAY_Y]; // monochrome display (1 = white, 0 = black)

////////////////////////////////////////////////////////////////////
//  Functionality
////////////////////////////////////////////////////////////////////

// print unknown opcode error message and exit
void unknownOpcode(uint16_t opcode) {
    printf("ERROR: unknown opcode 0x%X\n", opcode);
    exit(0);
}

// open and initialize a ROM given its path
void initRom(char *game) {
    FILE *fgame = fopen(game, "rb");

    // check if the game file exists
    if (!fgame) {
        printf("Error! Rom %s does not exist.\n", game);
        exit(0);
    }

    // read game into memory
    fread(&memory[0x200], 1, MAX_GAME_SIZE, fgame);
    fclose(fgame);
}

// draw a sprite to the display
// TODO: fix me!
void draw(uint8_t xCoordinate, uint8_t yCoordinate, uint8_t spriteHeight)  {
    // collision register defaults to 0
    V[0xF] = 0;

    for (uint8_t spriteByteIndex = 0; spriteByteIndex < spriteHeight; spriteByteIndex++) {
        // get sprite byte, counting from the I register
        uint8_t spriteByte = memory[I + spriteByteIndex];

        // iterate through each bit in the sprite byte
        for (uint8_t spriteBitIndex = 0; spriteBitIndex < 8; spriteBitIndex++) {
            if(((spriteByte >> spriteBitIndex) & 0x1) == 1) {
                bool *pixelVal = &display[yCoordinate + spriteByteIndex][xCoordinate + (7 - spriteBitIndex)];
                // set collision register to 1 if any pixels become turned off
                if(*pixelVal == 1) {
                    V[0xF] = 1;
                }
                // set pixel val to pixel val XOR 1
                *pixelVal = *pixelVal ^ 1;
            }
        }
    }
 
    // present the updated display on the console
    for (int i = 0; i < DISPLAY_Y; i++) {
        for (int j = 0; j < DISPLAY_X; j++) {
            if (display[i][j])
                printf("0");
            else
                printf(" ");
        }
        printf("\n");
    }
}

// decode and execute the opcode instruction
void execute(uint16_t opcode) {
    // executed instruction depends on highest order byte
    switch (opcode & 0xF000) {
        // first byte is 0
        case 0x0000:
            switch (opcode & 0x0FF0) {
                case 0x00E0: // clear screen
                    memset(display, 0, sizeof(bool) * (DISPLAY_X * DISPLAY_Y));
                    break;
                default:
                    unknownOpcode(opcode);
            }
            break;
        case 0x1000: // jump to address NNN
            PC = NNN(opcode);
            break;
        case 0x3000: // skip if VX == NN
            if (V[X(opcode)] == NN(opcode)) {
                PC += 2;
            }
            break;
        case 0x4000: // skip if VX != NN
            if (V[X(opcode)] == NN(opcode)) {
                PC += 2;
            }
            break;
        case 0x5000: // skip if VX == VY
            if (V[X(opcode)] == V[Y(opcode)]) {
                PC += 2;
            }
            break;
        case 0x6000: // set register VX to NN
            V[X(opcode)] = NN(opcode);
            break;
        case 0x7000: // add NN to register VX
            V[X(opcode)] += NN(opcode);
            break;
        case 0x9000: // skip if VX != VY
            if (V[X(opcode)] != V[Y(opcode)]) {
                PC += 2;
            }
            break;
        case 0xA000: // set index register I to NNN
            I = NNN(opcode);
            break;
        case 0xD000: // display / draw XYN
            draw(V[X(opcode)], V[Y(opcode)], N(opcode));
            break;
        default:
            unknownOpcode(opcode);
    }
}

int main(int argc, char *argv[]) {
    // Handle input arguments
    if (argc != 2) {
        printf("Please specify an input ROM.\n");
        return 0;
    }

    // initialize global vars
    PC = 0x200;
    I = 0;
    memset(memory, 0, sizeof(uint8_t) * MEMORY_SIZE);
    memset(V, 0, sizeof(uint8_t) * NUM_GENERAL_REGISTERS);
    memset(display, 0, sizeof(bool) * (DISPLAY_X * DISPLAY_Y));

    initRom(argv[1]);

    uint16_t opcode = 0;

    // main process loop
    while (1) {
        // read the instruction that the PC is currently pointing to & increment PC
        opcode = memory[PC] << 8 | memory[PC + 1];
        PC += 2;

        // execute the operation
        execute(opcode);
    }
}

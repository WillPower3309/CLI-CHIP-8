//   ██████╗██╗     ██╗     ██████╗██╗  ██╗██╗██████╗        █████╗
//  ██╔════╝██║     ██║    ██╔════╝██║  ██║██║██╔══██╗      ██╔══██╗
//  ██║     ██║     ██║    ██║     ███████║██║██████╔╝█████╗╚█████╔╝
//  ██║     ██║     ██║    ██║     ██╔══██║██║██╔═══╝ ╚════╝██╔══██╗
//  ╚██████╗███████╗██║    ╚██████╗██║  ██║██║██║           ╚█████╔╝
//   ╚═════╝╚══════╝╚═╝     ╚═════╝╚═╝  ╚═╝╚═╝╚═╝            ╚════╝

////////////////////////////////////////////////////////////////////
//  Imports & Definitions
////////////////////////////////////////////////////////////////////

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
//  Global CHIP-8 Hardware Vars
////////////////////////////////////////////////////////////////////

bool     display[DISPLAY_X][DISPLAY_Y]; // monochrome display (1 = white, 0 = black)
uint8_t  memory[MEMORY_SIZE]; // emulated RAM
uint8_t  V[NUM_GENERAL_REGISTERS]; // general purpose registers
uint16_t PC; // program counter - points to current instruction in memory
uint16_t I; // register used to point at locations in memory

////////////////////////////////////////////////////////////////////
//  Functionality
////////////////////////////////////////////////////////////////////

// print unknown opcode error message and exit
void unknownOpcode(uint16_t opcode) {
    printf("unknown opcode 0x%X\n", opcode);
    exit(0);
}

// open and initialize a ROM given its path
void initRom(char *game) {
    FILE *fgame = fopen(game, "rb");

    if (!fgame) {
        printf("Error! Rom %s does not exist.\n", game);
        exit(0);
    }

    // read game into memory
    fread(&memory[0x200], 1, MAX_GAME_SIZE, fgame);
    fclose(fgame);
}

// decode and execute the opcode instruction
void execute(uint16_t opcode) {
    // executed instruction depends on highest order byte
    switch (opcode & 0xF000) {
        // first byte is 0
        case 0x0000:
            switch (opcode & 0x0FF0) {
                case 0x00E0: // clear screen
                    memset(display, 0, sizeof display);
                    break;
                default:
                    unknownOpcode(opcode);
            }
            break;
        case 0x1000: // jump to address NNN
            printf("jump to %d\n", NNN(opcode));
            // PC = NNN;
            break;
        case 0x6000: // set register VX to NN
            printf("set register\n");
            V[X(opcode)] = NN(opcode);
            break;
        case 0x7000: // add NN to register VX
            printf("add to register\n");
            V[X(opcode)] += NN(opcode);
            break;
        case 0xA000: // set index register I to NNN
            printf("set index register\n");
            I = NNN(opcode);
            break;
        case 0xD000: // display / draw XYN
            printf("draw\n");
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
    initRom(argv[1]);
    
    uint16_t opcode;

    // main process loop
    while (1) {
        // read the instruction that the PC is currently pointing to
        opcode = memory[PC] << 8 | memory[PC + 1];
        PC += 2;

        // execute the opcode
        execute(opcode);
    }
}

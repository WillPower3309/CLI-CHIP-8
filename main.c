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

#define MEMORY_SIZE 4096
#define MAX_GAME_SIZE (0x1000 - 0x200)

////////////////////////////////////////////////////////////////////
//  Global Vars
////////////////////////////////////////////////////////////////////

// 64 x 32 monochrome display (1 = white, 0 = black)
bool display[64][32];

uint8_t memory[MEMORY_SIZE];
uint16_t PC;
uint16_t opcode;

////////////////////////////////////////////////////////////////////
//  Functionality
////////////////////////////////////////////////////////////////////

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
void execute() {
    switch (opcode & 0xF000) {
        default:
            printf("unknown opcode 0x%x\n", opcode);
            exit(0);
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

    // main process loop
    while (1) {
        // read the instruction that the PC is currently pointing to
        opcode = memory[PC] << 8 | memory[PC + 1];
        PC += 2;

        // execute the opcode
        execute();
    }
}

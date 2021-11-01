//   ██████╗██╗     ██╗     ██████╗██╗  ██╗██╗██████╗        █████╗
//  ██╔════╝██║     ██║    ██╔════╝██║  ██║██║██╔══██╗      ██╔══██╗
//  ██║     ██║     ██║    ██║     ███████║██║██████╔╝█████╗╚█████╔╝
//  ██║     ██║     ██║    ██║     ██╔══██║██║██╔═══╝ ╚════╝██╔══██╗
//  ╚██████╗███████╗██║    ╚██████╗██║  ██║██║██║           ╚█████╔╝
//   ╚═════╝╚══════╝╚═╝     ╚═════╝╚═╝  ╚═╝╚═╝╚═╝            ╚════╝

////////////////////////////////////////////////////////////////////
//  Imports & Definitions
////////////////////////////////////////////////////////////////////


#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <locale.h>


////////////////////////////////////////////////////////////////////
//  Global Vars
////////////////////////////////////////////////////////////////////


bool key[NUM_KEYS]; // given key is pressed if its hex index is true

bool ORIGINAL_FORMAT; // 0: post CHIP-48 instructions, 1: original operations

bool drawFlag; // if true, ncurses will update the CLI

// CHIP-8 Hardware 
uint8_t  memory[MEMORY_SIZE]; // emulated RAM
uint16_t PC; // program counter - points to current instruction in memory
uint16_t I; // register used to point at location in memory
uint8_t  V[NUM_GENERAL_REGISTERS]; // general purpose registers
uint16_t stack[STACK_SIZE];
short    stackPtr; // points to top of stack
uint8_t  delayTimer;
uint8_t  soundTimer;
bool     display[DISPLAY_Y][DISPLAY_X]; // monochrome display (1 = white, 0 = black)


////////////////////////////////////////////////////////////////////
//  Functions
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


// draw a sprite to the display variable
void draw(uint8_t xCoordinate, uint8_t yCoordinate, uint8_t spriteHeight) {
    // take mod bounds to account for overflow
    uint8_t x = xCoordinate % DISPLAY_X;
    uint8_t y = yCoordinate % DISPLAY_Y;

    // collision register VF defaults to 0
    V[0xF] = 0;

    for (uint8_t spriteByteIndex = 0; spriteByteIndex < spriteHeight; spriteByteIndex++) {
        // get sprite byte, counting from the I register
        uint8_t spriteByte = memory[I + spriteByteIndex];

        // iterate through each bit in the sprite byte
        for (uint8_t spriteBitIndex = 0; spriteBitIndex < 8; spriteBitIndex++) {
            if(((spriteByte >> spriteBitIndex) & 0x1) == 1) {
                bool *pixelVal = &display[y + spriteByteIndex][x + (7 - spriteBitIndex)];
                // set collision register VF to 1 if any pixels become turned off
                if(*pixelVal == 1) {
                    V[0xF] = 1;
                }
                // set pixel val to pixel val XOR 1
                *pixelVal ^= 1;
            }

            // check if right edge reached
            if (x == DISPLAY_X - 1) {
                break;
            }
        }

        // check if bottom edge reached
        if (y + spriteByteIndex == DISPLAY_Y - 1) {
            break;
        }
    }
 
    drawFlag = 1;
}


// decode and execute the opcode instruction
void execute(uint16_t opcode) {
    // executed instruction depends on highest order (first) byte
    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x00FF) {
                case 0x00E0: // clear screen
                    memset(display, 0, sizeof(bool) * (DISPLAY_X * DISPLAY_Y));
                    drawFlag = 1;
                    break;
                case 0x00EE: // return from subroutine
                    // pop the PC prior to calling subroutine from stack
                    PC = stack[--stackPtr];
                    break;
                default:
                    unknownOpcode(opcode);
            }
            break;
        case 0x1000: // jump to address NNN
            PC = NNN(opcode);
            break;
        case 0x2000: // call subroutine at address NNN
            // push current PC to stack
            stack[stackPtr++] = PC;
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
        case 0x8000: // logical & arithmetic functions
            switch (opcode & 0x000F) {
                case 0x0000: // set VX to VY
                    V[X(opcode)] = V[Y(opcode)];
                    break;
                case 0x0001: // VX = VX OR VY
                    V[X(opcode)] |= V[Y(opcode)];
                    break;
                case 0x0002: // VX = VX AND VY
                    V[X(opcode)] &= V[Y(opcode)];
                    break;
                case 0x0003: // VX = VX XOR VY
                    V[X(opcode)] ^= V[Y(opcode)];
                    break;
                case 0x0004: // VX = VX + VY (set carry flag unlike 7XNN add)
                    V[0xF] = ((int) V[X(opcode)] + (int) V[Y(opcode)]) > 255 ? 1 : 0;
                    V[X(opcode)] += V[Y(opcode)];
                    break;
                case 0x0005: // VX = VX - VY
                    if (V[X(opcode)] > V[Y(opcode)]) {
                        V[0xF] = 1;
                    }
                    else if (V[X(opcode)] < V[Y(opcode)]) {
                        V[0xF] = 0;
                    }
                    V[X(opcode)] -= V[Y(opcode)];
                    break;
                case 0x0006: // shift right
                    if (ORIGINAL_FORMAT) {
                        V[X(opcode)] = V[Y(opcode)];
                    }
                    V[0xF] = V[X(opcode)] & 0x1;
                    V[X(opcode)] >>= 1;
                    break;
                case 0x0007: // VX = VY - VX
                    if (V[Y(opcode)] > V[X(opcode)]) {
                        V[0xF] = 1;
                    }
                    else if (V[Y(opcode)] < V[X(opcode)]) {
                        V[0xF] = 0;
                    }
                    V[X(opcode)] = V[Y(opcode)] - V[X(opcode)];
                    break;
                case 0x000E: // shift left
                    if (ORIGINAL_FORMAT) {
                        V[X(opcode)] = V[Y(opcode)];
                    }
                    V[0xF] = (V[X(opcode)] >> 7) & 0x1;
                    V[X(opcode)] <<= 1;
                    break;
                default:
                    unknownOpcode(opcode);
            }
            break;
        case 0x9000: // skip if VX != VY
            if (V[X(opcode)] != V[Y(opcode)]) {
                PC += 2;
            }
            break;
        case 0xA000: // set index register I to NNN
            I = NNN(opcode);
            break;
        case 0xB000: // jump with offset
            if (ORIGINAL_FORMAT) {
                // jump to NNN + V0
                PC = NNN(opcode) + V[0];
            }
            else {
                // jump to XNN + V0
                PC = (opcode & 0x0FFF) + V[X(opcode)];
            }
            break;
        case 0xC000: // generate random num to VX
            V[X(opcode)] = (rand() % 256) & NN(opcode);
            break;
        case 0xD000: // display / draw XYN
            draw(V[X(opcode)], V[Y(opcode)], N(opcode));
            break;
        case 0xE000: // skip if key
            switch (opcode & 0x00FF) {
                case 0x009E: // skip if VX key is pressed
                    if(key[V[(X(opcode))]]) {
                        PC += 2;
                    }
                    break;
                case 0x00A1: // skip if VX key not pressed
                    if(!key[V[(X(opcode))]]) {
                        PC += 2;
                    }
                    break;
                default:
                    unknownOpcode(opcode);
            }
            break;
        case 0xF000: // miscellaneous operations
            switch (opcode & 0x00FF) {
                case 0x0007: // set VX to delay timer
                    V[X(opcode)] = delayTimer;
                    break;
                case 0x000A: // get key
                    for(int i = 0; i < NUM_KEYS; i++) {
                        if (key[i]) {
                            V[X(opcode)] = i;
                            return;
                        }
                    }
                    // subtract 2 from PC so the command is repeated if no key press
                    PC -= 2;
                    break;
                case 0x0015: // set delay timer to VX
                    delayTimer = V[X(opcode)];
                    break;
                case 0x0018: // set sound timer to VX
                    soundTimer = V[X(opcode)];
                    break;
                case 0x001E: // add to index
                    I += V[X(opcode)];
                    break;
                case 0x0029: // font: set I to hexadecimal character in VX
                    I = 5 * V[X(opcode)]; // multiply by 5 since 5 bytes per character in fontset
                    break;
                case 0x0033: // binary coded decimal conversion
                    memory[I] = V[X(opcode)] / 100; // store first digit in memory[I]
                    memory[I + 1] = (V[X(opcode)]  % 100) / 10; // store second digit in next block
                    memory[I + 2] = V[X(opcode)] % 10; // store third digit in next block
                    break;
                case 0x0055: // store V0 to VX in memory, starting at I
                    for (uint16_t i = 0; i < X(opcode); i++) {
                        memory[I + i] = V[i];
                    }
                    if (ORIGINAL_FORMAT) {
                        I += X(opcode) + 1;
                    }
                    break;
                case 0x0065: // load V0 to VX from memory, starting at I
                    for (uint16_t i = 0; i < X(opcode); i++) {
                        V[i] = memory[I + i];
                    }
                    if (ORIGINAL_FORMAT) {
                        I += X(opcode) + 1;
                    }
                    break;
                default:
                    unknownOpcode(opcode);
            }
            break;
        default:
            unknownOpcode(opcode);
    }
}

void tick() {
    if (delayTimer > 0) {
        delayTimer--;
    }
    if (soundTimer > 0) {
        soundTimer--;
        if (soundTimer == 0) {
            // TODO: BEEP ME
        }
    }
}

void initC8() {
    // initialize global vars
    ORIGINAL_FORMAT = 0;
    PC = 0x200;
    I = 0;
    stackPtr = 0;
    delayTimer = 0;
    soundTimer = 0;
    memset(memory, 0, sizeof(uint8_t) * MEMORY_SIZE);
    memset(V, 0, sizeof(uint8_t) * NUM_GENERAL_REGISTERS);
    memset(stack, 0, sizeof(uint16_t) * STACK_SIZE);
    memset(display, 0, sizeof(bool) * (DISPLAY_X * DISPLAY_Y));
    memset(key, 0, sizeof(bool) * NUM_KEYS);

    // store font in memory
    uint8_t fontset[80] = { 
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
    for (int i = 0; i < 80; i++) {
        memory[i] = fontset[i];
    }
}

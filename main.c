// TODO:
// chip8.h formatting standard (includes)
// extern var proper usage
// key input

#include "chip8.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>
#include <ncursesw/ncurses.h>


// map inputs to keys
int keymap(unsigned char k) {
    switch (k) {
        case '1': return 0x1;
        case '2': return 0x2;
        case '3': return 0x3;
        case '4': return 0xc;

        case 'q': return 0x4;
        case 'w': return 0x5;
        case 'e': return 0x6;
        case 'r': return 0xd;

        case 'a': return 0x7;
        case 's': return 0x8;
        case 'd': return 0x9;
        case 'f': return 0xe;
                  
        case 'z': return 0xa;
        case 'x': return 0x0;
        case 'c': return 0xb;
        case 'v': return 0xf;

        default:  return -1;
    }
}


// print the display to the console
void consoleDisplay(WINDOW *window) {
    // establish display variable from chip8.c
    extern bool display[DISPLAY_Y][DISPLAY_X];

    for (int y = 0; y < DISPLAY_Y; y++) {
        for (int x = 0; x < DISPLAY_X; x++) {
            mvwaddstr(
                window,
                y + 1,
                (x * 2) + 1,
                display[y][x] == 1 ? "\u2588\u2588" : "  "
            );
        }
    }
    wrefresh(window);
}


int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "");

    // Handle input arguments
    if (argc != 2) {
        printf("Please specify an input ROM.\n");
        return 0;
    }

    // establish variables from chip8.c
    extern bool drawFlag;
    extern uint16_t PC;
    extern uint8_t memory[MEMORY_SIZE];
    extern bool key[NUM_KEYS];

    // initialize chip8 variables
    initC8();
    // load and init the ROM
    initRom(argv[1]);

    // init ncurses
    initscr();

    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);
    WINDOW *screenWin = newwin(
        DISPLAY_Y + 2,
        (DISPLAY_X * 2) + 2,
        (yMax - (DISPLAY_Y - 2)) / 2,
        (xMax - ((DISPLAY_X * 2) - 2)) / 2
    );
    cbreak();
    nodelay(screenWin, TRUE);
    keypad(screenWin, TRUE);
    noecho();
    curs_set(0); // hide cursor
    timeout(100);
 
    // Draw screen border
    box(screenWin, 0, 0);

    // main process loop
    uint16_t opcode = 0;
    while (1) {
        unsigned char pressedKey = wgetch(screenWin);

        // if escape key is pressed
        if (pressedKey == 27) {
            break;
        }

        // refresh keys to false
        memset(key, 0, sizeof(bool) * NUM_KEYS);

        int index = keymap(pressedKey);
        if (index > -1) {
            key[index] = 1;
        }

        // read the instruction that the PC is currently pointing to & increment PC
        opcode = memory[PC] << 8 | memory[PC + 1];
        PC += 2;

        // execute the operation
        execute(opcode);

        // update the display
        if (drawFlag) {
            drawFlag = FALSE;
            consoleDisplay(screenWin);
        }

        // update timers
        tick();

        // delay to slow down emulation speed
        usleep(1200);
    }

    endwin();
    return 0;
}

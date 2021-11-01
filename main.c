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

// print the display to the console
void consoleDisplay(WINDOW *window) {
    // establish display variable from chip8.c
    extern bool display[DISPLAY_Y][DISPLAY_X];

    for (int y = 0; y < DISPLAY_Y; y++) {
        for (int x = 0; x < DISPLAY_X; x++) {
            if (display[y][x]) {
                wmove(window, y + 1, (x * 2) + 1);
                waddwstr(window, L"\u2588\u2588");
            }
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

//   ██████╗██╗     ██╗     ██████╗██╗  ██╗██╗██████╗        █████╗
//  ██╔════╝██║     ██║    ██╔════╝██║  ██║██║██╔══██╗      ██╔══██╗
//  ██║     ██║     ██║    ██║     ███████║██║██████╔╝█████╗╚█████╔╝
//  ██║     ██║     ██║    ██║     ██╔══██║██║██╔═══╝ ╚════╝██╔══██╗
//  ╚██████╗███████╗██║    ╚██████╗██║  ██║██║██║           ╚█████╔╝
//   ╚═════╝╚══════╝╚═╝     ╚═════╝╚═╝  ╚═╝╚═╝╚═╝            ╚════╝

////////////////////////////////////////////////////////////////////
//  Imports
////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

////////////////////////////////////////////////////////////////////
//  Global Vars
////////////////////////////////////////////////////////////////////

// monochrome display (1 = white, 0 = black)
bool display[64][32];

////////////////////////////////////////////////////////////////////
//  Functionality
////////////////////////////////////////////////////////////////////

// open and initialize a ROM given its path
void initRom(char *path) {
    printf("Init Rom\n");

    FILE *rom = fopen(path, "rb");
    if (!rom) {
        printf("Error! Rom %s does not exist.\n", path);
        exit(0);
    }
}

// read the instruction that the PC is currently pointing to from memory
void fetch() {
    printf("fetch\n");
}

// decode and execute a given instruction
void execute() {
    printf("execute\n");
}

int main(int argc, char *argv[]) {
    // Handle input arguments
    if (argc == 2) {
        initRom(argv[1]);
    }
    else {
        printf("Please specify an input ROM.\n");
        return 0;
    }

    // main process loop
    while (1) {
        // fetch
        fetch();
        // execute
        execute();

        break;
    }
}
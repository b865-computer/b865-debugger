#include <iostream>
#include <cstdio>
#include "Emulator.h"

Emulator emulator;

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        fprintf(stderr, "Input file missing, Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    emulator.init();
    if(emulator.load(argv[1]))
    {
        return 1;
    }
    emulator.start();

    while(emulator.isRunning())
    {
        std::this_thread::sleep_for(std::chrono::nanoseconds(1000));
    }

    return emulator.exitCode();
}
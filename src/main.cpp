#include <iostream>
#include <cstdio>
#include "Emulator.h"

Emulator emulator;

int main(int argc, char *argv[])
{
    if(emulator.init())
    {
        return 1;
    }

    if(argc >= 2)
    {
        if(emulator.load(argv[1]))
        {
            return 1;
        }
    }

    return emulator.main();
}
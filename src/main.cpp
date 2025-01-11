#include <iostream>
#include <cstdio>
#include "Emulator.h"
#include "FilePath.h"

Emulator emulator;

int main(int argc, char *argv[])
{
    setExeBasePath(argv[0]);
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
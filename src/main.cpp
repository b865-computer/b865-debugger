#include <iostream>
#include <cstdio>
#include "Emulator.h"
#include "Utils.h"

Emulator emulator;

int main(int argc, char *argv[])
{
    initExeBasePath();
    setCwdArgv0(argv[0]);
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
#include "Pheriph.h"

Pheriph::Pheriph(uint8_t len)
{
    if (len)
    {
        regs = (uint8_t *)calloc(len, 1);
        if (!regs)
        {
            fprintf(stderr, "Failed to allocate: %i bytes", len);
            exit(1);
        }
    }
}

Pheriph noPheriph(0);
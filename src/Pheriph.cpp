#include "Pheriph.h"

Pheriph::Pheriph()
{
    initRegs(0);
}

Pheriph::Pheriph(uint16_t len)
{
    initRegs(len);
}

Pheriph::Pheriph(uint16_t len, std::string name)
    : m_name(name)
{
    initRegs(len);
}

Pheriph::Pheriph(uint16_t len, std::string name, std::vector<std::string> regNames)
    : m_regNames(regNames), m_name(name)
{
    initRegs(len);
}

Pheriph::~Pheriph()
{
    stop();
    if (regs)
    {
        delete[] regs;
        regs = nullptr;
    }
    return;
}

void Pheriph::stop()
{
    if (!running)
    {
        return;
    }
    running = false;
}

/// @brief allocate memory for the registers
/// @param len number of registers
void Pheriph::initRegs(int len)
{
    regs = nullptr;
    if (len)
    {
        regs = (uint8_t *)calloc(1, len);
        if (!regs)
        {
            fprintf(stderr, "Failed to allocate: %i uint8_ts", len);
            exit(1);
        }
    }
}

#pragma once
#ifndef _DEBUGGER_H_
#define _DEBUGGER_H_

#include "common.h"

struct debugSym
{
    uint16_t address = 0;
    unsigned long long line = 0;
    unsigned long long fileID = 0;
    std::string symbol; // can hold filename (only holds symbol name if line is equal to noLine)
    const static unsigned long long noLine = 0xFFFFFFFFFFFFFFFF;
};

typedef debugSym breakpoint;

class DebuggerData
{
public:
    int init(std::string configFileName);
    std::vector<std::string> getFileNames();
    int getBreakpointData(std::vector<breakpoint> &);
    breakpoint getBreakpoint(uint16_t address);
    
public:
    std::vector<debugSym> SymbolData;

private:
    std::vector<debugSym> AdrData;
    std::vector<std::string> filenames;
};

#endif
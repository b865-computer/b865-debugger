#pragma once
#ifndef _DEBUGGER_H_
#define _DEBUGGER_H_

#include "Common.h"

struct debugSym
{
    uint16_t address = 0;
    uint64_t line = 0;
    uint64_t fileID = 0;
    std::string symbol; // can hold filename (only holds symbol name if line is equal to noLine)
    const static uint64_t noLine = 0xFFFFFFFFFFFFFFFF;
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
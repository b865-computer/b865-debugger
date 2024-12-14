#pragma once
#ifndef _DEBUGGER_H_
#define _DEBUGGER_H_

#include "common.h"

struct debugSym
{
    uint16_t address;
    size_t line;
    size_t fileID;
    std::string file;
};

typedef debugSym breakpoint;

class DebuggerData
{
public:
    int init(std::string configFileName);
    std::vector<std::string> getFileNames();
    int getBreakpointData(std::vector<breakpoint> &);

private:
    std::vector<breakpoint> breakpoints;
    std::vector<debugSym> debuggerSymbols;
    std::vector<std::string> filenames;
};

#endif
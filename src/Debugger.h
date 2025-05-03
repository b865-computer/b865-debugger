#pragma once
#ifndef _DEBUGGER_H_
#define _DEBUGGER_H_

#include "Parser/Parser.h"
#include "Common.h"

typedef LinkerRecord CodePosition; // name = fileName
typedef CodePosition breakpoint;

class DebuggerDataHelper
{
public:
    int init(std::string configFileName);
    std::string getBinFile();

    CodePosition getPosition(uint64_t addr);
    
public:

private:
    std::string binfile;
    DebuggerData m_data;
};

#endif // _DEBUGGER_H_
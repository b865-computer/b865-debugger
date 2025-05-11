#pragma once
#ifndef _DEBUGGER_H_
#define _DEBUGGER_H_

#include "Parser/Parser.h"
#include <CdbgExpr.h>
#include "Common.h"
#include "Breakpoints.h"

typedef Breakpoint CodePosition;

class FuncScopeData
{
public:
    size_t level;
    size_t block;
};

typedef std::vector<CdbgExpr::SymbolDescriptor> SymbolVector;
typedef std::vector<std::pair<FuncScopeData, CdbgExpr::SymbolDescriptor>> FuncScopedSymbolVector;
typedef std::unordered_map<std::string, SymbolVector> FileScopedSymbols;
typedef std::unordered_map<std::string, FuncScopedSymbolVector> FuncScopedSymbols;

class DebuggerDataHelper
{
public:
    int init(std::string configFileName);

    std::string getBinFile();
    CodePosition getPosition(uint64_t addr);
    
private:
    void createSymbolDescriptors();
    LinkerRecord getLinkerRecordForSymbol(const SymbolRecord& symbolRec);
    std::vector<CdbgExpr::CType> getCTypeFromTypeChain(const TypeChainRecord& typeChain);


public:
    FileScopedSymbols fileScope;
    FuncScopedSymbols funcScope;
    SymbolVector globalScope;
    DebuggerData data;

private:
    std::string binfile;
};

#endif // _DEBUGGER_H_
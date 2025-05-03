#pragma once
#ifndef _DEBUGGERDATA_H_
#define _DEBUGGERDATA_H_

#include <string>
#include <vector>
#include <cstdint>
#include <map>

enum class REG
{
    A_IDX,
    X_IDX,
    Y_IDX,
    SP_IDX,
    R0_IDX,
    R1_IDX,
    R2_IDX,
    R3_IDX,
};

class TypeChainRecord
{
public:
    int size = 0;
    class Type
    {
    public:
        enum class DCLType
        {
            ARRAY,
            FUNCTION,
            GEN_POINTER,
            CODE_POINTER,
            EXT_RAM_POINTER,
            INT_RAM_POINTER,
            PAGED_POINTER,
            UPPER128_POINTER,
            LONG,
            INT,
            CHAR,
            SHORT,
            VOID,
            FLOAT,
            STRUCT,
            SBIT,
            BITFIELD,
            UNKNOWN,
        };
        DCLType DCLtype = DCLType::UNKNOWN;
        union
        {
            int n;
            struct
            {
                char offset;
                char size;
            } bitField;
        } num;
        std::string name;
    };
    std::vector<Type> types;
    bool sign = false;
};

class Scope
{
public:
    enum class Type
    {
        GLOBAL = 'G',
        FUNCTION = 'L',
        FILE = 'F',
        STRUCT = 'S',
        UNKNOWN,
    };
    Type type = Type::UNKNOWN;
    std::string name;
};

enum class AddressSpace
{
    EXT_STACK = 'A',
    INT_STACK = 'B',
    CODE = 'C',
    CODE_STATIC = 'D',
    INT_RAM_LOW = 'E',
    EXT_RAM = 'F',
    INT_RAM = 'G',
    BIT_ADR = 'H',
    SFR_SPACE = 'I',
    SBIT_SPACE = 'J',
    REGISTER_SPACE = 'R',
    FUNCTION_UNDEF = 'Z',
    UNKNOWN,
};

class ScopeNameLevelBlock
{
public:
    Scope scope;
    std::string name;
    uint64_t level = 0;
    uint64_t block = 0;
};

class SymbolRecord : public ScopeNameLevelBlock
{
public:
    TypeChainRecord typeChain;
    AddressSpace addressSpace = AddressSpace::UNKNOWN;
    bool onStack = false;
    int stack_offs = 0;
    std::vector<REG> registers;
};

class FunctionRecord : public SymbolRecord
{
public:
    bool interrupt = false;
    int interruptNum = 0;
    int regBankNum = 0;
};

class TypeMember
{
public:
    int offset = 0;
    SymbolRecord member;
};

class TypeRecord
{
public:
    Scope scope;
    std::string name;
    std::vector<TypeMember> members;
};

class LinkerRecord : public ScopeNameLevelBlock
// name contains filename when type is *_LINE
{
public:
    enum class Type
    {
        SYMBOL_ADDR,
        SYMBOL_END_ADDR,
        ASM_LINE,
        C_LINE,
    };
    Type type;
    uint64_t line = 0;
    uint64_t addr = 0;
};

class LineData
{
public:
    std::string filename;
    uint64_t line = 0;
};

class ScopeData
{
public:
    std::vector<SymbolRecord> symbols;
    std::vector<FunctionRecord> functions;
    std::vector<TypeRecord> types;
    std::vector<LinkerRecord> linkerRecords;
};

class DebuggerData
{
public:
    std::vector<std::string> modules;
    ScopeData globalScope;
    std::map<std::string, ScopeData> fileScope;
    std::map<std::string, ScopeData> funcScope;

    bool fail = false; // set by the parser indicates if parsing failed (true = fail)

public:
    void clear();
    void addModule(const std::string& module);
    void addSymbol(const SymbolRecord& symbol);
    void addFunc(const FunctionRecord& func);
    void addType(const TypeRecord& type);
    void addLinkerRecord(const LinkerRecord& record);

    LineData getLine(uint64_t addr);

    const std::vector<LinkerRecord>& getSymbolAddrs();

private: 
    void checkScopeExists(Scope scope);

    std::vector<LinkerRecord> symbolAddrs;
};

#endif
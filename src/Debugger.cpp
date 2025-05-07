#include "Debugger.h"
#include <fstream>
#include <json/json.h>
#include "Utils.h"
#include <algorithm>

/// @brief initializes the DebuggerDataHelper class based on the given json file
/// @param configFileName the configuration .json file
/// @return 0 if the initialization was successful, 1 otherwise
int DebuggerDataHelper::init(std::string configFileName)
{
    std::string path = getPath(configFileName);
    std::ifstream configFile(configFileName, std::ios::in);
    if (!configFile.is_open())
    {
        fprintf(stderr, "Error: Unable to open configuration file %s\n", configFileName.c_str());
        return 1;
    }
    Json::Value config;
    try {
        configFile >> config;
        configFile.close();
    }
    catch (Json::RuntimeError &e)
    {
        printf("Json runtimeError: %s", e.what());
        return 1;
    }
    std::string debugSymbolFileName(config["dbg"].asString());

    debugSymbolFileName = path + "/" + debugSymbolFileName;

    std::string mapFileName(config["map"].asString());

    mapFileName = path + "/" + mapFileName;

    binfile = config["bin"].asString();

    DbgDataParser parser;

    data = parser.parse(debugSymbolFileName, mapFileName);

    if (data.fail)
    {
        return 1;
    }
    
    createSymbolDescriptors();

    return 0;
}

/// @brief returns the binary file name
std::string DebuggerDataHelper::getBinFile()
{
    return binfile;
}

CodePosition DebuggerDataHelper::getPosition(uint64_t addr)
{
    CodePosition pos;
    auto line = data.getLine(addr);
    pos.name = line.filename;
    pos.addr = addr;
    pos.line = line.line;
    return pos;
}

void DebuggerDataHelper::createSymbolDescriptors()
{
    for (auto& symbolRec : data.globalScope.symbols)
    {
        CdbgExpr::SymbolDescriptor symbol;
        auto linkerRec = getLinkerRecordForSymbol(symbolRec);
        uint64_t addr = linkerRec.addr;
        symbol.hasAddress = false;
        symbol.setValue(addr);
        symbol.name = symbolRec.name;
        symbol.size = symbolRec.typeChain.size;
        symbol.cType = getCTypeFromTypeChain(symbolRec.typeChain);
        symbol.hasAddress = true;
        for (auto& type : symbolRec.typeChain.types)
        {
            if (type.DCLtype == TypeChainRecord::Type::DCLType::FUNCTION)
            {
                symbol.hasAddress = false;
            }
        }
        symbol.isSigned = symbolRec.typeChain.sign;
        printf("Symbol: %s = 0x%04lX\n", symbol.name.c_str(), symbol.toUnsigned());
        globalScope.push_back(symbol);
    }
}

LinkerRecord DebuggerDataHelper::getLinkerRecordForSymbol(const SymbolRecord &symbolRec)
{
    // TODO: implement file and function scopes.
    auto it = std::find_if(data.globalScope.linkerRecords.begin(), 
        data.globalScope.linkerRecords.end(), 
        [symbolRec](LinkerRecord& linkerRec){
            if (linkerRec.type != LinkerRecord::Type::SYMBOL_ADDR)
            {
                return false;
            }
            return symbolRec.name == linkerRec.name;
        });
    if (it != data.globalScope.linkerRecords.end())
    {
        return *it;
    }
    return LinkerRecord();
}

std::vector<CdbgExpr::CType> DebuggerDataHelper::getCTypeFromTypeChain(const TypeChainRecord &typeChain)
{
    std::vector<CdbgExpr::CType> cTypes;
    for (size_t i = 0; i < typeChain.types.size(); i++)
    {
        CdbgExpr::CType cType;
        auto& type = typeChain.types[i];
        switch (type.DCLtype)
        {
        case TypeChainRecord::Type::DCLType::ARRAY:
            cType = CdbgExpr::CType::Type::ARRAY;
            break;
        case TypeChainRecord::Type::DCLType::BITFIELD:
            cType = CdbgExpr::CType::Type::BITFIELD;
            cType.offset = type.num.bitField.offset;
            cType.size = type.num.bitField.size;
            break;
        case TypeChainRecord::Type::DCLType::CHAR:
            cType = CdbgExpr::CType::Type::CHAR;
            break;
        case TypeChainRecord::Type::DCLType::CODE_POINTER:
            cType = CdbgExpr::CType::Type::POINTER;
            break;
        case TypeChainRecord::Type::DCLType::EXT_RAM_POINTER:
            cType = CdbgExpr::CType::Type::POINTER;
            break;
        case TypeChainRecord::Type::DCLType::FLOAT:
            cType = CdbgExpr::CType::Type::FLOAT;
            break;
        case TypeChainRecord::Type::DCLType::FUNCTION:
            cType = CdbgExpr::CType::Type::POINTER;
            break;
        case TypeChainRecord::Type::DCLType::GEN_POINTER:
            cType = CdbgExpr::CType::Type::POINTER;
            break;
        case TypeChainRecord::Type::DCLType::INT:
            cType = CdbgExpr::CType::Type::INT;
            break;
        case TypeChainRecord::Type::DCLType::INT_RAM_POINTER:
            cType = CdbgExpr::CType::Type::POINTER;
            break;
        case TypeChainRecord::Type::DCLType::LONG:
            cType = CdbgExpr::CType::Type::LONG;
            if (typeChain.types.size() > i + 1 && typeChain.types[i + 1].DCLtype == TypeChainRecord::Type::DCLType::LONG)
            {
                i++;
                cType = CdbgExpr::CType::Type::LONGLONG;
            }
            break;
        case TypeChainRecord::Type::DCLType::PAGED_POINTER:
            cType = CdbgExpr::CType::Type::POINTER;
            break;
        case TypeChainRecord::Type::DCLType::SBIT:
            cType = CdbgExpr::CType::Type::VOID;
            break;
        case TypeChainRecord::Type::DCLType::SHORT:
            cType = CdbgExpr::CType::Type::SHORT;
            break;
        case TypeChainRecord::Type::DCLType::STRUCT:
            cType = CdbgExpr::CType::Type::STRUCT;
            break;
        case TypeChainRecord::Type::DCLType::UPPER128_POINTER:
            cType = CdbgExpr::CType::Type::POINTER;
            break;
        case TypeChainRecord::Type::DCLType::VOID:
            cType = CdbgExpr::CType::Type::VOID;
            break;
        
        default:
            cType = CdbgExpr::CType::Type::UNKNOWN;
            break;
        }
        cTypes.push_back(cType);
    }
    return cTypes;
}

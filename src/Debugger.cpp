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
        symbol.hasAddress = true;
        symbol.name = symbolRec.name;
        symbol.size = symbolRec.typeChain.size;
        symbol.cType = getCTypeFromTypeChain(symbolRec.typeChain);
        symbol.isSigned = symbolRec.typeChain.sign;
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
    for (const auto& type : typeChain.types)
    {
        CdbgExpr::CType cType;
        switch (type.DCLtype)
        {
        case TypeChainRecord::Type::DCLType::VOID:
            cType = CdbgExpr::CType::VOID;
            break;
        
        default:
            break;
        }
        cTypes.push_back(cType);
    }
    return cTypes;
}

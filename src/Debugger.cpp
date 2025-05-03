#include "Debugger.h"
#include <fstream>
#include <json/json.h>
#include "Utils.h"

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

    binfile = config["bin"].asString();

    DbgDataParser parser;

    m_data = parser.parse(debugSymbolFileName);

    if (m_data.fail)
    {
        return 1;
    }
    
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
    auto line = m_data.getLine(addr);
    pos.name = line.filename;
    pos.addr = addr;
    pos.line = line.line;
    return pos;
}

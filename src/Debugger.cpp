#include "Debugger.h"
#include <fstream>
#include <json/json.h>
#include "Utils.h"

/// @brief initializes the DebuggerData class based on the given json file
/// @param configFileName the configuration .json file
/// @return 0 if the initialization was successful, 1 otherwise
int DebuggerData::init(std::string configFileName)
{
    std::string path = getPath(configFileName);
    std::ifstream configFile(configFileName, std::ios::in);
    if (!configFile.is_open())
    {
        fprintf(stderr, "Error: Unable to open configuration file %s\n", configFileName.c_str());
        return 1;
    }
    Json::Value config;
    configFile >> config;
    configFile.close();
    std::string debugSymbolFileName(config["dbg"].asString());

    debugSymbolFileName = path + "/" + debugSymbolFileName;

    std::ifstream debugSymbolFile(debugSymbolFileName, std::ios::in);
    if (!debugSymbolFile.is_open())
    {
        fprintf(stderr, "Error: unable to open debug symbol file %s\n", debugSymbolFileName.c_str());
        return 1;
    }

    filenames.clear();

    std::string line;
    std::string type;
    uint64_t begin;
    AdrData.clear();
    while (std::getline(debugSymbolFile, line))
    {
        begin = line.find(' ');
        if (begin == std::string::npos)
        {
            fprintf(stderr, "Error: invalid .noi file: %s\n", debugSymbolFileName.c_str());
            return 1;
        }
        type = line.substr(0, begin);
        if (type == "DEF")
        {
            debugSym symbol;
            std::string file = line.substr(begin + 1);
            auto separatorPos = file.find_first_of('.');
            // if the file name contains a dot that means its NOT a symbol definition (eg. like _main, or loop)
            if (separatorPos != std::string::npos)
            {
                symbol.line = strtoull(file.substr(separatorPos + 1).c_str(), nullptr, 0);
                // we know that line contains at least one space
                symbol.address = (uint16_t)strtoull(line.substr(line.find_last_of(' ')).c_str(), nullptr, 0);
                file.erase(separatorPos);
                symbol.symbol = file;
            }
            else
            {
                symbol.address = (uint16_t)strtoull(line.substr(line.find_last_of(' ')).c_str(), nullptr, 0);
                if((separatorPos = file.find_last_of(' ')) != std::string::npos)
                {
                    file.erase(separatorPos);
                }
                symbol.symbol = file; // file is equal to the symbol name in this case
                symbol.line = debugSym::noLine;
            }
            AdrData.push_back(symbol);
        }
        else if (type == "FILE")
        {
            if(line.find_last_of("\n\r") != std::string::npos)
            {
                // erase the newline charachter
                line.erase(line.find_last_of("\n\r"));
            }
            filenames.push_back(path + "/" + line.substr(begin + 1));
        }
        else if (type == "LOAD")
        {
            if(line.find_last_of("\n\r") != std::string::npos)
            {
                // erase the newline charachter
                line.erase(line.find_last_of("\n\r"));
            }
            filenames.insert(filenames.begin(), path + "/" + line.substr(begin + 1));
        }
    }
    debugSymbolFile.close();
    SymbolData.clear();
    for (int i = 0; i < AdrData.size(); i++)
    {
        if(AdrData[i].line == debugSym::noLine)
        {
            SymbolData.push_back(AdrData[i]);
            AdrData.erase(AdrData.cbegin() + i);
            i--; // We don't want to increment i, becaule we just removed an element
            continue;
        }
        for (int j = 1; j < filenames.size(); j++)
        {
            if (AdrData[i].symbol == filenames[j].substr(4).substr(0, filenames[j].length() - 8)) // remove src/ from the filename
            {
                AdrData[i].fileID = j;
                AdrData[i].symbol.clear(); // erase filename
                break;
            }
        }
    }
    return 0;
}

/// @brief returns the file names of the source files and the hex file name (index 0 is the hex file)
std::vector<std::string> DebuggerData::getFileNames()
{
    return filenames;
}

/// @brief Updates the addresses of breakpoints based on the debugger symbols.
/// @param breakpoints the breakpoints to update
/// @return 0 if successful, 1 otherwise (always 0)
int DebuggerData::getBreakpointData(std::vector<breakpoint> &breakpoints)
{
    for (int i = 0; i < breakpoints.size(); i++)
    {
        for (int j = 0; j < AdrData.size(); j++)
        {
            if (filenames[AdrData[j].fileID] == breakpoints[i].symbol && AdrData[j].line == breakpoints[i].line)
            {
                breakpoints[i].address = AdrData[j].address;
                break;
            }
        }
    }
    return 0;
}


breakpoint DebuggerData::getBreakpoint(uint16_t address)
{
    for (int i = 0; i < AdrData.size(); i++)
    {
        if(AdrData.size() > (i + 1))
        {
            if (AdrData[i].address <= address && AdrData[i + 1].address > address)
            {
                return AdrData[i];
            }
        }
        else
        {
            if (AdrData[i].address == address)
            {
                return AdrData[i];
            }
        }
    }
    return breakpoint();
}

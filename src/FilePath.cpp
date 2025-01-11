#include "FilePath.h"
#include <stdio.h>

std::string exeBasePath;

void setExeBasePath(char* argv0)
{
    exeBasePath = argv0;
    if(exeBasePath.find_last_of('/') != std::string::npos)
    {
        exeBasePath.erase(exeBasePath.find_last_of('/'), std::string::npos);
    }
    if(exeBasePath.find_last_of('\\') != std::string::npos)
    {
        if(exeBasePath[exeBasePath.find_last_of('\\') - 1] == '\\')
        {
            exeBasePath.erase(exeBasePath.find_last_of('\\') - 1, std::string::npos);
        }
        else
        {
            exeBasePath.erase(exeBasePath.find_last_of('\\'), std::string::npos);
        }
    }
}

std::string getFilePathFromExeRelative(std::string relPath)
{
    if(!relPath.size())
    {
        goto END;
    }
    if(relPath[0] != '.')
    {
        goto END;
    }
    relPath = exeBasePath + "/" + relPath;
END:
    return relPath;
}

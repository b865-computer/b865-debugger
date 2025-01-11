#include "FilePath.h"
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#elif __APPLE__

#include <mach-o/dyld.h>
#include <climits>
#include <filesystem>

#elif
#include <unistd.h>
#endif

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

std::string exeBasePath;
std::string cwd;

void initExeBasePath()
{
#ifdef _WIN32
    // Windows specific
    wchar_t szPath[MAX_PATH];
    GetModuleFileNameW( NULL, szPath, MAX_PATH );
    std::wstring ws(szPath);
    std::string str(ws.begin(), ws.end());
#elif __APPLE__
    char szPath[PATH_MAX];
    uint32_t bufsize = PATH_MAX;
    if (!_NSGetExecutablePath(szPath, &bufsize))
        return std::filesystem::path{szPath}.parent_path() / ""; // to finish the folder path with (back)slash
    return;  // error
    std::string str(szPath);
#else
    // Linux specific
    char szPath[PATH_MAX];
    ssize_t count = readlink( "/proc/self/exe", szPath, PATH_MAX );
    if( count < 0 || count >= PATH_MAX )
        return; // error
    szPath[count] = '\0';
    std::string str(szPath);
#endif
    exeBasePath = getPath(str);
}

std::string getExeBasePath()
{
    return exeBasePath;
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

std::string getPath(std::string filename)
{
    if(!filename.size())
    {
        goto END;
    }
    if(filename.find_last_of("\\/") != std::string::npos)
    {
        if(filename.find_last_of("\\/") == filename.find_last_of('\\'))
        {
            if(filename[filename.find_last_of('\\') - 1] == '\\')
            {
                filename.erase(filename.find_last_of('\\') - 1, std::string::npos);
            }
            else
            {
                filename.erase(filename.find_last_of('\\'), std::string::npos);
            }
        }
        else
        {
            filename.erase(filename.find_last_of("\\/"), std::string::npos);
        }
    }
    else
    {
        return "";
    }
END:
    return filename;
}

/// @brief checks if the extension of the file equals to the given extension
/// @param filename the filename
/// @param ext the extension to check, '.' must not be included in the extension
/// @return true if the extension of the file equals to the given extension, false otherwise
bool isExtEqual(std::string filename, std::string ext)
{
    return getExt(filename) == ext;
}

/// @brief gets the extension of the file
/// @param filename the filename
/// @return the extension of the file, if the file has no extension, an empty string is returned
std::string getExt(std::string filename)
{
    // check if the file has an extension, if not return an empty string
    if(filename.find_last_of('.') != std::string::npos &&
        // if a dot is found it must be after the last slash
        (filename.find_last_of('/') == std::string::npos || (filename.find_last_of('.') > filename.find_last_of('/'))) &&
        (filename.find_last_of('\\') == std::string::npos || (filename.find_last_of('.') > filename.find_last_of('\\'))))
    {
        return filename.substr(filename.find_last_of('.') + 1);
    }
    return "";
}

void setCwdArgv0(char *argv0)
{
    cwd = getPath(argv0);
}

std::string getCwd()
{
    return cwd;
}

#include "Utils.h"

std::string exeBasePath;
std::string g_cwd;

void initExeBasePath()
{
#ifdef _WIN32
    // Windows specific
    wchar_t szPath[MAX_PATH];
    GetModuleFileNameW( NULL, szPath, MAX_PATH );
    std::wstring ws(szPath);
    std::string str(ws.begin(), ws.end());
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
    g_cwd = getPath(argv0);
}

std::string getCwd()
{
    return g_cwd;
}

M_PROCESS_OUT startProgram(const std::string& cwd, const std::string& cmd, M_PROCESS& process)
{
#ifdef _WIN32
    std::string command;
    if(cwd.size() > 1)
    {
        command = "powershell /c \"cd " + cwd + " | " + cmd + "\"";
    }
    else
    {
        command = "powershell /c \"cd . | " + cmd + "\"";
    }
    HANDLE outWrite;
    HANDLE outRead;
    SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
    if(!CreatePipe(&outRead, &outWrite, &sa, 0))
    {
        fprintf(stderr, "Error creating pipe: %d\n", GetLastError());
        return INVALID_HANDLE_VALUE;
    }
    if(!SetHandleInformation(outRead, HANDLE_FLAG_INHERIT, 0))
    {
        fprintf(stderr, "Error setting handle information: %d\n", GetLastError());
        CloseHandle(outRead);
        CloseHandle(outWrite);
        return INVALID_HANDLE_VALUE;
    }
    STARTUPINFOA si = {sizeof(STARTUPINFOA)};
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = outWrite;
    si.hStdError = outWrite;

    PROCESS_INFORMATION pi = {0};

    if(!CreateProcessA(NULL,
                        const_cast<char*>(command.c_str()),
                        NULL,
                        NULL,
                        TRUE,
                        CREATE_NO_WINDOW,
                        NULL,
                        NULL,
                        &si,
                        &pi))
    {
        fprintf(stderr, "Error creating process: %d\n", GetLastError());
        CloseHandle(outRead);
        CloseHandle(outWrite);
        return INVALID_HANDLE_VALUE;
    }

    CloseHandle(outWrite);
    CloseHandle(pi.hThread);
    process = pi.hProcess;
    return outRead;
#else
    process = (bool)0;
    std::string command = cmd;
    if(cwd.size())
    {
        command = "bin/bash -r cd" + cwd + " && " + cmd;
    }
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe)
    {
        fprintf(stderr, "Error creating process.\n");
        return nullptr;
    }

    int fd = fileno(pipe);
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    process = (bool)1;
    return pipe;
#endif
    return 0; 
}

bool pollProgramOutput(M_PROCESS_OUT process, std::string& outPutLines)
{
#ifdef _WIN32
    DWORD bytesRead;
    char buffer[128];
    bool dataAvailable = false;

    while(true)
    {
        if(!PeekNamedPipe(process, NULL, 0, NULL, &bytesRead, NULL))
        {
            return false;
        }

        if(bytesRead == 0)
        {
             break;
        }

        if(ReadFile(process, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            outPutLines += buffer;
            dataAvailable = true;
        }
        else
        {
            break;
        }
    }
    return dataAvailable;
#else
    char buffer[128];
    while(fgets(buffer, sizeof(buffer), process) != nullptr)
    {
        outPutLines += buffer;
    }
    int status = pclose(process);
    return (status == -1);
#endif
    return false;
}

unsigned long programExitCode(M_PROCESS process, M_PROCESS_OUT out, bool* running)
{
#ifdef _WIN32
    DWORD exitCode;
    *running = true;
    if(GetExitCodeProcess(process, &exitCode) && exitCode != STILL_ACTIVE)
    {
        *running = false;
        CloseHandle(out);
        CloseHandle(process);
    }
    else
    {
        return 0;
    }
    return exitCode;
#else
    (void)process;
    (void)out;
    (void)running;
    return 0;
#endif
}
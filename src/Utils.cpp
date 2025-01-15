#include "Utils.h"

template<typename tVal>
tVal map_value(std::pair<tVal,tVal> a, std::pair<tVal, tVal> b, tVal inVal)
{
  tVal inValNorm = inVal - a.first;
  tVal aUpperNorm = a.second - a.first;
  tVal normPosition = inValNorm / aUpperNorm;

  tVal bUpperNorm = b.second - b.first;
  tVal bValNorm = normPosition * bUpperNorm;
  tVal outVal = b.first + bValNorm;

  return outVal;
}

template double map_value<double>(std::pair<double, double>, std::pair<double, double>, double);

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

std::string getFnWithoutPath(std::string filename)
{
    if(filename.find_last_of("\\/") != std::string::npos)
    {
        filename.erase(0, filename.find_last_of("\\/") + 1);
    }
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
    process = false;
    std::string command;
    if (cwd.size()) {
        command = "cd " + cwd + " && " + cmd + " 2>&1";
    } else {
        command = cmd + " 2>&1";
    }

    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        fprintf(stderr, "Error creating process.\n");
        process = false;
        return nullptr;
    }

    int fd = fileno(pipe);
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    process = true;
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
    if (!process) {
        return false;
    }

    char buffer[128];

    while (fgets(buffer, sizeof(buffer), process) != nullptr) {
        outPutLines += buffer;
    }
    // Check if the process has terminated by testing EOF
    if (feof(process)) {
        return false; // Process is finished
    }

    return true;
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
    if (!process || !out) {
        *running = false;
        return 0;
    }

    int status = pclose(out);
    if(status != -1)
    {
        *running = false;
        return WEXITSTATUS(status);
    }
    return 0;
#endif
}

void pinThreadToCore(std::thread::native_handle_type handle, int core_id)
{
#ifdef _WIN32

    DWORD_PTR mask = (1ULL << core_id);
    if (!SetThreadAffinityMask((HANDLE)handle, mask))
    {
        fprintf(stderr, "Failed to set thread affinity. Error: %lu\n", GetLastError());
    }
#else
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    if (pthread_setaffinity_np(handle, sizeof(cpu_set_t), &cpuset) != 0)
    {
        fprintf(stderr, "Failed to set thread affinity.\n");
    }
#endif
}

void setThreadPriority(std::thread::native_handle_type handle, bool high_priority)
{
#ifdef _WIN32
    int priority = high_priority ? THREAD_PRIORITY_HIGHEST : THREAD_PRIORITY_NORMAL;
    if (!SetThreadPriority((HANDLE)handle, priority))
    {
        fprintf(stderr, "Failed to set thread priority. Error: %lu\n", GetLastError());
    }
#else
    sched_param sch_params;
    sch_params.sched_priority = high_priority ? sched_get_priority_max(SCHED_FIFO) : 0;

    int result = pthread_setschedparam(handle, high_priority ? SCHED_FIFO : SCHED_OTHER, &sch_params) != 0;
    if (result != 0)
    {
        if (errno == EPERM) {
            fprintf(stderr, "Failed to set thread priority: Insufficient permissions.\n");
        } else if (errno == EINVAL) {
            fprintf(stderr, "Failed to set thread priority: Invalid policy or priority.\n");
        } else {
            fprintf(stderr, "Failed to set thread priority. Error code: %d\n", result);
        }
    }
#endif
}
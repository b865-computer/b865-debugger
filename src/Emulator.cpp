#include "Emulator.h"
#include "FilePath.h"

#ifdef _WIN32
#include <Windows.h>
typedef HANDLE M_PROCESS_OUT;
typedef HANDLE M_PROCESS;
#define M_PROCESS_INVALID INVALID_HANDLE_VALUE
#else
#include <fcntl.h>
#include <unistd.h>
typedef FILE* M_PROCESS_OUT;
typedef bool M_PROCESS;
#define M_PROCESS_INVALID 0
#endif

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

    // int len = MultiByteToWideChar(CP_UTF8, 0, cmd.c_str(), -1, NULL, 0);
    // std::wstring wcmd(len, L'\0');
    // MultiByteToWideChar(CP_UTF8, 0, cmd.c_str(), -1, &wcmd[0], len);

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

bool pollProgramOutput(M_PROCESS_OUT process, std::vector<std::string>& outPutLines)
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
            outPutLines.push_back(buffer);
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
        outPutLines.push_back(buffer);
    }
    int status = pclose(pipe)
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


CPU cpu;

void cycle(void)
{
    cpu.cycle();
}

Emulator::Emulator()
    : m_fq(1000000), m_clock(cycle), m_cpu(cpu), m_debuggerData(), m_gui(cpu.getStatus(), m_clock, cpu, m_debuggerData.SymbolData)
{
    m_clock.setHZ(m_fq.HZ);
}

int Emulator::init()
{
    m_cpu.init();
    m_clock.init();
    return m_gui.init();
}

int Emulator::load(std::string filename, std::string path)
{
    if(path.size() != 0)
    {
        filename = path + "/" + filename;
        m_gui.projectPath = path;
    }
    else
    {
        path = m_gui.projectPath = getPath(filename);
    }
    if(isExtEqual(filename, "b865"))
    {
        m_gui.projectFileName = filename;
        m_gui.sourceFileNames.clear();
        m_clock.setStatus(false);
        m_gui.NewProjectOpened = false;
        if(m_debuggerData.init(filename))
        {
            m_gui.displayError("Failed to Open project:\n%s", m_gui.projectFileName.c_str());
            return 1;
        }
        m_gui.sourceFileNames = m_debuggerData.getFileNames();
        filename = m_gui.projectPath + '/' + m_gui.sourceFileNames[0];
    }
    if(m_cpu.loadProgramFromFile(filename))
    {
        m_gui.displayError("Failed to load program from file:\n%s", filename.c_str());
        return 1;
    }
    return 0;
}

int Emulator::load(std::vector<uint8_t> &programData)
{
    return m_cpu.loadProgram(programData.data(), programData.size());
}

int Emulator::main()
{
    std::vector<std::string> outputLines;
    m_gui.ConsoleText = &outputLines;
    M_PROCESS_OUT buildProcessOut;
    M_PROCESS buildProcess;
    bool buildProcessRunning = false;
    std::string buildCmd;
    start();
    while (isRunning())
    {
        m_gui.mainLoop();
        if(!m_clock.getStatus())
        {
            m_gui.currentPosition = m_debuggerData.getBreakpoint(m_cpu.getStatus().PC.addr);
        }
        if (m_gui.NewProjectOpened)
        {
            if(load(m_gui.projectFileName))
            {
                continue;
            }
            m_cpu.startExec();
        }
        if(m_gui.buildRunning && !buildProcessRunning)
        {
#ifdef _WIN32
            buildCmd = "make";
#endif
            outputLines.clear();
            outputLines.push_back(buildCmd);
            buildProcessOut = startProgram(m_gui.projectPath, buildCmd, buildProcess);
            if(buildProcess == M_PROCESS_INVALID || buildProcessOut == M_PROCESS_INVALID)
            {
                m_gui.displayError("Failed to start build process");
                m_gui.buildRunning = false;
                continue;
            }
            buildProcessRunning = true;
        }
        if (buildProcessRunning)
        {
            buildProcessRunning = pollProgramOutput(buildProcessOut, outputLines);
            if(!buildProcessRunning)
            {
                unsigned long exitCode;
                if((exitCode = programExitCode(buildProcess, buildProcessOut, &buildProcessRunning)))
                {
                    m_gui.displayError("Build process exited with code: %d", exitCode);
                }
                if(!buildProcessRunning)
                {
                    outputLines.push_back("Process exited with code: " + std::to_string(exitCode));
                }
            }
            m_gui.buildRunning = buildProcessRunning;
        }
    }
    return 0;
}

void Emulator::start()
{
    m_cpu.startExec();
    m_clock.setStatus(false);
}

void Emulator::stop()
{
    m_clock.terminate();
    m_cpu.stopPheripherials();
    m_gui.terminate();
}

std::chrono::nanoseconds Emulator::getRunTime_ns()
{
    return m_clock.getRunTime_ns();
}

bool Emulator::isRunning()
{
    if (!m_gui.windowClosed())
    {
        return true;
    }
    else
    {
        stop();
    }
    return false;
}
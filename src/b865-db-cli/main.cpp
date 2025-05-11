#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <filesystem>
#include "Emulator.h"
#include "Parser/Parser.h"
#include "CLI.h"

CLI cli;
Emulator emulator([](const std::string& error)
{
    printf(error.c_str());
});
std::string projectFile;

void deallocExit(int code = 0)
{
    emulator.terminate();
    exit(code);
}

void handleArgs(const std::vector<std::string> &args)
{
    if (args.empty())
    {
        return;
    }
    projectFile = args[0];
}

int main(int argc, char *argv[])
{
    cli.usage_str = 
    "Usage: b865-db <options> <project file>\n"
    "options:\n";

    emulator.init();
    emulator.start();

    // Convert command-line arguments to a vector of strings
    std::vector<std::string> args(argv + 1, argv + argc);

    cli.addCommand("quit", "", true, [](const std::vector<std::string> &args)
                   { cli.quit(); (void)args;}, "Quit the program");
    cli.addCommand("print", "<string>", true, [](const std::vector<std::string> &args)
                   { if (args.size() > 1) { std::cout << args[1] << std::endl; } }, "Print the string");
    cli.addCommand("break", "<position>", true, [](const std::vector<std::string> &args)
                   { emulator.addBreakpoint(args); }, "Add a breakpoint at the specified location (file:line or line [in the current file])");
    cli.addCommand("delete", "<id>", true, [](const std::vector<std::string> &args)
                   { emulator.delBreakpoint(args); }, "Delete the breakpoint(s) with the specified id(s)");
    cli.addCommand("run", "", true, [](const std::vector<std::string> &args)
                    {
                        (void)args;
                        emulator.start();
                        emulator.continue_exec();
                    }, "Start the emulator/debugger");
    cli.addCommand("continue", "", true, [](const std::vector<std::string> &args)
                   { emulator.continue_exec(); emulator.start(); (void)args; }, "continue the execution of the program");
    handleArgs(args);

    BreakpointList::print = true;
    BreakpointList::execPath = std::filesystem::path(projectFile).parent_path().string();

    if (emulator.load(projectFile))
    {
        deallocExit(1);
        return 1;
    }

    std::cout << "b865-debugger (type 'quit' or 'q' to exit, 'help' for usage)\n";

    while (1)
    {
        if (!emulator.clockRunning() && cli.run())
        {
            break;
        }
        if (emulator.clockRunning())
        {
            if (emulator.pausedAtBreakpoint())
            {
                size_t id = emulator.getCurrentBreakPointId();
                printf("Program hit breakpoint %ld at address 0x%04x\n", id, emulator.m_cpu.getStatus().PC.addr);
                emulator.stop();
            }
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }

    emulator.terminate();

    deallocExit();

    return 0;
}

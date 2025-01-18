#pragma once
#ifndef _GUI_H_
#define _GUI_H_

#include "Common.h"

#include <GL/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "CPU.h"
#include "Clock.h"
#include "Debugger.h"
#include "Window.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void renderSideBar();
void renderSideTool();
void renderToolBar();
void renderEditor();
void renderConsole();
void renderFilesOpened();

enum FileInputType
{
    programFile = 0,
    projectFile,
};

class GUI
{
public:
    friend void framebuffer_size_callback(GLFWwindow *window, int width, int height);
    friend void renderSideBar();
    friend void renderSideTool();
    friend void renderToolBar();
    friend void renderEditor();
    friend void renderConsole();
    friend void renderFilesOpened();

    enum ToolType
    {
        TOOL_EXPLORER,
        TOOL_DEBUGGER,
    };

    GUI(const CPU_Status &status, Clock &clock, CPU &cpu, std::vector<debugSym> &symbolData);
    ~GUI();
    int init();
    void terminate();
    bool windowClosed();
    int main();
    void displayError(const char *fmt, ...);

private:
    void renderMenu();
    int render();
    bool LoadTextureFromMemory(const void *data, uint64_t data_size, GLuint *out_texture, int *out_width, int *out_height);
    bool LoadTextureFromFile(const char *file_name, GLuint *out_texture, int *out_width, int *out_height);

public:
    bool NewProjectOpened = false;
    std::string projectFileName;
    std::string projectPath;
    std::vector<std::string> sourceFileNames;
    std::vector<breakpoint> breakpoints;
    breakpoint currentPosition;
    uint16_t lastPosition;
    std::vector<debugSym> &m_symbolData;
    uint64_t m_frequencyHZ = 1000000;
    bool buildRunning = false;
    std::string *ConsoleText;
    bool ins_level;

private:
    ToolType sideBarToolType = TOOL_EXPLORER;
    GLFWwindow *window;
    const CPU_Status &m_CPUStatus;
    CPU &m_cpu;
    Clock &m_clock;
    Pheriph **m_pheripherials = nullptr;
    int m_pheriphCount = 0;
    bool end = false;
    std::string error_str;
    bool error_display;
    bool building = false;
    
    Window_Attrib* mainWindow;
};

#endif
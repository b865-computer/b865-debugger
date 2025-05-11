#pragma once
#ifndef _GUI_H_
#define _GUI_H_

#include "Common.h"

#include <GL/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "Emulator.h"
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
void renderExpressions();

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
    friend void renderExpressions();

    enum ToolType
    {
        TOOL_EXPLORER,
        TOOL_EMUALTOR,
        TOOL_DEBUGGER,
    };

    GUI();
    ~GUI();
    int init();
    void terminate();
    int main();
    void displayError(const char *fmt, ...);
    int load(std::string filename, std::string path = "");

private:
    void renderMenu();
    int render();
    bool LoadTextureFromMemory(const void *data, uint64_t data_size, GLuint *out_texture, int *out_width, int *out_height);
    bool LoadTextureFromFile(const char *file_name, GLuint *out_texture, int *out_width, int *out_height);

public:
    bool NewProjectOpened = false;
    std::string projectFileName;
    std::string projectPath;
    CodePosition currentPosition;
    uint16_t lastPosition;
    uint64_t m_frequencyHZ = 1000000;
    bool buildRunning = false;
    std::string *ConsoleText;
    bool ins_level;

private:
    Emulator m_emulator;
    ToolType sideBarToolType = TOOL_EXPLORER;
    GLFWwindow *window;
    const CPU_Status &m_CPUStatus;
    CPU &m_cpu;
    Clock &m_clock;
    std::string error_str;
    bool error_display;
    bool building = false;
    
    Window_Attrib* mainWindow;
};

#endif
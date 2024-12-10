#pragma once
#ifndef _GUI_H_
#define _GUI_H_

#include "Common.h"

#include <GL/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "CPU.h"
#include "Clock.h"

class GUI
{
public:
    GUI(const CPU_Status& status, Clock& clock, CPU& cpu);
    ~GUI();
    int init();
    void terminate();
    bool windowClosed();
    int mainLoop();
    // void AddPheriphList(Pheriph** pheripherials, int count);

private:
    void renderMenu();
    bool LoadTextureFromMemory(const void* data, size_t data_size, GLuint* out_texture, int* out_width, int* out_height);
    bool LoadTextureFromFile(const char* file_name, GLuint* out_texture, int* out_width, int* out_height);

private:
    GLFWwindow *window;
    const CPU_Status& m_CPUStatus;
    CPU& m_cpu;
    Clock& m_clock;
    uint64_t m_frequencyHZ = 1000000;
    Pheriph** m_pheripherials = nullptr;
    int m_pheriphCount = 0;
    bool end = false;
};

#endif
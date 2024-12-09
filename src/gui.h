#pragma once
#ifndef _GUI_H_
#define _GUI_H_

#include "Common.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "CPU.h"

class GUI
{
public:
    GUI(const CPU_Status& status);
    int init();
    void terminate();
    bool windowClosed();
    int mainLoop();

private:
    void drawText(int x, int y, std::string text);

private:
    const CPU_Status& m_CPUStatus;
    std::thread main_thread;
    bool end = false;
};

#endif
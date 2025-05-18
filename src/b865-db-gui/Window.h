#pragma once

#ifndef _WINDOW_H_
#define _WINDOW_H_
#include "Common.h"
#include "imgui.h"
#include <unordered_map>
#include <algorithm>

class Window_Attrib
{
public:
    bool visible = false;
    bool disableImGuiBegin = false;
    Window_Attrib *parent = nullptr;
    std::string name;
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar;
    ImGuiWindowClass window_class;

    ImGuiID windowViewportID;

    std::vector<Window_Attrib *> children;

    void (*render_func)() = nullptr;

    // Constructors
    Window_Attrib(std::string _name, bool _visible, bool disableImGuiBegin = false, ImGuiWindowFlags _windowFlags = (ImGuiWindowFlags)0);
    Window_Attrib();

    void setRenderFunc(void (*func)());
    void addChild(Window_Attrib *child);
    void removeChild(Window_Attrib *child);
    void render();
};

#endif
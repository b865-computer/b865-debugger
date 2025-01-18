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
    int x = 0, y = 0, width = 0, height = 0;
    int maxWidth = 0, maxHeight = 0; // Zero = no limit, Negative = offset from parent size
    bool visible = false;
    int resizePriority = 0;
    int offsetFromOriginX = 0, offsetFromOriginY = 0;
    bool disableImGuiBegin = false;
    Window_Attrib *parent;
    std::string name;
    enum class LayoutType
    {
        None,
        Horizontal,
        Vertical
    };
    LayoutType layout = LayoutType::None;

    std::vector<Window_Attrib *> children;

    void (*render_func)() = nullptr;

    // Constructors
    Window_Attrib(std::string _name, int _x, int _y, int _maxWidth, int _maxHeight, bool _visible, int _resizePriority, bool disableImGuiBegin = false);
    Window_Attrib();

    void setRenderFunc(void (*func)());
    void addChild(Window_Attrib *child);
    void removeChild(Window_Attrib *child);
    void adjustLayout(int displayWidth = 0, int displayHeight = 0);
    void setLayout(LayoutType _layout, int displayWidth = 0, int displayHeight = 0);
    void render();
};

#endif
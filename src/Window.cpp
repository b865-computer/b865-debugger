#include "Window.h"

Window_Attrib::Window_Attrib(std::string _name, int _x, int _y, int _maxWidth, int _maxHeight, bool _visible, int _resizePriority, bool _disableImGuiBegin)
    : name(_name), x(_x), y(_y), maxWidth(_maxWidth), maxHeight(_maxHeight), visible(_visible), resizePriority(_resizePriority), disableImGuiBegin(_disableImGuiBegin) {}

Window_Attrib::Window_Attrib() {}

void Window_Attrib::setRenderFunc(void (*func)())
{
    render_func = func;
}

// Add a child and maintain sorted order based on resizePriority
void Window_Attrib::addChild(Window_Attrib *child)
{
    child->parent = this;
    auto it = std::lower_bound(children.begin(), children.end(), child,
                               [](const Window_Attrib *a, const Window_Attrib *b)
                               {
                                   return a->resizePriority < b->resizePriority;
                               });
    children.insert(it, child);
}

// Remove a child
void Window_Attrib::removeChild(Window_Attrib *child)
{
    child->parent = nullptr;
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end())
    {
        children.erase(it);
    }
}

// Adjust the window's size and position (auto layout)
void Window_Attrib::adjustLayout(int displayWidth, int displayHeight)
{
    int parentWidth, parentHeight;
    if (parent == nullptr)
    {
        parentWidth = displayWidth;
        parentHeight = displayHeight;
        offsetFromOriginX = 0;
        offsetFromOriginY = 0;
    }
    else
    {
        parentWidth = parent->width;
        parentHeight = parent->height;
        offsetFromOriginX = parent->offsetFromOriginX + parent->x;
        offsetFromOriginY = parent->offsetFromOriginY + parent->y;
    }

    int consumedWidthByNegative = 0;
    int consumedHeightByNegative = 0;

    if (parent != nullptr)
    {
        for (auto sibling : parent->children)
        {
            if (sibling != this)
            {
                if (sibling->maxWidth < 0 && sibling->resizePriority > this->resizePriority && sibling->visible)
                    consumedWidthByNegative += -sibling->maxWidth;

                if (sibling->maxHeight < 0 && sibling->resizePriority > this->resizePriority && sibling->visible)
                    consumedHeightByNegative += -sibling->maxHeight;
            }
        }
    }

    if (maxWidth < 0)
    {
        width = std::min(-maxWidth, parentWidth - consumedWidthByNegative);
        x = parentWidth - width - consumedWidthByNegative;
    }
    else if (maxWidth > 0)
    {
        width = std::min(maxWidth, parentWidth - consumedWidthByNegative - x);
    }
    else
    {
        width = parentWidth - consumedWidthByNegative - x;
    }

    if (maxHeight < 0)
    {
        height = std::min(-maxHeight, parentHeight - consumedHeightByNegative);
        y = parentHeight - height - consumedHeightByNegative;
    }
    else if (maxHeight > 0)
    {
        height = std::min(maxHeight, parentHeight - consumedHeightByNegative - y);
    }
    else
    {
        height = parentHeight - consumedHeightByNegative - y;
    }

    for (auto child : children)
    {
        child->adjustLayout();
    }
}

void Window_Attrib::setLayout(LayoutType _layout, int displayWidth, int displayHeight)
{
    if (layout == LayoutType::Horizontal)
    {
        int offsetX = 0;
        for (auto &child : children)
        {
            if (!child->visible)
                continue;

            child->x = offsetX;      // Set position relative to parent
            offsetX += child->width; // Update the offset for the next child
        }
    }
    else if (layout == LayoutType::Vertical)
    {
        int offsetY = 0;
        for (auto &child : children)
        {
            if (!child->visible)
                continue;

            child->y = offsetY;       // Set position relative to parent
            offsetY += child->height; // Update the offset for the next child
        }
    }
    adjustLayout(displayWidth, displayHeight);
}

void Window_Attrib::render()
{
    if (render_func != nullptr)
    {
        std::string windowTitle = "window_" + name;
        int Xpos = x + offsetFromOriginX;
        int Ypos = y + offsetFromOriginY;
        ImGui::SetNextWindowPos(ImVec2(Xpos, Ypos), ImGuiCond_Always);

        ImGuiWindowFlags_ flags;

        bool render = true;

        ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
        if(!disableImGuiBegin)
        {
            render = ImGui::Begin(windowTitle.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
        }
        if (render)
        {
            render_func();
            // Render child windows
            for (auto &child : children)
            {
                if (child->visible)
                {
                    child->render();
                }
            }
        }
        if(!disableImGuiBegin)
        {
            ImGui::End();
        }
    }
    else
    {
        // Render child windows
        for (auto &child : children)
        {
            if (child->visible)
            {
                child->render();
            }
        }
    }
}
#include "Window.h"
#include <imgui_internal.h>

Window_Attrib::Window_Attrib(std::string _name, bool _visible, bool _disableImGuiBegin, ImGuiWindowFlags _windowFlags)
    : visible(_visible), disableImGuiBegin(_disableImGuiBegin), name(_name), windowFlags(_windowFlags | ImGuiWindowFlags_NoTitleBar)
{
}

Window_Attrib::Window_Attrib() {}

void Window_Attrib::setRenderFunc(void (*func)())
{
    render_func = func;
}

// Add a child and maintain sorted order based on resizePriority
void Window_Attrib::addChild(Window_Attrib *child)
{
    child->parent = this;
    child->window_class.ClassId = ImHashStr(name.c_str());
    children.push_back(child);
}

// Remove a childcd
void Window_Attrib::removeChild(Window_Attrib *child)
{
    child->parent = nullptr;
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end())
    {
        children.erase(it);
    }
}

void Window_Attrib::render()
{
    if (!visible)
    {
        return;
    }

    bool render = true;

    if (parent == nullptr)
    {
        window_class.ParentViewportId = ImGui::GetMainViewport()->ID;
    }
    else
    {
        window_class.ParentViewportId = parent->window_class.ClassId;
    }

    ImGui::SetNextWindowClass(&window_class);

    render = ImGui::Begin(name.c_str(), nullptr, windowFlags);
    windowViewportID = ImGui::GetWindowViewport()->ID;

    if (children.size())
    {
        ImGui::DockSpace(ImGui::GetID((name + "##dockspace").c_str()));
    }

    if (render && render_func != nullptr && !children.size())
    {
        render_func();
    }
        
    ImGui::End();

    if (render)
    {
        // Render child windows
        for (auto &child : children)
        {
            if (child->visible)
            {
                child->render();}
        }
    }
}
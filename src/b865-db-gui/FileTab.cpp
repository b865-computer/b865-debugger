#include "FileTab.h"
#include "imgui.h"
#include "Utils.h"
#include <algorithm>

FileTab::FileTab(const std::string filename, FileWarningCallback _callback)
    : m_filename(filename), m_callback(_callback)
{
    m_exists = false;
}

FileTab::~FileTab()
{
    m_callback = nullptr;
}

bool FileTab::init()
{
    bool ret;
    m_buffer = load(&ret);
    if (!ret)
    {
        m_init = true;
    }
    return ret;
}

int FileTab::refresh()
{
    if (!m_exists)
    {
        return 0;
    }
    bool ret;
    std::string buffer = load(&ret);
    if (ret)
    {
        return 1;
    }
    if (m_buffer != buffer)
    {
        if (m_modified)
        {
            if (m_callback("File modified by another program,\nsave this version, or discard changes",
                           FileWarningCallbackType::FileWarningCallbackType_SAVE_DISCARD) ==
                FileWarningCallbackReturnType::FileWarningCallbackReturnType_SAVE)
            {
                return save(m_buffer);
            }
            else
            {
                m_buffer = buffer;
                m_modified = false;
            }
        }
        else
        {
            if (m_callback("File modified by another program,\ndo you want to reload the file?",
                           FileWarningCallbackType::FileWarningCallbackType_YES_NO) ==
                FileWarningCallbackReturnType::FileWarningCallbackReturnType_YES)
            {
                m_buffer = buffer;
                m_modified = false;
            }
        }
    }
    return 0;
}

bool FileTab::ismodified()
{
    return m_modified;
}

int FileTab::save(std::string &buffer)
{
    m_file.open(m_filename, std::ios::out | std::ios::binary);
    if (!m_file.is_open())
    {
        m_exists = false;
        if (m_callback("Unable to open file (deleted),\nKeep file in editor?",
                       FileWarningCallbackType::FileWarningCallbackType_YES_NO) ==
            FileWarningCallbackReturnType::FileWarningCallbackReturnType_NO)
        {
            return 1;
        }
        return 0;
    }
    m_file.write(&buffer[0], buffer.size());
    if (m_file.bad())
    {
        m_callback("Failed to save file", FileWarningCallbackType::FileWarningCallbackType_OK);
    }
    m_file.close();
    m_modified = false;
    m_exists = true;
    return 0;
}

std::string FileTab::load(bool *ret)
{
    *ret = false;
    m_file.open(m_filename, std::ios::in | std::ios::binary);
    if (!m_file.is_open())
    {
        m_exists = false;
        if (m_callback("Unable to open file (deleted),\nKeep file in editor?",
                       FileWarningCallbackType::FileWarningCallbackType_YES_NO) ==
            FileWarningCallbackReturnType::FileWarningCallbackReturnType_NO)
        {
            *ret = true;
            return "";
        }
        return m_buffer;
    }
    std::string buffer;
    m_file.seekg(0, std::ios::end);
    size_t size = m_file.tellg();
    m_file.seekg(0, std::ios::beg);
    buffer.resize(size);
    m_file.read(&buffer[0], size);
    m_file.close();
    m_exists = true;
    m_modified = false;
    return buffer;
}

bool FileTab::exists()
{
    return m_exists;
}

std::string &FileTab::getContent()
{
    return m_buffer;
}

void FileTab::modify(const std::string &buffer)
{
    m_buffer = std::move(buffer);
    m_modified = true;
}

FileTabManager::FileTabManager()
{
}

FileTabManager::~FileTabManager()
{
}

void FileTabManager::renderFileTabs()
{
    bool pop = false;
    if (ImGui::BeginChild("TabScrollRegion", ImVec2(0, 30), ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_HorizontalScrollbar))
    {
        for (size_t i = 0; i < m_fileTabs.size(); i++)
        {
            auto fileTab = m_fileTabs[i];
            pop = false;
            if (fileTab == m_currentFileTab)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.8f, 1.0f));
                pop = true;
            }
            ImGui::PushID(i);

            ImGui::BeginChild(getFnWithoutPath(fileTab->m_filename).c_str(), ImVec2(0, 30), ImGuiChildFlags_AutoResizeX);

            ImGui::Text((getFnWithoutPath(fileTab->m_filename) + (fileTab->ismodified() ? "*" : "")).c_str());
            ImGui::SameLine();
            if ((ImGui::IsItemHovered(ImGuiMouseButton_Left) || (fileTab == m_currentFileTab)))
            {
                if (ImGui::SmallButton("x"))
                {
                    removeFileTab(fileTab);
                }
            }
            else {
                ImGui::SmallButton(" ");
            }
            if (i < m_fileTabs.size() - 1) {
                ImGui::SameLine();
                ImGui::Text("|");
            }
            ImGui::EndChild();

            if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
            {
                m_currentFileTab = fileTab;
                m_changed = true;
            }

            if (pop)
            {
                ImGui::PopStyleColor();
            }

            ImGui::SameLine();
            ImGui::PopID();
        }
    }
    ImGui::EndChild();
}

void FileTabManager::refreshFileTabs()
{
    for (auto fileTab : m_fileTabs)
    {
        if (fileTab->refresh())
        {
            removeFileTab(fileTab);
        }
    }
}

void FileTabManager::saveFileTab(std::shared_ptr<FileTab> fileTab)
{
    fileTab->save(fileTab->m_buffer);
}

bool FileTabManager::changedCurrentTab()
{
    if (m_changed)
    {
        m_changed = false;
        return true;
    }
    return false;
}

std::shared_ptr<FileTab> FileTabManager::getCurrentFileTab()
{
    return m_currentFileTab;
}

std::shared_ptr<FileTab> FileTabManager::getFileTab(std::string filename)
{
    for (auto fileTab : m_fileTabs)
    {
        if (fileTab->m_filename == filename)
        {
            return fileTab;
        }
    }
    return nullptr;
}

bool FileTabManager::closeFiles()
{
    for (auto fileTab : m_fileTabs)
    {
        if (fileTab->ismodified())
        {
            auto ret = fileTab->m_callback("File modified, save changes?", FileTab::FileWarningCallbackType::FileWarningCallbackType_SAVE_DISCARD_CANCEL);
            if (ret == FileTab::FileWarningCallbackReturnType::FileWarningCallbackReturnType_SAVE)
            {
                saveFileTab(fileTab);
            }
            else if (ret == FileTab::FileWarningCallbackReturnType::FileWarningCallbackReturnType_CANCEL)
            {
                return false;
            }
        }
        removeFileTab(fileTab);
    }
    m_fileTabs.clear();
    m_currentFileTab = nullptr;
    return true;
}

bool FileTabManager::removeFileTab(std::shared_ptr<FileTab> fileTab)
{
    if (fileTab == nullptr)
    {
        return true;
    }
    if (fileTab->ismodified())
    {
        auto ret = fileTab->m_callback("File modified, save changes?", FileTab::FileWarningCallbackType::FileWarningCallbackType_SAVE_DISCARD_CANCEL);
        if (ret == FileTab::FileWarningCallbackReturnType::FileWarningCallbackReturnType_SAVE)
        {
            saveFileTab(fileTab);
        }
        else if (ret == FileTab::FileWarningCallbackReturnType::FileWarningCallbackReturnType_CANCEL)
        {
            return false;
        }
    }
    auto it = std::find(m_fileTabs.begin(), m_fileTabs.end(), fileTab);
    if (it != m_fileTabs.end())
    {
        it = m_fileTabs.erase(it);
        if (fileTab == m_currentFileTab)
        {
            if (it != m_fileTabs.end())
            {
                m_currentFileTab = *it.base();
            }
            else
            {
                m_currentFileTab = nullptr;
            }
            m_changed = true;
        }
        fileTab = nullptr;
    }
    return true;
}

void FileTabManager::addFileTab(const std::string &filename, FileTab::FileWarningCallback callback)
{
    m_fileTabs.push_back(std::make_unique<FileTab>(filename, callback));
    m_fileTabs.back()->init();
}
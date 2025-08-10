#pragma once
#ifndef _FILETAB_H_
#define _FILETAB_H_
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <functional>
#include <memory>
#include "Event.h"

class FileTabManager;

class FileTab
{
public:
    friend class FileTabManager;
    enum class FileWarningCallbackType
    {
        FileWarningCallbackType_OK,
        FileWarningCallbackType_OK_CANCEL,
        FileWarningCallbackType_YES_NO,
        FileWarningCallbackType_SAVE_DISCARD,
        FileWarningCallbackType_SAVE_DISCARD_CANCEL,
        FileWarningCallbackType_LOAD_CANCEL,
        FileWarningCallbackType_SAVE_CLOSE_CANCEL,
    };
    enum class FileWarningCallbackReturnType
    {
        FileWarningCallbackReturnType_OK,
        FileWarningCallbackReturnType_CANCEL,
        FileWarningCallbackReturnType_YES,
        FileWarningCallbackReturnType_NO,
        FileWarningCallbackReturnType_DISCARD,
        FileWarningCallbackReturnType_LOAD,
        FileWarningCallbackReturnType_SAVE,
        FileWarningCallbackReturnType_CLOSE,
    };
    using FileWarningCallback = std::function<FileWarningCallbackReturnType(const std::string &str, FileWarningCallbackType type)>;
    FileTab(const std::string filename, FileWarningCallback _callback);
    ~FileTab();
    int refresh();
    bool ismodified();
    int save(std::string &buffer);
    std::string load(bool *ret);
    bool init();
    bool exists();
    std::string &getContent();
    void modify(const std::string &buffer);

private:
    std::string m_filename;
    bool m_modified;
    bool m_exists;
    bool m_init;
    std::string m_buffer;
    FileWarningCallback m_callback;
    std::fstream m_file;
};

class FileTabManager
{
public:
    FileTabManager();
    ~FileTabManager();
    void addFileTab(const std::string &filename, FileTab::FileWarningCallback callback);
    bool removeFileTab(std::shared_ptr<FileTab> fileTab);
    void refreshFileTabs();
    void renderFileTabs(EventLoop* events);
    void saveFileTab(std::shared_ptr<FileTab>);
    bool changedCurrentTab();
    std::shared_ptr<FileTab> getCurrentFileTab();
    std::shared_ptr<FileTab> getFileTab(std::string filename);
    bool closeFiles();

private:
    std::vector<std::shared_ptr<FileTab>> m_fileTabs;
    std::shared_ptr<FileTab> m_currentFileTab = nullptr;
    bool m_changed;
};

#endif
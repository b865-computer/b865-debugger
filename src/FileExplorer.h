#pragma once

#ifndef _FILE_EXPLORER_H_
#define _FILE_EXPLORER_H_

#include <imgui.h>
#include <filesystem>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <functional>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/inotify.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

// File watching utilities
class FileWatcher {
public:
    using Callback = std::function<void(const std::string& path)>;

    FileWatcher(const std::string& path, Callback callback);
    ~FileWatcher();

private:
    std::string _path;
    Callback _callback;
    std::atomic<bool> _running;
    std::thread _watchThread;

    void watch();
};

// ImGui File Explorer with Tree
class FileExplorer {
public:
    using FileOpenCallback = std::function<void(const std::string&)>;
    using FileRefreshCallback = std::function<void(void)>;
    FileExplorer(const std::string& rootPath, FileOpenCallback onFileOpenCallback = nullptr, FileRefreshCallback onFileRefreshCallback = nullptr);
    void render(bool disableImGuiBegin = false);
    void setDirectory(const std::string& newRootPath);

private:
    struct Entry {
        fs::path path;
        bool isDirectory;
        bool isExpanded = false;
    };

    std::string _rootPath;
    std::string _currentPath;
    std::unordered_map<std::string, std::vector<Entry>> _directoryCache;
    std::unique_ptr<FileWatcher> _fileWatcher;
    std::mutex _mutex;
    FileOpenCallback _onFileOpen;
    FileRefreshCallback _onFileRefresh;

    void loadDirectory(const std::string& path);
    void renderTree(const std::string& path);
};

#endif
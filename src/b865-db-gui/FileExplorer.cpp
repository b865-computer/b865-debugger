#include "FileExplorer.h"
#include <iostream>

FileWatcher::FileWatcher(const std::string &path, Callback callback)
    : _path(path), _callback(callback), _running(true)
{
    _watchThread = std::thread([this]
                               { this->watch(); });
}

FileWatcher::~FileWatcher()
{
    _running = false;
    _watchThread.detach();
}

void FileWatcher::watch()
{
#ifdef _WIN32
    HANDLE hDir = CreateFileA(
        _path.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL);

    if (hDir == INVALID_HANDLE_VALUE)
        return;

    char buffer[1024];
    DWORD bytesReturned;

    while (_running)
    {
        if (ReadDirectoryChangesW(hDir, buffer, sizeof(buffer), FALSE,
                                  FILE_NOTIFY_CHANGE_FILE_NAME |
                                      FILE_NOTIFY_CHANGE_DIR_NAME |
                                      FILE_NOTIFY_CHANGE_LAST_WRITE,
                                  &bytesReturned, NULL, NULL))
        {
            _callback(_path);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    CloseHandle(hDir);
#else
    int fd = inotify_init();
    if (fd < 0)
        return;

    int wd = inotify_add_watch(fd, _path.c_str(), IN_MODIFY | IN_CREATE | IN_DELETE);
    if (wd < 0)
        return;

    char buffer[1024];
    while (_running)
    {
        int length = read(fd, buffer, sizeof(buffer));
        if (length > 0)
        {
            _callback(_path);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    inotify_rm_watch(fd, wd);
    close(fd);
#endif
    _running = true;
}

FileExplorer::FileExplorer(const std::string &rootPath, FileOpenCallback onFileOpenCallback, FileRefreshCallback onFileRefreshCallback)
    : _rootPath(rootPath), _currentPath(rootPath), _onFileOpen(onFileOpenCallback), _onFileRefresh(onFileRefreshCallback)
{
    loadDirectory(_rootPath);

    // Start file watcher
    _fileWatcher = std::make_unique<FileWatcher>(_rootPath, [this](const std::string &)
                                                 {
            std::lock_guard<std::mutex> lock(_mutex);
            loadDirectory(_rootPath); 
            if (_onFileRefresh)
            {
                _onFileRefresh();
            }});
}

void FileExplorer::render(bool disableImGuiBegin)
{
    std::lock_guard<std::mutex> lock(_mutex);
    bool render = true;
    if (!disableImGuiBegin)
    {
        render = ImGui::Begin("File Explorer");
    }
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 5);
    if (render)
    {
        renderTree(_rootPath);
    }
    ImGui::PopStyleVar();
    if (!disableImGuiBegin)
    {
        ImGui::End();
    }
}

void FileExplorer::setDirectory(const std::string &newRootPath)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _rootPath = newRootPath;
    _currentPath = newRootPath;
    loadDirectory(_rootPath);

    // Restart file watcher
    _fileWatcher = std::make_unique<FileWatcher>(_rootPath, [this](const std::string &)
                                                 {
            std::lock_guard<std::mutex> lock(_mutex);
            loadDirectory(_rootPath); });
}

void FileExplorer::loadDirectory(const std::string &path)
{
    std::vector<Entry> entries;
    try
    {
        for (const auto &entry : fs::directory_iterator(path))
        {
            if (entry.path().filename() == ".git") {
                continue;
            }
            entries.push_back({entry.path(), entry.is_directory()});
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error loading directory " << path << ": " << e.what() << std::endl;
    }
    _directoryCache[fs::absolute(path).string()] = entries;
}

void FileExplorer::renderTree(const std::string &path)
{
    auto it = _directoryCache.find(fs::absolute(path).string());
    if (it == _directoryCache.end())
    {
        loadDirectory(path);
        it = _directoryCache.find(fs::absolute(path).string());
        assert(it != _directoryCache.end());
    }

    auto &entries = it->second;

    for (auto &entry : entries)
    {
        ImGui::PushID(entry.path.string().c_str());
        if (entry.isDirectory)
        {
            if (ImGui::TreeNodeEx(entry.path.filename().string().c_str(), entry.isExpanded ? ImGuiTreeNodeFlags_DefaultOpen : 0))
            {
                entry.isExpanded = true;
                renderTree(entry.path.string());
                ImGui::TreePop();
            }
            else
            {
                entry.isExpanded = false;
            }
        }
        else
        {
            ImGui::TreeNodeEx(entry.path.filename().string().c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);

            // Detect double-click and trigger callback
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                if (_onFileOpen) {
                    _onFileOpen(entry.path.string()); // Call the callback
                }
            }
        }
        ImGui::PopID();
    }
}
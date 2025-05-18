#include "gui.h"

GUI::GUI()
    :  m_emulator([this](const std::string& str)
    {
        displayError(str.c_str());
    }), m_CPUStatus(m_emulator.m_cpu.getStatus()), m_cpu(m_emulator.m_cpu), m_clock(m_emulator.m_clock) {}

GUI::~GUI() {}

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImGuiFileDialog.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include "TextEditor.h"
#include "FileExplorer.h"
#include "FileTab.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdio.h>
#include <stdlib.h>

FileTabManager fileTabManager;

FileTab::FileWarningCallbackReturnType fileCallBack(const std::string &str, FileTab::FileWarningCallbackType type)
{
    // TODO: implement a real callback
    (void)type;
    fprintf(stderr, "FileTab::FileWarningCallback: %s\n", str.c_str());
    return FileTab::FileWarningCallbackReturnType::FileWarningCallbackReturnType_NO;
}

void openFile(const std::string &path)
{
    fileTabManager.addFileTab(path, fileCallBack);
}

void refreshFiles()
{
    fileTabManager.refreshFileTabs();
}

FileExplorer explorer(".", openFile, refreshFiles);

TextEditor editor;

TextEditor::LanguageDefinition AssemblyLangDef();

const char *noOpenedFileText = "No opened souce files.";
bool customHzInput = false;
bool isRunning = false;
uint64_t customFrequencyHZ = 1;
bool fileOpenInput = false;
bool showRealFrequency = false;
std::string openedFileName = "";
bool changedSource = false;
bool console = true;

GUI *gui;

FileInputType fileOpenInputType = projectFile;

int toolBarImage_width = 0;
int toolBarImage_height = 0;
int errorImage_width;
int errorImage_height;
GLuint resetImage_id = 0;
GLuint stopImage_id = 0;
GLuint startImage_id = 0;
GLuint tickImage_id = 0;
GLuint errorImage_id = 0;

int sideBarImage_width = 0;
int sideBarImage_height = 0;
GLuint explorerImage_id = 0;
GLuint debuggerImage_id = 0;
GLuint emulatorImage_id = 0;

Window_Attrib window_main("main", true, false, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);
Window_Attrib window_toolBar("ToolBar", true, false);
Window_Attrib window_sideBar("SideBar", true);
Window_Attrib window_sideTool("SideTool", true, false, ImGuiWindowFlags_HorizontalScrollbar);
Window_Attrib window_fileopen("FilesOpened", true, true);
Window_Attrib window_editor("Editor", true);
Window_Attrib window_console("Console", true);

void error_callback(int error, const char *description)
{
    fprintf(stderr, "Error %i: %s\n", error, description);
}

// Simple helper function to load an image into a OpenGL texture with common settings
bool GUI::LoadTextureFromMemory(const void *data, uint64_t data_size, GLuint *out_texture, int *out_width, int *out_height)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char *image_data = stbi_load_from_memory((const unsigned char *)data, (int)data_size, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload pixels into texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

// Open and read a file, then forward to LoadTextureFromMemory()
bool GUI::LoadTextureFromFile(const char *file_name, GLuint *out_texture, int *out_width, int *out_height)
{
    FILE *f = fopen(file_name, "rb");
    if (f == NULL)
        return false;
    fseek(f, 0, SEEK_END);
    uint64_t file_size = (uint64_t)ftell(f);
    if (file_size == (uint64_t)-1)
        return false;
    fseek(f, 0, SEEK_SET);
    void *file_data = IM_ALLOC(file_size);
    if (fread(file_data, 1, file_size, f) != file_size)
    {
        IM_FREE(file_data);
        fclose(f);
        return false;
    }
    bool ret = LoadTextureFromMemory(file_data, file_size, out_texture, out_width, out_height);
    IM_FREE(file_data);
    fclose(f);
    return ret;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    (void)scancode;
    (void)mods;
    bool control = (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL));
    if (key == GLFW_KEY_O && control && action == GLFW_PRESS)
    {
        fileOpenInput = true;
        fileOpenInputType = projectFile;
    }
    if (key == GLFW_KEY_T && control && action == GLFW_PRESS)
    {
        isRunning = !isRunning;
    }
    if (key == GLFW_KEY_S && control && action == GLFW_PRESS)
    {
        if (openedFileName.size())
        {
            std::ofstream file(openedFileName);
            if (file.is_open())
            {
                auto text = editor.GetText();
                if (text[text.size() - 1] == '\n')
                {
                    text.pop_back();
                }
                file << text;
                file.close();
                changedSource = true;
            }
            else
            {
                fprintf(stderr, "Unable to open file: %s\n", openedFileName.c_str());
                gui->displayError("Unable to open file: %s\n", openedFileName.c_str());
            }
        }
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    (void)width;
    (void)height;
    gui->render();
    glfwSwapBuffers(window);
}

void GUI::terminate()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);

    glfwTerminate();
}

int GUI::init()
{
    if (m_emulator.init())
    {
        return 1;
    }
    gui = this;

    window_main.addChild(&window_toolBar);
    window_main.addChild(&window_sideBar);
    window_main.addChild(&window_sideTool);
    window_main.addChild(&window_fileopen);
    window_main.addChild(&window_editor);
    window_main.addChild(&window_console);

    window_toolBar.setRenderFunc(renderToolBar);
    window_sideBar.setRenderFunc(renderSideBar);
    window_sideTool.setRenderFunc(renderSideTool);
    window_editor.setRenderFunc(renderEditor);
    window_console.setRenderFunc(renderConsole);
    window_fileopen.setRenderFunc(renderFilesOpened);

    mainWindow = &window_main;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    window = glfwCreateWindow(800, 700, "Emulator", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);

    // printWindowLayouts(mainWindow);

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    // io.Fonts->AddFontDefault();
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    // IM_ASSERT(font != nullptr);

    toolBarImage_width = 0;
    toolBarImage_height = 0;
    resetImage_id = 0;
    bool ret = LoadTextureFromFile(getFilePathFromExeRelative("./resources/reset.png").c_str(), &resetImage_id, &toolBarImage_width, &toolBarImage_height);
    IM_ASSERT(ret);
    stopImage_id = 0;
    ret = LoadTextureFromFile(getFilePathFromExeRelative("./resources/stop.png").c_str(), &stopImage_id, &toolBarImage_width, &toolBarImage_height);
    IM_ASSERT(ret);
    startImage_id = 0;
    ret = LoadTextureFromFile(getFilePathFromExeRelative("./resources/start.png").c_str(), &startImage_id, &toolBarImage_width, &toolBarImage_height);
    IM_ASSERT(ret);
    tickImage_id = 0;
    ret = LoadTextureFromFile(getFilePathFromExeRelative("./resources/tick.png").c_str(), &tickImage_id, &toolBarImage_width, &toolBarImage_height);
    IM_ASSERT(ret);

    errorImage_id = 0;
    ret = LoadTextureFromFile(getFilePathFromExeRelative("./resources/error.png").c_str(), &errorImage_id, &errorImage_width, &errorImage_height);
    IM_ASSERT(ret);

    explorerImage_id = 0;
    ret = LoadTextureFromFile(getFilePathFromExeRelative("./resources/folder-outline.png").c_str(), &explorerImage_id, &sideBarImage_width, &sideBarImage_height);
    IM_ASSERT(ret);
    emulatorImage_id = 0;
    ret = LoadTextureFromFile(getFilePathFromExeRelative("./resources/memory.png").c_str(), &emulatorImage_id, &sideBarImage_width, &sideBarImage_height);
    IM_ASSERT(ret);
    debuggerImage_id = 0;
    ret = LoadTextureFromFile(getFilePathFromExeRelative("./resources/bug-play.png").c_str(), &debuggerImage_id, &sideBarImage_width, &sideBarImage_height);
    IM_ASSERT(ret);

    editor.SetText(noOpenedFileText);
    editor.SetLanguageDefinition(AssemblyLangDef());
    editor.SetReadOnly(false);
    editor.SetTabSize(4);
    editor.SetImGuiChildIgnored(true);

    return 0;
}

void GUI::renderMenu()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open Project", "Ctrl+O"))
            {
                fileOpenInput = true;
                fileOpenInputType = projectFile;
            }
            if (ImGui::MenuItem("Open .hex Program"))
            {
                fileOpenInput = true;
                fileOpenInputType = programFile;
            }
            if (ImGui::MenuItem("Exit", "Alt+F4"))
            {
                glfwSetWindowShouldClose(window, true);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Emulator"))
        {
            if (ImGui::MenuItem("Build project", "Ctrl+B"))
            {
                building = true;
            }
            if (ImGui::MenuItem("Start/Stop", "Ctrl+S", &isRunning))
            {
                m_clock.setStatus(!m_clock.getStatus());
            }
            if (ImGui::BeginMenu("Frequency"))
            {
                if (ImGui::MenuItem("Custom"))
                {
                    customHzInput = true;
                }
                if (ImGui::MenuItem("10 MHz", nullptr, (m_frequencyHZ == 10000000)))
                {
                    m_frequencyHZ = 10000000;
                }
                if (ImGui::MenuItem("5 MHz", nullptr, (m_frequencyHZ == 5000000)))
                {
                    m_frequencyHZ = 5000000;
                }
                if (ImGui::MenuItem("2 MHz", nullptr, (m_frequencyHZ == 2000000)))
                {
                    m_frequencyHZ = 2000000;
                }
                if (ImGui::MenuItem("1 MHz", nullptr, (m_frequencyHZ == 1000000)))
                {
                    m_frequencyHZ = 1000000;
                }
                if (ImGui::MenuItem("500 kHz", nullptr, (m_frequencyHZ == 500000)))
                {
                    m_frequencyHZ = 500000;
                }
                if (ImGui::MenuItem("100 kHz", nullptr, (m_frequencyHZ == 100000)))
                {
                    m_frequencyHZ = 100000;
                }
                if (ImGui::MenuItem("50 kHz", nullptr, (m_frequencyHZ == 50000)))
                {
                    m_frequencyHZ = 50000;
                }
                if (ImGui::MenuItem("10 kHz", nullptr, (m_frequencyHZ == 10000)))
                {
                    m_frequencyHZ = 10000;
                }
                if (ImGui::MenuItem("5 khz", nullptr, (m_frequencyHZ == 5000)))
                {
                    m_frequencyHZ = 5000;
                }
                if (ImGui::MenuItem("1 khz", nullptr, (m_frequencyHZ == 1000)))
                {
                    m_frequencyHZ = 1000;
                }
                if (ImGui::MenuItem("500 Hz", nullptr, (m_frequencyHZ == 500)))
                {
                    m_frequencyHZ = 500;
                }
                if (ImGui::MenuItem("100 Hz", nullptr, (m_frequencyHZ == 100)))
                {
                    m_frequencyHZ = 100;
                }
                if (ImGui::MenuItem("50 Hz", nullptr, (m_frequencyHZ == 50)))
                {
                    m_frequencyHZ = 50;
                }
                if (ImGui::MenuItem("10 Hz", nullptr, (m_frequencyHZ == 10)))
                {
                    m_frequencyHZ = 10;
                }
                if (ImGui::MenuItem("5 Hz", nullptr, (m_frequencyHZ == 5)))
                {
                    m_frequencyHZ = 5;
                }
                if (ImGui::MenuItem("1 Hz", nullptr, (m_frequencyHZ == 1)))
                {
                    m_frequencyHZ = 1;
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Instruction-level", nullptr, &ins_level))
            {
                ins_level = !ins_level;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Show Frequency", nullptr, showRealFrequency))
            {
                showRealFrequency = !showRealFrequency;
            }
            if (ImGui::MenuItem("Show Terminal", nullptr, console))
            {
                console = !console;
                window_console.visible = console;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

int GUI::render()
{
    int display_w, display_h;
    auto& io = ImGui::GetIO();

    glfwGetFramebufferSize(window, &display_w, &display_h);
    
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    renderMenu();

    ImGui::SetNextWindowPos(ImVec2(0, 20));
    ImGui::SetNextWindowSize(ImVec2(display_w, display_h - 20));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    mainWindow->render();
    ImGui::PopStyleVar(1);

    if (customHzInput)
    {
        ImGui::OpenPopup("Frequency");
        customHzInput = false;
    }
    if (ImGui::BeginPopupModal("Frequency", &customHzInput, ImGuiWindowFlags_AlwaysAutoResize)) // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
    {
        ImGui::InputInt("Hz", (int *)&customFrequencyHZ);
        if (ImGui::Button("Ok"))
        {
            m_frequencyHZ = customFrequencyHZ;
            ImGui::CloseCurrentPopup();
        }
        ImGui::End();
    }
    if (m_clock.getHZ() != m_frequencyHZ)
    {
        bool running = m_clock.getStatus();
        m_clock.setStatus(false);
        m_clock.setHZ(m_frequencyHZ);
        m_clock.setStatus(running);
        isRunning = false;
    }

    if (error_display)
    {
        ImGui::OpenPopup("Error");
        error_display = false;
    }
    if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Image(errorImage_id, ImVec2(errorImage_width, errorImage_height));
        ImGui::SameLine();
        ImGui::BeginChild("##text&button", ImVec2(0, 0), ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
        ImGui::Text(error_str.c_str());
        if (ImGui::Button("Ok"))
        {
            error_display = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndChild();
        ImGui::EndPopup();
    }
    if (building)
    {
        if (projectPath == "")
        {
            displayError("Please open a project first.");
            building = false;
        }
        else
        {
            buildRunning = true;
            changedSource = false;
            isRunning = false;
            m_clock.setStatus(isRunning);
            ImGui::OpenPopup("Building");
            building = false;
        }
    }
    if (ImGui::BeginPopupModal("Building", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Building project, please wait.");
        if (!buildRunning)
        {
            building = false;
            NewProjectOpened = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::Render();
    glViewport(0, 0, display_w, display_h);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
    return 0;
}

int GUI::load(std::string filename, std::string path)
{
    printf("loading file: %s\n", filename.c_str());
    if(path.size() != 0)
    {
        filename = path + "/" + filename;
        projectPath = path;
    }
    else
    {
        path = projectPath = getPath(filename);
    }
    
    if(isExtEqual(filename, "b865"))
    {
        projectFileName = filename;
        NewProjectOpened = false;
    }
    if (m_emulator.load(filename))
    {
        return 1;
    }
    explorer.setDirectory(projectPath);
    return 0;
}

int GUI::main()
{
    std::string outputLines;
    ConsoleText = &outputLines;
    M_PROCESS_OUT buildProcessOut = nullptr;
    M_PROCESS buildProcess;
    bool buildProcessRunning = false;
    std::string buildCmd;
    m_emulator.start();

    while (!glfwWindowShouldClose(window))
    {
        m_emulator.main();
        if(!m_clock.getStatus())
        {
            currentPosition = m_emulator.m_debuggerData.getPosition(m_cpu.getStatus().PC.addr - 1);
        }
        
        if (NewProjectOpened)
        {
            if(!load(projectFileName))
            {
                m_cpu.startExec();
            }
        }
        
        if(buildRunning && !buildProcessRunning)
        {
            buildCmd = "make";
            outputLines.clear();
            outputLines += buildCmd + "\n";
            buildProcessOut = startProgram(projectPath, buildCmd, buildProcess);
            if(buildProcess == M_PROCESS_INVALID || buildProcessOut == M_PROCESS_INVALID)
            {
                displayError("Failed to start build process");
                buildRunning = false;
            }
            else
            {
                buildProcessRunning = true;
            }
        }
        if (buildProcessRunning)
        {
            buildProcessRunning = pollProgramOutput(buildProcessOut, outputLines);
            if(!buildProcessRunning)
            {
                unsigned long exitCode;
                if((exitCode = programExitCode(buildProcess, buildProcessOut, &buildProcessRunning)))
                {
                    displayError("Build process exited with code: %d", exitCode);
                }
                if(!buildProcessRunning)
                {
                    outputLines += ("Process exited with code: " + std::to_string(exitCode) + "\n");
                }
            }
            buildRunning = buildProcessRunning;
        }

        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != GLFW_FALSE || glfwGetWindowAttrib(window, GLFW_VISIBLE) != GLFW_TRUE)
        {
            ImGui_ImplGlfw_Sleep(100);
            continue;
        }
        render();
        m_clock.setStatus(isRunning);
        if (isRunning && changedSource)
        {
            building = true;
        }
        glfwSwapBuffers(window);
        ImGui_ImplGlfw_Sleep(10);
    }
    terminate();
    m_emulator.terminate();
    return 0;
}

void GUI::displayError(const char *fmt, ...)
{
    error_display = true;
    va_list args;
    va_start(args, fmt); // Initialize the argument list

    // Determine the required buffer size
    int size = vsnprintf(nullptr, 0, fmt, args) + 1; // +1 for null terminator
    va_end(args);

    // Allocate a buffer of the required size
    std::vector<char> buffer(size);

    // Format the string into the buffer
    va_start(args, fmt);
    vsnprintf(buffer.data(), size, fmt, args);
    va_end(args);

    buffer[size - 1] = '\0';

    // Store the formatted string in error_str
    error_str = std::string(buffer.data());
}

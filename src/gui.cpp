#include "gui.h"

GUI::GUI(const CPU_Status &status, Clock &clock, CPU &cpu, std::vector<debugSym> &symbolData)
    : m_CPUStatus(status), m_clock(clock), m_cpu(cpu), m_symbolData(symbolData)
{
    m_pheripherials = nullptr;
    m_pheriphCount = 0;
}

GUI::~GUI()
{
    if (m_pheripherials)
    {
        delete[] m_pheripherials;
    }
}

bool GUI::windowClosed()
{
    return end;
}

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImGuiFileDialog.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include "TextEditor.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdio.h>
#include <stdlib.h>

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

Window_Attrib window_main("main", 0, 20, 0, 0, true, 0);
Window_Attrib window_side_tool("side_tool", 0, 0, 248, 0, true, 1);
Window_Attrib window_toolBar("ToolBar", 0, 0, 248, 26, true, 1, true);
Window_Attrib window_side_bar_tool("side_bar_tool", 0, 26, 248, 0, true, 0);
Window_Attrib window_sideBar("SideBar", 0, 0, 48, 0, true, 1);
Window_Attrib window_sideTool("SideTool", 48, 0, 200, 0, true, 0);
Window_Attrib window_editor_console("editor_console", 248, 0, 0, 0, true, 0);
Window_Attrib window_fileopen("FilesOpened", 0, 0, 0, 26, true, 1, true);
Window_Attrib window_editor("Editor", 0, 26, 0, 0, true, 0);
Window_Attrib window_console("Console", 0, 0, 0, -300, true, 2);

void error_callback(int error, const char *description)
{
    fprintf(stderr, "Error: %s\n", description);
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
    if (file_size == -1)
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
    gui->mainWindow->adjustLayout(width, height);
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

void printWindowLayouts(Window_Attrib *window, int indent = 0)
{
    fprintf(stdout, "%*s%s: posX: %d, posY: %d, width: %d, height: %d\n", indent, "", window->name.c_str(), window->x + window->offsetFromOriginX, window->y + window->offsetFromOriginY, window->width, window->height);
    indent++;
    for (auto child : window->children)
    {
        printWindowLayouts(child, indent);
    }
}

int GUI::init()
{
    gui = this;
    m_pheripherials = m_cpu.mem.getPheripherials(&m_pheriphCount);

    window_main.addChild(&window_side_tool);
    window_side_tool.addChild(&window_toolBar);
    window_side_tool.addChild(&window_side_bar_tool);
    window_side_bar_tool.addChild(&window_sideBar);
    window_side_bar_tool.addChild(&window_sideTool);
    window_side_bar_tool.setLayout(Window_Attrib::LayoutType::Horizontal);
    window_side_tool.setLayout(Window_Attrib::LayoutType::Vertical);
    window_main.addChild(&window_editor_console);
    window_editor_console.addChild(&window_fileopen);
    window_editor_console.addChild(&window_editor);
    window_editor_console.addChild(&window_console);
    window_editor_console.setLayout(Window_Attrib::LayoutType::Vertical);
    window_main.setLayout(Window_Attrib::LayoutType::Horizontal);

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
    io.IniFilename = NULL;

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    mainWindow->adjustLayout(display_w, display_h);

    printWindowLayouts(mainWindow);

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
    glfwGetFramebufferSize(window, &display_w, &display_h);

    mainWindow->adjustLayout(display_w, display_h);

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // ImGui::GetIO();

    renderMenu();

    mainWindow->render();

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
    return 0;
}

int GUI::main()
{
    if (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != GLFW_FALSE || glfwGetWindowAttrib(window, GLFW_VISIBLE) != GLFW_TRUE)
        {
            ImGui_ImplGlfw_Sleep(100);
            return 0;
        }
        render();
        m_clock.setStatus(isRunning);
        if (isRunning && changedSource)
        {
            building = true;
        }
        glfwSwapBuffers(window);
    }
    else
    {
        end = true;
    }
    ImGui_ImplGlfw_Sleep(10);
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

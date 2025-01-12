#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImGuiFileDialog.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include "gui.h"
#include "TextEditor.h"
#include "FilePath.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdlib.h>
#include <stdio.h>

TextEditor editor;

TextEditor::LanguageDefinition AssemblyLangDef();

const char *noOpenedFileText = "No opened souce files.";
bool customHzInput = false;
bool isRunning = false;
uint64_t customFrequencyHZ = 1;
bool fileOpenInput = false;
bool showRealFrequency = false;
std::string openedFileName = "";
std::string Path = "";
bool changedSource = false;
bool console = false;

enum FileInputType
{
    programFile = 0,
    projectFile,
};
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

void error_callback(int error, const char *description)
{
    fprintf(stderr, "Error: %s\n", description);
}

// Simple helper function to load an image into a OpenGL texture with common settings
bool GUI::LoadTextureFromMemory(const void *data, unsigned long long data_size, GLuint *out_texture, int *out_width, int *out_height)
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
    unsigned long long file_size = (unsigned long long)ftell(f);
    if (file_size == -1)
        return false;
    fseek(f, 0, SEEK_SET);
    void *file_data = IM_ALLOC(file_size);
    fread(file_data, 1, file_size, f);
    bool ret = LoadTextureFromMemory(file_data, file_size, out_texture, out_width, out_height);
    IM_FREE(file_data);
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
            std::ofstream file(Path + '/' + openedFileName);
            if (file.is_open())
            {
                file << editor.GetText();
                file.close();
                changedSource = true;
            }
            else
            {
                fprintf(stderr, "Unable to open file: %s\n", openedFileName.c_str());
            }
        }
    }
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
    m_pheripherials = m_cpu.mem.getPheripherials(&m_pheriphCount);

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

    glfwSetKeyCallback(window, key_callback);

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
    bool ret = LoadTextureFromFile(getFilePathFromExeRelative("./share/reset.png").c_str(), &resetImage_id, &toolBarImage_width, &toolBarImage_height);
    IM_ASSERT(ret);
    stopImage_id = 0;
    ret = LoadTextureFromFile(getFilePathFromExeRelative("./share/stop.png").c_str(), &stopImage_id, &toolBarImage_width, &toolBarImage_height);
    IM_ASSERT(ret);
    startImage_id = 0;
    ret = LoadTextureFromFile(getFilePathFromExeRelative("./share/start.png").c_str(), &startImage_id, &toolBarImage_width, &toolBarImage_height);
    IM_ASSERT(ret);
    tickImage_id = 0;
    ret = LoadTextureFromFile(getFilePathFromExeRelative("./share/tick.png").c_str(), &tickImage_id, &toolBarImage_width, &toolBarImage_height);
    IM_ASSERT(ret);
    errorImage_id = 0;
    ret = LoadTextureFromFile(getFilePathFromExeRelative("./share/error.png").c_str(), &errorImage_id, &errorImage_width, &errorImage_height);
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
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

int GUI::mainLoop()
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
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);

        // ImGui::GetIO();

        renderMenu();

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings;

        if (ImGui::Begin("Status", nullptr, flags))
        {
            ImGui::SetWindowPos(ImVec2(0, 44));
            ImGui::SetWindowSize(ImVec2(display_w > 200 ? 200 : display_w, display_h - 44));
            ImGui::Text("Frequency: %lliHz", m_frequencyHZ);
            if (showRealFrequency)
            {
                ImGui::Text("Real freq. %.0fHz", ((double)m_clock.getCycles() / ((double)m_clock.getRunTime_ns().count() / 1e9)));
            }

            if (ImGui::CollapsingHeader("Registers"))
            {
                ImGui::Text("PC: 0x%04X", m_CPUStatus.PC.addr);
                ImGui::Text("A: 0x%02X", m_CPUStatus.A);
                ImGui::Text("B: 0x%02X", m_CPUStatus.B);
                ImGui::Text("IR0: 0x%02X", m_CPUStatus.IR0);
                ImGui::Text("IR1: 0x%02X", m_CPUStatus.IR1);
                ImGui::Text("AR: 0x%02X", m_CPUStatus.AR);
                ImGui::Separator();
                ImGui::Text("ACC: 0x%02X", m_CPUStatus.registers[0]);
                ImGui::Text("X: 0x%02X", m_CPUStatus.registers[1]);
                ImGui::Text("Y: 0x%02X", m_CPUStatus.registers[2]);
                ImGui::Text("SP: 0x%02X", m_CPUStatus.registers[3]);
                ImGui::Text("R0: 0x%02X", m_CPUStatus.registers[4]);
                ImGui::Text("R1: 0x%02X", m_CPUStatus.registers[5]);
                ImGui::Text("R2: 0x%02X", m_CPUStatus.registers[6]);
                ImGui::Text("R3: 0x%02X", m_CPUStatus.registers[7]);
            }

            if (ImGui::CollapsingHeader("Flags"))
            {
                ImGui::Text("Carry: %i", m_CPUStatus.flags.carry);
                ImGui::Text("Zero: %i", m_CPUStatus.flags.zero);
                ImGui::Text("Negative: %i", m_CPUStatus.flags.negative);
            }

            if (ImGui::CollapsingHeader("Emulation internals"))
            {
                ImGui::Text("MAR: 0x%04X", m_CPUStatus.MAR.addr);
                ImGui::Text("InsCycle: %i", m_CPUStatus.InsCycle);
                ImGui::Text("AdrCycle: %i", m_CPUStatus.AdrCycle);
                ImGui::Text("Addressing: %s", m_CPUStatus.AdrState ? "true" : "false");
                ImGui::Text("Signals: 0x%08x", m_CPUStatus.signals);
                ImGui::Text("RI: %i", m_CPUStatus.RI);
                ImGui::Text("RO: %i", m_CPUStatus.RO);
                ImGui::Text("ALU OP: %i", m_CPUStatus.ALU_OP);
            }

            for (int i = 0; i < m_pheriphCount; i++)
            {
                if (ImGui::CollapsingHeader(m_pheripherials[0]->m_name.c_str()))
                {
                    for (int i = 0; i < m_pheripherials[0]->m_regNames.size(); i++)
                    {
                        ImGui::Text(" %s: %i",
                                    m_pheripherials[0]->m_regNames[i].c_str(),
                                    m_pheripherials[0]->regs[i]);
                    }
                }
            }

            if (ImGui::CollapsingHeader("Symbols"))
            {
                for (int i = 0; i < m_symbolData.size(); i++)
                {
                    ImGui::Text("%s: 0x%04X", m_symbolData[i].symbol.c_str(), m_symbolData[i].address);
                }
            }
        }
        ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2, 2));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        if (ImGui::Begin("ToolBar", nullptr, flags))
        {
            ImGui::SetWindowPos(ImVec2(0, 20));
            ImGui::SetWindowSize(ImVec2(display_w < 200 ? display_w : 200, 30));
            if (ImGui::ImageButton("reset", resetImage_id, ImVec2(toolBarImage_height, toolBarImage_width)))
            {
                m_cpu.startExec();
            }
            ImGui::SameLine();
            if (m_clock.getStatus())
            {
                if (ImGui::ImageButton("stop", stopImage_id, ImVec2(toolBarImage_height, toolBarImage_width)))
                {
                    isRunning = false;
                }
            }
            else
            {
                if (ImGui::ImageButton("start", startImage_id, ImVec2(toolBarImage_height, toolBarImage_width)))
                {
                    isRunning = true;
                }
            }
            ImGui::SameLine();
            if (ImGui::ImageButton("tick", tickImage_id, ImVec2(toolBarImage_height, toolBarImage_width)))
            {
                m_clock.singleCycle();
            }
        }
        ImGui::End();

        ImGui::PopStyleVar(2);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2, 2));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        if (ImGui::Begin("FilesOpened", nullptr, flags))
        {
            ImGui::SetWindowPos(ImVec2(display_w > 200 ? 200 : display_w, 20));
            ImGui::SetWindowSize(ImVec2(display_w > 200 ? display_w - 200 : 0, 30));
            if (sourceFileNames.size() <= 1)
            {
                editor.SetText(noOpenedFileText);
            }
            for (unsigned long long i = 1; i < sourceFileNames.size(); i++) // start with 1 because 0 is the .hex file name
            {
                bool currentBreakpoint = false;
                if (i == currentPosition.fileID)
                {
                    currentBreakpoint = true;
                }
                if (ImGui::Button(sourceFileNames[i].c_str()) || (currentBreakpoint && currentPosition.address != lastPosition))
                {
                    lastPosition = currentPosition.address;
                    if (openedFileName != sourceFileNames[i])
                    {
                        openedFileName = sourceFileNames[i];
                        std::ifstream file(projectPath + '/' + openedFileName);
                        if (file.is_open())
                        {
                            std::stringstream buffer;
                            buffer << file.rdbuf();
                            editor.SetText(buffer.str());
                            file.close();
                        }
                        else
                        {
                            fprintf(stderr, "Unable to open file: %s\n", openedFileName.c_str());
                        }
                    }
                    if (currentBreakpoint)
                    {
                        TextEditor::Breakpoints brps;
                        brps.insert(currentPosition.line);
                        editor.SetBreakpoints(brps);
                    }
                }
                ImGui::SameLine();
            }
        }
        ImGui::End();

        ImGui::PopStyleVar(2);

        ImGui::SetNextWindowPos(ImVec2(200, 44));
        ImGui::SetNextWindowSize(ImVec2(display_w > 200 ? display_w - 200 : 0, display_h > (44 + (console ? 300 : 0)) ? display_h - (44 + (console ? 300 : 0)) : 0));

        if (fileOpenInput)
        {
            // open Dialog Simple
            IGFD::FileDialogConfig config;
            config.flags = ImGuiFileDialogFlags_DisableCreateDirectoryButton;
            config.path = ".";
            std::string filters;
            if (fileOpenInputType == projectFile)
            {
                filters = ".b865";
            }
            else
            {
                filters = ".hex";
            }
            ImGuiFileDialog::Instance()->OpenDialog("0", "Choose File", filters.c_str(), config);

            // display
            if (ImGuiFileDialog::Instance()->Display("0", flags,
                                                     ImVec2(display_w > 200 ? display_w - 200 : 0, display_h > 44 ? display_h - 44 : 0),
                                                     ImVec2(display_w > 200 ? display_w - 200 : 0, display_h > (44 + (console ? 300 : 0)) ? display_h - (44 + (console ? 300 : 0)) : 0)))
            {
                if (ImGuiFileDialog::Instance()->IsOk())
                {
                    std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                    if (fileOpenInputType == projectFile)
                    {
                        NewProjectOpened = true;
                        projectFileName = filePathName;
                        for (int i = (filePathName.length() - 1); i > 0; i--)
                        {
                            char c = filePathName.at(i);
                            if (c == '/' || c == '\\')
                            {
                                projectPath = filePathName.substr(0, i);
                                Path = projectPath;
                                break;
                            }
                        }
                    }
                    else
                    {
                        isRunning = false;
                        m_clock.setStatus(isRunning);
                        m_cpu.loadProgramFromFile(filePathName);
                        m_cpu.startExec();
                        editor.SetText(noOpenedFileText);
                    }
                }
                // close
                ImGuiFileDialog::Instance()->Close();

                fileOpenInput = false;
            }
        }
        else
        {
            ImGui::Begin("editor", nullptr, flags);
            editor.Render("");
            ImGui::End();
        }

        m_clock.setStatus(isRunning);
        if(isRunning && changedSource)
        {
            building = true;
        }

        if (customHzInput)
        {
            ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings;
            ImGui::SetWindowPos(ImVec2(265, 165));
            ImGui::SetWindowSize(ImVec2(195, 80));
            ImGui::Begin("Frequency:", &customHzInput, flags); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::InputInt("Hz", (int *)&customFrequencyHZ);
            if (ImGui::Button("Ok"))
            {
                customHzInput = false;
                m_frequencyHZ = customFrequencyHZ;
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

        ImGui::PopStyleVar(); // Popups and the console need border.

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

        if(console && ImGui::Begin("Console", nullptr, flags))
        {
            ImGui::SetWindowPos(ImVec2(200, display_h > 300 ? display_h - 300 : 0));
            ImGui::SetWindowSize(ImVec2(display_w > 200 ? display_w - 200 : 0, 300));
            ImGui::Text("Terminal");
            ImGui::Separator();
            ImGui::BeginChild("#Text");
            for(const auto& line : *ConsoleText)
            {
                ImGui::TextWrapped("%s", line.c_str());
            }
            ImGui::EndChild();
        }
        if(console)
        {
            ImGui::End();
        }

        ImGui::Render();
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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

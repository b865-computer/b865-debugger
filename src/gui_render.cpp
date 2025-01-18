#include "gui.h"
#include "TextEditor.h"
#include "ImGuiFileDialog.h"

extern TextEditor editor;

extern const char *noOpenedFileText;
extern bool customHzInput;
extern bool isRunning;
extern uint64_t customFrequencyHZ;
extern bool fileOpenInput;
extern bool showRealFrequency;
extern std::string openedFileName;
extern bool changedSource;
extern bool console;

extern GUI *gui;

extern FileInputType fileOpenInputType;

extern int toolBarImage_width;
extern int toolBarImage_height;
extern int errorImage_width;
extern int errorImage_height;
extern GLuint resetImage_id;
extern GLuint stopImage_id;
extern GLuint startImage_id;
extern GLuint tickImage_id;
extern GLuint errorImage_id;

extern int sideBarImage_width;
extern int sideBarImage_height;
extern GLuint explorerImage_id;
extern GLuint debuggerImage_id;

void renderSideBar()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);

    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 0.5f));

    if (ImGui::ImageButton("explorer_button", explorerImage_id, ImVec2(sideBarImage_height, sideBarImage_width)))
    {
        gui->sideBarToolType = GUI::ToolType::TOOL_EXPLORER;
    }
    if (ImGui::ImageButton("debugger_button", debuggerImage_id, ImVec2(sideBarImage_height, sideBarImage_width)))
    {
        gui->sideBarToolType = GUI::ToolType::TOOL_DEBUGGER;
    }

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
}

void renderSideTool()
{
    if (gui->sideBarToolType == GUI::ToolType::TOOL_EXPLORER)
    {
    }
    else if (gui->sideBarToolType == GUI::ToolType::TOOL_DEBUGGER)
    {
        ImGui::Text("Frequency: %lliHz", gui->m_frequencyHZ);
        if (showRealFrequency)
        {
            ImGui::Text("Real freq. %.0fHz", ((double)gui->m_clock.getCycles() / ((double)gui->m_clock.getRunTime_ns().count() / 1e9)));
        }

        if (ImGui::CollapsingHeader("Registers"))
        {
            ImGui::Text("PC: 0x%04X", gui->m_CPUStatus.PC.addr);
            ImGui::Text("A: 0x%02X", gui->m_CPUStatus.A);
            ImGui::Text("B: 0x%02X", gui->m_CPUStatus.B);
            ImGui::Text("IR0: 0x%02X", gui->m_CPUStatus.IR0);
            ImGui::Text("IR1: 0x%02X", gui->m_CPUStatus.IR1);
            ImGui::Text("AR: 0x%02X", gui->m_CPUStatus.AR);
            ImGui::Separator();
            ImGui::Text("ACC: 0x%02X", gui->m_CPUStatus.registers[0]);
            ImGui::Text("X: 0x%02X", gui->m_CPUStatus.registers[1]);
            ImGui::Text("Y: 0x%02X", gui->m_CPUStatus.registers[2]);
            ImGui::Text("SP: 0x%02X", gui->m_CPUStatus.registers[3]);
            ImGui::Text("R0: 0x%02X", gui->m_CPUStatus.registers[4]);
            ImGui::Text("R1: 0x%02X", gui->m_CPUStatus.registers[5]);
            ImGui::Text("R2: 0x%02X", gui->m_CPUStatus.registers[6]);
            ImGui::Text("R3: 0x%02X", gui->m_CPUStatus.registers[7]);
        }

        if (ImGui::CollapsingHeader("Flags"))
        {
            ImGui::Text("Carry: %i", gui->m_CPUStatus.flags.carry);
            ImGui::Text("Zero: %i", gui->m_CPUStatus.flags.zero);
            ImGui::Text("Negative: %i", gui->m_CPUStatus.flags.negative);
        }

        if (ImGui::CollapsingHeader("Emulation internals"))
        {
            ImGui::Text("MAR: 0x%04X", gui->m_CPUStatus.MAR.addr);
            ImGui::Text("InsCycle: %i", gui->m_CPUStatus.InsCycle);
            ImGui::Text("AdrCycle: %i", gui->m_CPUStatus.AdrCycle);
            ImGui::Text("Addressing: %s", gui->m_CPUStatus.AdrState ? "true" : "false");
            ImGui::Text("Signals: 0x%08x", gui->m_CPUStatus.signals.val);
            ImGui::Text("RI: %i", gui->m_CPUStatus.RI);
            ImGui::Text("RO: %i", gui->m_CPUStatus.RO);
            ImGui::Text("ALU OP: %i", gui->m_CPUStatus.ALU_OP);
        }

        for (int i = 0; i < gui->m_pheriphCount; i++)
        {
            if (ImGui::CollapsingHeader(gui->m_pheripherials[0]->m_name.c_str()))
            {
                for (int i = 0; i < gui->m_pheripherials[0]->m_regNames.size(); i++)
                {
                    ImGui::Text(" %s: %i",
                                gui->m_pheripherials[0]->m_regNames[i].c_str(),
                                gui->m_pheripherials[0]->regs[i]);
                }
            }
        }

        if (ImGui::CollapsingHeader("Symbols"))
        {
            for (int i = 0; i < gui->m_symbolData.size(); i++)
            {
                ImGui::Text("%s: 0x%04X", gui->m_symbolData[i].symbol.c_str(), gui->m_symbolData[i].address);
            }
        }
    }
}

void renderToolBar()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2, 2));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    if(ImGui::Begin("ToolBar", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus))
    {
        if (ImGui::ImageButton("reset", resetImage_id, ImVec2(toolBarImage_height, toolBarImage_width)))
        {
            gui->m_cpu.startExec();
        }
        ImGui::SameLine();
        if (gui->m_clock.getStatus())
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
            gui->m_clock.singleCycle();
        }
    }
    ImGui::End();
    ImGui::PopStyleVar(3);
}

void renderFilesOpened()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2, 2));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    if (ImGui::Begin("FilesOpened", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus))
    {
        if (gui->sourceFileNames.size() <= 1)
        {
            editor.SetText(noOpenedFileText);
        }
        for (uint64_t i = 1; i < gui->sourceFileNames.size(); i++) // start with 1, first one is the program file
        {
            bool currentBreakpoint = false;
            if (i == gui->currentPosition.fileID)
            {
                currentBreakpoint = true;
            }
            if (ImGui::Button(getFnWithoutPath(gui->sourceFileNames[i]).c_str()) || (currentBreakpoint && gui->currentPosition.address != gui->lastPosition))
            {
                gui->lastPosition = gui->currentPosition.address;
                if (openedFileName != gui->sourceFileNames[i])
                {
                    openedFileName = gui->sourceFileNames[i];
                    std::ifstream file(openedFileName, std::ios::in);
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
                        gui->displayError("Unable to open file: %s", openedFileName.c_str());
                    }
                }
                if (currentBreakpoint)
                {
                    TextEditor::Breakpoints brps;
                    brps.insert(gui->currentPosition.line);
                    editor.SetBreakpoints(brps);
                }
            }
            ImGui::SameLine();
        }
    }
    ImGui::End();
    ImGui::PopStyleVar(3);
}

void renderEditor()
{
    if (fileOpenInput)
    {
        // open Dialog Simple
        IGFD::FileDialogConfig config;
        config.flags = ImGuiFileDialogFlags_DisableCreateDirectoryButton | ImGuiFileDialogFlags_NoDialog;
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
        if (ImGuiFileDialog::Instance()->Display("0", ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                if (fileOpenInputType == projectFile)
                {
                    gui->NewProjectOpened = true;
                    gui->projectFileName = filePathName;
                    for (int i = (filePathName.length() - 1); i > 0; i--)
                    {
                        char c = filePathName.at(i);
                        if (c == '/' || c == '\\')
                        {
                            gui->projectPath = filePathName.substr(0, i);
                            break;
                        }
                    }
                }
                else
                {
                    isRunning = false;
                    gui->m_clock.setStatus(isRunning);
                    gui->m_cpu.loadProgramFromFile(filePathName);
                    gui->m_cpu.startExec();
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
        editor.Render("");
    }
}

void renderConsole()
{
    ImGui::Text("Terminal");
    ImGui::Separator();
    ImGui::BeginChild("#Text");
    ImGui::TextWrapped((*gui->ConsoleText).c_str());
    ImGui::EndChild();
}

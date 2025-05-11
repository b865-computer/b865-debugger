#include "gui.h"
#include "TextEditor.h"
#include "ImGuiFileDialog.h"
#include "FileExplorer.h"
#include "FileTab.h"
#include <imgui_stdlib.h>
#include <CdbgExpr.h>

extern FileTabManager fileTabManager;
extern FileExplorer explorer;
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
extern GLuint emulatorImage_id;

void renderSideBar()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);

    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 0.5f));

    if (ImGui::ImageButton("explorer_button", explorerImage_id,
        ImVec2(sideBarImage_height, sideBarImage_width)))
    {
        gui->sideBarToolType = GUI::ToolType::TOOL_EXPLORER;
    }
    if (ImGui::ImageButton("emulator_button", emulatorImage_id,
        ImVec2(sideBarImage_height, sideBarImage_width)))
    {
        gui->sideBarToolType = GUI::ToolType::TOOL_EMUALTOR;
    }
    if (ImGui::ImageButton("debugger_button", debuggerImage_id,
        ImVec2(sideBarImage_height, sideBarImage_width)))
    {
        gui->sideBarToolType = GUI::ToolType::TOOL_DEBUGGER;
    }

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
}

std::vector<std::string> exprs;
size_t currentEditingExpr = 0;
bool editingExpr = false;

void renderCValue(const CdbgExpr::SymbolDescriptor& val)
{
    if (val.cType.size() < 1)
    {
        return;
    }
    std::ostringstream result;
    auto cType = val.cType;
    if (cType[0] == CdbgExpr::CType::Type::POINTER)
    {
        if (cType.size() < 2)
        {
            ImGui::Text("*<unknown type>");
            return;
        }

        if (cType[1] == CdbgExpr::CType::Type::CHAR)
        {
            if (!val.getValue())
            {
                ImGui::Text("0x0");
                return;
            }
            else
            {
                result << "0x" << std::hex << val.getValue();
            }
            result << " \"";
            uint64_t addr = val.getValue();
            char ch;
            while ((ch = static_cast<char>(val.data->getByte(addr++))) != '\0')
            {
                result << ch;
            }
            result << "\"";
            ImGui::Text(result.str().c_str());
        }
        else
        {
            result << val.typeOf();
            result << "0x" << std::hex << val.getValue();
            ImGui::Text(result.str().c_str());
        }
    }
    else if (cType[0] == CdbgExpr::CType::Type::ARRAY)
    {
        ImGui::SameLine();
        ImGui::Text("= []");
        for (size_t i = 0; i < cType[0].size; i++)
        {
            if(ImGui::TreeNode((std::string("[") + std::to_string(i) + "]").c_str()))
            {
                renderCValue(val.dereference(i));
                ImGui::TreePop();
            }
            else
            {
                ImGui::SameLine();
                ImGui::Text("= %s", val.dereference(i).toString().c_str());
            }
        }
    }
    else if (cType[0] == CdbgExpr::CType::Type::STRUCT)
    {
        ImGui::SameLine();
        ImGui::Text("= {}");
        for (auto& member : val.members)
        {
            if(ImGui::TreeNode(member.first.c_str()))
            {
                renderCValue(member.second.symbol);
                ImGui::TreePop();
            }
            else
            {
                ImGui::SameLine();
                ImGui::Text("= %s", val.toString().c_str());
            }
            ImGui::NewLine();
        }
        return;
    }
    else
    {
        ImGui::Text("%s%s", val.typeOf().c_str(), std::visit([](auto && value) 
            { return std::to_string(value); }, val.getRealValue()).c_str());
    }

    return;
}

void renderExpressions()
{
    for (size_t i = 0; i < exprs.size(); i++)
    {
        ImGui::PushID(static_cast<int>(i));

        if (editingExpr && currentEditingExpr == i)
        {
            ImGui::InputText("##edit_expr", &exprs[i]);
        }
        else
        {
            // Transparent header when idle, visible on hover/active
            ImGui::PushStyleColor(ImGuiCol_Header,        ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2f, 0.5f, 0.9f, 0.3f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive,  ImVec4(0.2f, 0.5f, 0.9f, 0.6f));

            CdbgExpr::SymbolDescriptor result;

            try
            {
                CdbgExpr::Expression e(exprs[i], &gui->m_emulator);
                result = e.eval(true);
            }
            catch (const std::exception& ex)
            {
            }

            if (ImGui::TreeNodeEx(exprs[i].c_str(), ImGuiTreeNodeFlags_OpenOnArrow))
            {
                renderCValue(result);
                ImGui::TreePop();
            }
            else
            {
                ImGui::SameLine();
                ImGui::Text("= %s", result.toString().c_str());
            }

            ImGui::PopStyleColor(3);
        }

        ImGui::PopID();
    }

    // Button for adding new expressions
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.5f, 0.9f, 0.3f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.2f, 0.5f, 0.9f, 0.6f));

    if (ImGui::Button("+"))
    {
        exprs.push_back("");
        editingExpr = true;
        currentEditingExpr = exprs.size() - 1;
    }

    ImGui::PopStyleColor(3);
}

void renderSideTool()
{
    if (ImGui::Begin("SideTool", nullptr,
        ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings))
    {
        if (gui->sideBarToolType == GUI::ToolType::TOOL_EXPLORER)
        {
            explorer.render(true);
        }
        else if (gui->sideBarToolType == GUI::ToolType::TOOL_EMUALTOR)
        {
            ImGui::Text("Frequency: %liHz", gui->m_frequencyHZ);
            if (showRealFrequency)
            {
                ImGui::Text("Real freq. %.0fHz",
                    ((double)gui->m_clock.getCycles() /
                    ((double)gui->m_clock.getRunTime_ns().count() / 1e9)));
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

            for (int i = 0; i < gui->m_emulator.m_pheriphCount; i++)
            {
                if (ImGui::CollapsingHeader(gui->m_emulator.m_pheripherials[0]->m_name.c_str()))
                {
                    for (size_t i = 0; i < gui->m_emulator.m_pheripherials[0]->m_regNames.size(); i++)
                    {
                        ImGui::Text(" %s: %i",
                                    gui->m_emulator.m_pheripherials[0]->m_regNames[i].c_str(),
                                    gui->m_emulator.m_pheripherials[0]->regs[i]);
                    }
                }
            }

            if (ImGui::CollapsingHeader("Symbols, variables"))
            {
                for (const auto& symbol : 
                    gui->m_emulator.m_debuggerData.globalScope)
                {
                    ImGui::Text(" %s: 0x%04lX", symbol.name.c_str(), symbol.value);
                }
                for (const auto& pair : gui->m_emulator.m_debuggerData.fileScope)
                {
                    if (ImGui::CollapsingHeader(("File: " + pair.first).c_str()))
                    {
                        for (const auto& symbol : pair.second)
                        {
                            ImGui::Text(" %s: 0x%04lX", symbol.name.c_str(), symbol.value);
                        }
                    }
                }
                for (const auto& pair : gui->m_emulator.m_debuggerData.funcScope)
                {
                    if (ImGui::CollapsingHeader(("Func" + pair.first).c_str()))
                    {
                        for (const auto& symbol : pair.second)
                        {
                            ImGui::Text(" %s: 0x%04lX", symbol.second.name.c_str(), symbol.second.value);
                        }
                    }
                }
            }
        }
        else if (gui->sideBarToolType == GUI::ToolType::TOOL_DEBUGGER)
        {   
            renderExpressions();
        }
    }
    ImGui::End();
}

void renderToolBar()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2, 2));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    if (ImGui::Begin("ToolBar", nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus))
    {
        if (ImGui::ImageButton("reset", resetImage_id,
            ImVec2(toolBarImage_height, toolBarImage_width)))
        {
            gui->m_cpu.startExec();
        }
        ImGui::SameLine();
        if (gui->m_clock.getStatus())
        {
            if (ImGui::ImageButton("stop", stopImage_id,
                ImVec2(toolBarImage_height, toolBarImage_width)))
            {
                isRunning = false;
            }
        }
        else
        {
            if (ImGui::ImageButton("start", startImage_id,
                ImVec2(toolBarImage_height, toolBarImage_width)))
            {
                isRunning = true;
            }
        }
        ImGui::SameLine();
        if (ImGui::ImageButton("tick", tickImage_id,
            ImVec2(toolBarImage_height, toolBarImage_width)))
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
    if (ImGui::Begin("FilesOpened", nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus))
    {
        fileTabManager.renderFileTabs();
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
        config.flags = 
            ImGuiFileDialogFlags_DisableCreateDirectoryButton | ImGuiFileDialogFlags_NoDialog;
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
        if (ImGuiFileDialog::Instance()->Display("0",
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string path = getPath(filePathName);
                explorer.setDirectory(path);
                if (fileOpenInputType == projectFile)
                {
                    gui->NewProjectOpened = true;
                    gui->projectFileName = filePathName;
                    gui->projectPath = path;
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
        if (fileTabManager.changedCurrentTab())
        {
            FileTab *tab = fileTabManager.getCurrentFileTab();
            if (tab == nullptr)
            {
                editor.SetText(noOpenedFileText);
            }
            else
            {
                editor.SetText(tab->getContent());
            }
        }
        TextEditor::Breakpoints breakpoints = {(int)gui->currentPosition.line};
        editor.SetBreakpoints(breakpoints);
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

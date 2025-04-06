# === ImGui ===
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

add_library(imgui
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# === ImGuiColorTextEdit ===
add_library(ImGuiColorTextEdit
    ImGuiColorTextEdit/TextEditor.cpp
)
target_include_directories(ImGuiColorTextEdit PUBLIC ImGuiColorTextEdit)
target_link_libraries(ImGuiColorTextEdit PUBLIC imgui)

# === ImGuiFileDialog ===
add_library(ImGuiFileDialog
    ImGuiFileDialog/ImGuiFileDialog.cpp
)
target_include_directories(ImGuiFileDialog PUBLIC ImGuiFileDialog)
target_link_libraries(ImGuiFileDialog PUBLIC imgui)

# === libintelhex ===
add_library(libintelhex ${CMAKE_CURRENT_SOURCE_DIR}/libintelhex/src/intelhex.cc)
target_include_directories(libintelhex PUBLIC libintelhex/include)
CC = g++

LIB_IMGUI = lib/imgui
LIB_IMGUI_SRC = $(wildcard $(LIB_IMGUI)/imgui*.cpp)
LIB_IMGUI_OBJ = $(LIB_IMGUI_SRC:.cpp=.o) $(LIB_IMGUI)/backends/imgui_impl_glfw.o $(LIB_IMGUI)/backends/imgui_impl_opengl3.o

LIB_IMGUIFILEDIALOG = lib/ImGuiFileDialog
LIB_IMGUIFILEDIALOG_SRC = $(wildcard $(LIB_IMGUIFILEDIALOG)/ImGuiFileDialog*.cpp)
LIB_IMGUIFILEDIALOG_OBJ = $(LIB_IMGUIFILEDIALOG_SRC:.cpp=.o)

LIB_IMGUICOLOTEXTEDIT = lib/ImGuiColorTextEdit
LIB_IMGUICOLOTEXTEDIT_SRC = $(LIB_IMGUICOLOTEXTEDIT)/TextEditor.cpp
LIB_IMGUICOLOTEXTEDIT_OBJ = $(LIB_IMGUICOLOTEXTEDIT_SRC:.cpp=.o)

INCLUDEPATH = -I$(LIB_IMGUI) -I$(LIB_IMGUI)/backends -I$(LIB_IMGUIFILEDIALOG) -I$(LIB_IMGUICOLOTEXTEDIT) -Ilib/stb_image

CFLAGS = -g -fdiagnostics-color=always $(INCLUDEPATH)
CFLIBFLAGS = $(CFLAGS)
CFLAGS += $(INCLUDEPATH)
LDFLAGS = -lglfw3 -lopengl32

SRC = $(wildcard src/*.cpp)
OBJ = $(patsubst src/%.cpp,build/%.o,$(SRC))

.PHONY: all

all: build run 

run: emulator.exe
	./emulator.exe

build: libs emulator.exe

emulator.exe: libs $(OBJ)
	$(CC) -o emulator.exe $(OBJ) $(LIB_IMGUI_OBJ) $(LIB_IMGUIFILEDIALOG_OBJ) $(LIB_IMGUICOLOTEXTEDIT_OBJ) $(LDFLAGS)

build/%.o: src/%.cpp $(wildcard src/*.h)
	$(CC) $(CFLAGS) -o $@ -c $<

libs: imgui ImGuiFileDialog ImGuiColorTextEdit

imgui: $(LIB_IMGUI_OBJ)

ImGuiFileDialog: $(LIB_IMGUIFILEDIALOG_OBJ)

ImGuiColorTextEdit: $(LIB_IMGUICOLOTEXTEDIT_OBJ)

lib/%.o: lib/%.cpp $(wildcard lib/**/.h)
	$(CC) $(CFLIBFLAGS) -o $@ -c $<
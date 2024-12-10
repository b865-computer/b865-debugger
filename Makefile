CC = g++
INCLUDEPATH = -Ilib/imgui -Ilib/imgui/backends -Ilib/ImGuiFileDialog -Ilib/stb_image
CFLAGS = -g -fdiagnostics-color=always $(INCLUDEPATH)
LDFLAGS = -lglfw3 -lopengl32
LIB_SRC += $(wildcard lib/imgui/imgui*.cpp) $(wildcard lib/ImGuiFileDialog/ImGuiFileDialog*.cpp)
OBJ = build/main.o build/CPU.o build/MEM.o build/Pheriph.o build/gui.o\
	build/Emulator.o build/Clock.o build/gui_main.o
OBJ += lib/imgui/backends/imgui_impl_glfw.o lib/imgui/backends/imgui_impl_opengl3.o $(LIB_SRC:.cpp=.o)

.PHONY: all

all: build run 

run: emulator.exe
	./emulator.exe

build: emulator.exe

emulator.exe: $(OBJ)
	$(CC) -o emulator.exe $^ $(LDFLAGS)

build/%.o: src/%.cpp $(wildcard src/*.h)
	$(CC) $(CFLAGS) -o $@ -c $<

lib/%.o: lib/%.cpp $(wildcard lib/**/.h)
	$(CC) $(CFLAGS) -o $@ -c $<
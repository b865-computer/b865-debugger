CC = g++
INCLUDEPATH = -Ilib/glad/include -Ilib/imgui -Ilib/imgui/backends
CFLAGS = -g -fdiagnostics-color=always $(INCLUDEPATH)
LDFLAGS = -lglfw3 -lopengl32 -lgdi32 -lshell32
IMGUI_SRC  += $(wildcard lib/imgui/imgui*.cpp)
OBJ = build/main.o build/CPU.o build/MEM.o build/Pheriph.o build/Pheriph_IO.o build/gui.o\
	build/Emulator.o build/Clock.o build/gui_main.o
OBJ += lib/imgui/backends/imgui_impl_glfw.o lib/imgui/backends/imgui_impl_opengl3.o $(IMGUI_SRC:.cpp=.o)

.PHONY: all

all: build run 

run: emulator.exe
	./emulator.exe program.out

build: emulator.exe

emulator.exe: $(OBJ)
	$(CC) -o emulator.exe $^ $(LDFLAGS)

build/%.o: src/%.cpp $(wildcard src/*.h)
	$(CC) $(CFLAGS) -o $@ -c $<

lib/%.o: lib/%.cpp $(wildcard lib/**/.h)
	$(CC) $(CFLAGS) -o $@ -c $<
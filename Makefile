CC = g++
CFLAGS = -g -fdiagnostics-color=always

SRC  = $(wildcard src/*.cpp) $(wildcard src/**/*.cpp)
OBJ = build/main.o build/CPU.o build/MEM.o build/Pheriph.o build/Pheriph_IO.o
BUILD = build

.PHONY: all

all: build run 

run:
	./emulator.exe program.out

build: $(OBJ)
	$(CC) -o emulator $^ $(CFLAGS)

build/%.o: src/%.cpp $(wildcard src/*.h)
	$(CC) -o $@ -c $< $(CFLAGS)
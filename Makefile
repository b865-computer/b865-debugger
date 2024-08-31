CC = g++
CFLAGS = -g -fdiagnostics-color=always

SRC  = $(wildcard src/*.cpp) $(wildcard src/**/*.cpp)
OBJ = build/main.o build/CPU.o build/MEM.o build/Pheriph.o
BUILD = build

.PHONY: all

all: build clean run

run:

build: $(OBJ)
	$(CC) -o main.exe $^ $(CFLAGS)

build/%.o: src/%.cpp
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rmdir /s /q build
	mkdir build
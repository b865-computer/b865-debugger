CC = g++
CFLAGS = -g -fdiagnostics-color=always

SRC  = $(wildcard src/*.cpp) $(wildcard src/**/*.cpp)
OBJ = build/main.o build/CPU.o build/MEM.o build/Pheriph.o
BUILD = build

.PHONY: all

all: build test run 

run:
	./emulator

build: $(OBJ)
	$(CC) -o emulator $^ $(CFLAGS)

build/%.o: src/%.cpp
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm ./build/*

buildTest: $(SRC) test/emulator_test.cpp
	cd test && cmake --build .

test: buildTest
	./test/emulator_test
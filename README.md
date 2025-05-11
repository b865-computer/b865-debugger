# b865 Debugger

This project is a debugger/emualtor tool for my BreadBoard Computer (b865).

## Features

- Step-by-step execution of instructions.
- Memory inspection and modification.
- Register state visualization.

## Requirements

- C/C++ compiler (e.g., GCC or Clang).
- CMake

## Installation

1. Clone the repository:

    ```bash
    git clone https://github.com/Rbel12b/b865-debugger.git
    cd b865-debugger
    ```

2. Build the project:

    ```bash
    mkdir build
    cd build
    cmake ..
    cmkae --build .
    cd ..
    ```

3. Run the debugger:

    ```bash
    ./build/src/b865-cli /path/to/project.b865
    ./build/src/b865-gui # the gui app only works in the projects root directory because of the resources
    ```

## Usage

create a [b865-project](https://github.com/Rbel12b/b865-project), run ```make``` to generate .b865 and .bin files, then open the .b865 file in the debugger

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

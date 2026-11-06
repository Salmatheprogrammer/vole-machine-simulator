cmake_minimum_required(VERSION 4.1)
project(vole_machine_simulator)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(vole_machine_simulator
        main.cpp
        CPU.cpp
        CPU.h
        Memory.h
        Memory.cpp
        RegisterFile.cpp
        RegisterFile.h
        Machine.cpp
        Machine.h)

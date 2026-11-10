cmake_minimum_required(VERSION 3.16)
project(vole_machine_simulator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# One program: main.cpp opens the GUI, or the console menu with --console
add_executable(vole_machine_simulator
        main.cpp
        MainWindow.cpp
        MainWindow.h
        SimulatorUI.cpp
        SimulatorUI.h
        CPU.cpp
        CPU.h
        Machine.cpp
        Machine.h
        Memory.cpp
        Memory.h
        RegisterFile.cpp
        RegisterFile.h
)

target_link_libraries(vole_machine_simulator PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_compile_definitions(vole_machine_simulator PRIVATE QT_NO_KEYWORDS)
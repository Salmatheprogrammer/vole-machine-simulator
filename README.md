# Vole Machine Simulator

## Project Description
The Vole Machine Simulator is a C++ application that simulates some simple operations and data transfers in a computer system.
The user can interact with the simulator through a GUI which provides a graphical interface to load programs, execute instructions, modify registers and memory and display the results on the screen.

## Instructor
Dr. Mohammad El-Ramly

## Features
- Load Program from .txt file.
- Enter instructions directly in hexadecimal.
- Step-by-step execution.
- Run All.
- Edit memory cells and registers by double-clicking them.
- Light Mode and Dark Mode.
- Highlight invalid instructions.

## Project Structure
- Machine: responsible for coordinating the simulation components and managing the basic operations.
- CPU: responsible for executing instructions.
- Memory: responsible for storing data and instructions.
- RegisterFile: responsible for managing the 16 general-purpose registers.
- SimulatorUI: responsible for running the menu, handling user interactions.
- GUI: responsible for providing a graphical interface for users to interact with the simulator.

## How to Compile and Run
1. Open the CMakeLists.txt file using Qt Creator
2. Select the Desktop Qt 6.12.0 MinGW 64-bit kit.
3. Click Configure Project.
4. Build the project.
5. Run the simulator.


## How to Use
1. Run the simulator.
2. Load a program from a .txt file or enter instructions directly in hexadecimal.
3. Set an even start address in hexadecimal.
4. Click Load Program or Load Instructions.
5. Use Step to execute one instruction at a time, or Run All to execute the whole program.
6. Use Reset All to reset the simulator.
7. Double-click a memory cell or register to edit its hexadecimal value.
8. Switch between Light Mode and Dark Mode using the theme button.

## Team Members
- Rawan Osama
- Sama Tarek
- Salma Ayman
- Malak Abdelhamid
#pragma once

#include <iostream>
#include <string>

#include "Machine.h"

class SimulatorUI {
private:

public:
    SimulatorUI();

    void displayMenu();
    void loadProgram(Machine& machine);
    void run(Machine& machine);
    void step(Machine& machine);
    void reset(Machine& machine);
    void displayMemory(Machine& machine);
    void displayRegister(Machine& machine);
    void modifyMemory(Machine& machine);
    void modifyRegister(Machine& machine);
};

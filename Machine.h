#pragma once

#include <string>
#include <cstdint>

#include "Memory.h"
#include "CPU.h"
#include "RegisterFile.h"


class Machine {
private:
    Memory memory;
    RegisterFile registers;
    CPU cpu;

    bool isValidHexaInstruction
         (std::string instruction);
public:
    Machine();

    bool loadProgram( string filename, string startAddrHexa);

    void setMemoryCell(uint8_t addr, uint8_t val);
    void setRegister(uint8_t regNum, uint8_t val);

    bool step();
    bool run();
    bool isHalted() const;

    void resetAll();

    uint8_t getPC()const;
    std::string getIR()const;

    const Memory& getMemory()const;
    const RegisterFile& getRegisters()const;


};
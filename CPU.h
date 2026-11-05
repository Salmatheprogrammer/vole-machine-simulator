#pragma once
#include <cstdint>
#include <bits/stdc++.h>
#include "Memory.h"
#include "RegisterFile.h"
using namespace std;

class CPU {
private:
    uint8_t pc;
    string ir, opcode, operand;
    bool halted;
    bool valid;
    Memory& memory;
    RegisterFile& registers;

    void fetch();
    bool isValid();

    void execute();
    uint8_t floatAdd(uint8_t a, uint8_t b);
    string byteToHex (uint8_t byteFromMemory);
    double BrookshearsFloatingPointRes (string AorB);
    string bytesTobitPatterns(uint8_t inBytes);
    uint8_t bitsToByte(string bits);
    string decimalToBrookshear(double number);
public:
    CPU(Memory& mem, RegisterFile& regs);

    void reset(uint8_t startPC = 0);
    bool step();
    bool isHalted() const;
    uint8_t getPC() const;
    string getIR() const;
};
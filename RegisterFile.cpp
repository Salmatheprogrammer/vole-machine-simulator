
#include "RegisterFile.h"
uint8_t RegisterFile::get(uint8_t regNum) const {
    return registers[regNum];
}
void RegisterFile::set(uint8_t regNum, uint8_t value) {
    registers[regNum] = value;
}
void RegisterFile::clear() {
    registers.fill(0);
}
#include "Memory.h"
Memory::Memory() {
    cells.fill(0);
    screenBuffer.clear();
}
void Memory::write(uint8_t address, uint8_t value) {
    if (address == 0xFF) {
        screenBuffer += static_cast<char>(value);
    } else {
        cells[address] = value;
    }
}

uint8_t Memory::read(uint8_t address) const {
    return cells[address];
}

void Memory::clear() {
    cells.fill(0);
    screenBuffer.clear();
}

std::string Memory::getScreenOutput() const {
    return screenBuffer;
}

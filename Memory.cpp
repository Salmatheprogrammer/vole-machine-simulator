
#include "Memory.h"
#include <sstream>
#include <iomanip>

Memory::Memory() {
    cells.fill(0);
    screenBuffer.clear();
}

void Memory::write(uint8_t address, uint8_t value) {

    if (address == 0xFF) {

        std::ostringstream output;

        output << std::uppercase << std::hex
               << std::setw(2) << std::setfill('0')
               << static_cast<int>(value);

        screenBuffer += output.str();
        screenBuffer += " ";

    }
    else {
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

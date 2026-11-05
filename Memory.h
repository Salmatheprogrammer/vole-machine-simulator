
#ifndef VOLEMACHINE_MEMORY_H
#define VOLEMACHINE_MEMORY_H
#include <array>
#include <cstdint>
#include <string>

class Memory {
private:
    std::array<uint8_t, 256> cells{};
    std::string screenBuffer;
public:
    void write(uint8_t address, uint8_t value);
    uint8_t read(uint8_t address) const;
    void clear();
    std::string getScreenOutput() const;
    Memory();

};


#endif //VOLEMACHINE_MEMORY_H



#ifndef VOLEMACHINE_REGISTERFILE_H
#define VOLEMACHINE_REGISTERFILE_H
#include <array>
#include <cstdint>
#include <string>

class RegisterFile {
private:
    std::array<uint8_t, 16> registers{};
public:
    uint8_t get(uint8_t regNum) const;
    void set(uint8_t regNum, uint8_t value);
    void clear();
};


#endif //VOLEMACHINE_REGISTERFILE_H

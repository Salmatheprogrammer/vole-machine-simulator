#include <iostream>
#include "CPU.h"
using namespace std;

CPU::CPU(Memory& mem, RegisterFile& regs)
    : pc(0), ir("0000"), halted(false), memory(mem), registers(regs) {}



//accessing the program counter
uint8_t CPU :: getPC() const {
    return pc;
}
//accessing the instruction register
string CPU :: getIR() const {
    return ir;
}
//resetting
void CPU :: reset (uint8_t startPC) {
    pc = startPC;
    ir = "0000";
    halted = false;
}
//FETCH
/*converting memory bytes into hexadecimal strings so
we can read them exactly like the program sample,
and we can then fetch, decode, validate and execute*/
string CPU:: byteToHex(uint8_t byteFromMemory) {
    ostringstream ss;
    ss << hex << uppercase << setfill('0') << setw(2)
       << static_cast<int>(byteFromMemory);
    return ss.str();
}

double CPU:: BrookshearsFloatingPointRes (string AorB) {
    int sign = stoi(AorB.substr(0, 1));
    int exponent = stoi(AorB.substr(1, 3), nullptr, 2);
    int mantissa = stoi(AorB.substr(4, 4), nullptr, 2);
    double fraction = mantissa / 16.0;
    double res = pow(-1, sign) * fraction * pow(2, exponent - 4);
    return res;
}
string CPU:: bytesTobitPatterns(uint8_t inBytes) {
    string bits = bitset<8>(inBytes).to_string();
    return bits;
}
uint8_t CPU:: bitsToByte(string bits) {
    return static_cast<uint8_t>(stoi(bits,nullptr,2));
}
string CPU::decimalToBrookshear(double number) {
    // if the number is 0 write it in its 8-bit form
    if (number == 0)
        return "00000000";
    // sign
    int sign = 0;
    if (number < 0) {
        sign = 1;
        number = -number;
    }
    // E
    int exponent = 0;
    while (number >= 1.0) {
        number /= 2.0;
        exponent++;
    }
    while (number < 0.5) {
        number *= 2.0;
        exponent--;
    }
    //whole eponent
    int wholeExponent = exponent + 4;
    if (wholeExponent < 0 || wholeExponent > 7)
        return "";
    // Get 4 mantissa bits
    int mantissa = static_cast<int>(number * 16);
    string exponentBits = "";
    string mantissaBits = "";
    // Converting exponent to 3 bits
    for (int i = 0; i < 3; i++) {
        exponentBits = char('0' + wholeExponent % 2) + exponentBits;
        wholeExponent /= 2;
    }
    // Converting mantissa to 4 bits
    for (int i = 0; i < 4; i++) {
        mantissaBits = char('0' + mantissa % 2) + mantissaBits;
        mantissa /= 2;
    }
    return to_string(sign) + exponentBits + mantissaBits;
}


void CPU::fetch() {

    ir= byteToHex(memory.read (pc) ) +
        byteToHex( memory.read (pc+1) );
    opcode = ir[0];
    operand = ir.substr(1,3);
    pc+=2;

}
//isValid
bool CPU::isValid() {
    valid = false;
    //checking instruction length of 4
    if (ir.length() != 4)
        return false;
    //checking instruction is all hexadecimal for example: 2M07 would be considered right if that loop isn't added
    for (char c : ir) {
        if (!((c >= '0' && c <= '9') ||
              (c >= 'A' && c <= 'F'))) {
            return false;
              }
    }
    //checking instruction opcode and operands + thier special cases
    switch (opcode[0]) {
        case '1':
        case '2':
        case '3':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case 'B':
            valid=true;
        break;

        case '4':
            if (operand[0]=='0') {
                valid=true;
            }
            break;
        case 'A':
            if (operand[1]=='0') {
                valid=true;
            }
            break;
        case 'C':
            if (operand=="000") {
                valid=true;
            }
            break;

        default:
            valid=false;
    }
    if (valid) {
        return true;
    }
    else {
        return false;
    }
}

//EXECUTE
void CPU::execute() {
    int R,XY,S,T, result;
    R = stoi(operand.substr(0, 1), nullptr, 16);
    XY = stoi(operand.substr(1, 2), nullptr, 16);
    S = stoi(operand.substr(1, 1), nullptr, 16);
    T = stoi(operand.substr(2, 1), nullptr, 16);

    switch (opcode[0]) {
        case '1':
           registers.set(R, memory.read(XY));
            break;
        case '2':
            registers.set(R, XY);
            break;
        case '3':
            memory.write (XY, registers.get(R));
            break;
        case '4':
            R= operand[1];
            S= operand[2];
            registers.set(S, registers.get(R));
            break;
        case '5': {
            //using signed int of 8 bits stores the value of its sign without the need to convert for the sign
            int8_t AinTwosComp = registers.get(S);
            int8_t BinTwosComp = registers.get(T);
            int result5 = AinTwosComp + BinTwosComp;
            registers.set(R, result5);
            break;
        }
        case '6': {
            string A_binary = bytesTobitPatterns(registers.get(S));
            string B_binary= bytesTobitPatterns(registers.get(T));
            double A= BrookshearsFloatingPointRes(A_binary);
            double B= BrookshearsFloatingPointRes(B_binary);
            double result6 = A+B;
            string revrsedProcessBrookshears = decimalToBrookshear(result6);
            if (revrsedProcessBrookshears != "") {
                uint8_t finalResult = bitsToByte(revrsedProcessBrookshears);
                registers.set(R, finalResult);
            }
            break;
        }
        case '7': {
            uint8_t A = registers.get(S);
            uint8_t B = registers.get(T);
            auto result7 = A|B;
            registers.set(R, result7);
            break;
        }
        case '8': {
            uint8_t A = registers.get(S);
            uint8_t B = registers.get(T);
            auto result8 = A&B;
            registers.set(R, result8);
            break;
        }
        case '9': {
            uint8_t A = registers.get(S);
            uint8_t B = registers.get(T);
            auto result9 = A^B;
            registers.set(R, result9);
            break;
        }
        case 'A': {
            int X = stoi(operand.substr(2,1));
            auto resultA = registers.get(R);
            registers.set(R, resultA);
            for (int i=0; i<X;i++) {
              resultA =(resultA >> 1)|(resultA<<7);
            }
            registers.set(R, resultA);
            break;
        }
        case 'B': {
            if (registers.get(R)==registers.get(0)) {
                pc=XY;
            }
            break;
        }
        case 'C':
            halted = true;
            break;
    }
}

//is Halted
bool CPU::isHalted() const {
    if (halted) {
        return true;
    }
    else {
        return false;
    }
}

//STEP
bool CPU::step() {
    if (isHalted()) {
        return false;
    }
    fetch();
    if (!isValid()) {
        return false;
    }
    execute();
    return true;
}



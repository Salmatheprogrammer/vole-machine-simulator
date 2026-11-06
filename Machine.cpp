#include "Machine.h"
#include "CPU.h"
#include <fstream>
#include <cctype>


using namespace std;

//to validate the instruction
bool Machine::isValidHexaInstruction(string instruction) {
    if (instruction.length() != 4) {
        return false;
    }
    for (unsigned char c : instruction) {
        if (!isxdigit(c)) {
            return false;
        }
    }
    return true;
}
//to load a program
bool Machine::loadProgram(string filename, string startAddrHexa) {

    ifstream programFile(filename);
    if (!programFile) {  //validating of opening the file
        cout<< "Error: Invalid program file name\n";
        return false;
    }
    //validate address
    if (startAddrHexa.length() > 2){ return false;}
    for (unsigned char c : startAddrHexa) {
        if (!isxdigit(c)) {
            cout << "Invalid address\n";
            programFile.close();
            return false;
        }
    }
    //convert the hexa address to unsigned char((uint8_t))
    uint8_t startAddr=(uint8_t)stoi(startAddrHexa,nullptr,16);

    //validating if the address is even
    if (startAddr % 2 != 0) {
        cout << "Error: Start address must be an even number (00, 02, 04,...)\n";
        return false;
    }


    // variables in need for them in the coming while loop
    unsigned int currentAddr = startAddr;
    string line;
    Memory oldMemory=memory;

    // the loop that reads lines and stores the instructions in memory
    while (getline(programFile,line)) {

        // to skip spaces and comments
        size_t startOfInstruction = line.find_first_not_of(" \t");
        if (startOfInstruction ==string:: npos) {continue;}
        line=line.substr(startOfInstruction);
        if ( line.empty() || line[0] == ';') {
            continue;
        }

        // to not store data out of bound
        if (currentAddr + 1 > 255) {
            cout << "there isn't enough memory space\n";
            programFile.close();
            memory = oldMemory;
            return false;
        }

        string instructionInHexa = line.substr(0, 4);
        //to validate the hexa instruction
        if (!isValidHexaInstruction(instructionInHexa)) {
            cout << "Invalid hexadecimal instruction\n";
            programFile.close();
            memory = oldMemory;
            return false;
        }
        // to split and store instruction
        string byte1Str = instructionInHexa.substr(0, 2);
        string byte2Str = instructionInHexa.substr(2, 2);

        // converting instructions into binary
        uint8_t byte1 = (uint8_t) (stoi(byte1Str,nullptr,16));
        uint8_t byte2 = (uint8_t) (stoi(byte2Str,nullptr,16));


        //storing instructions in memory
        memory.write(( uint8_t)currentAddr , byte1);
        memory.write(( uint8_t)(currentAddr+1) ,byte2);
        currentAddr += 2;
    }
    programFile.close();
    cpu.reset(startAddr); // set pc with the starting address
    return true;
}

//constructor
Machine::Machine():cpu(memory,registers){}

// write in memory or registers
void Machine::setMemoryCell(uint8_t addr, uint8_t val) {
    memory.write(addr, val);
}
void Machine::setRegister(uint8_t regNum, uint8_t val) {
    registers.set(regNum, val);
}

// to run the program
bool Machine::step() {
    return cpu.step();
}
bool Machine::run() {
    if (cpu.isHalted()){cout<<"isHalted bool is true\n"; return false;} // to show if there is error while testing *****************###***
    while (!cpu.isHalted()) {
        if (!cpu.step()) {
            return false;
        }
    }
    return true;
}
bool Machine::isHalted () const{
    return cpu.isHalted();
}

// reset
void Machine::resetAll() {
    registers.clear();
    memory.clear();
    cpu.reset();
}

// to read pc or ir from CPU class
uint8_t Machine::getPC()const {
    return cpu.getPC();
}
string Machine::getIR()const {
    return cpu.getIR();
}


//to read cell from memory or registers by simulatorUI class
const Memory& Machine::getMemory()const { return memory;}

const RegisterFile& Machine::getRegisters()const{ return registers;}


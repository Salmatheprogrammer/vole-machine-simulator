#include "SimulatorUI.h"
#include <iostream> 
#include <string>
#include <iomanip>

using namespace std;

SimulatorUI::SimulatorUI() {}

void SimulatorUI::displayMenu(){

    cout << "1. Load Program" << endl;
    cout << "2. Run" << endl;
    cout << "3. Step" << endl;
    cout << "4. Reset" << endl;
    cout << "5. Display Memory" << endl;
    cout << "6. Display Register" << endl;
    cout << "7. Modify Memory" << endl;
    cout << "8. Modify Register" <<endl;
    cout << "9. printState" <<endl;
    cout << "10. printScreen" <<endl;
    cout << "11. Exit" <<endl;
    
}

void SimulatorUI:: loadProgram(Machine&machine){
string filename;
string startAddrHexa;
bool x;

    cout <<"enter file name:"<<endl;
    cin >> filename;

    cout << "enter startingAddress:" <<endl;
    cin >> startAddrHexa;

    x = machine.loadProgram(filename, startAddrHexa);
    
    if(x){
        cout << "the program is loaded successfully" <<endl;
    }
    else{
        cout << "fail to load program" <<endl;
    }
    
}
void SimulatorUI::run(Machine&machine){

   bool x = machine.run();
    
    if(x){
        cout << "the program is run successfully" <<endl;
    }
    else{
        cout << "fail to run program" <<endl;
    }
}

void SimulatorUI::step(Machine&machine){
   
    bool x = machine.step();
    
    if(x){
        cout << "the result of the instruction" <<endl;
    }
    else{
        cout << "fail to find a result of the instruction" <<endl;
    }
}
void SimulatorUI::reset(Machine&machine){

    machine.resetAll();
    cout << "the machine is reset" <<endl;
}
void SimulatorUI::displayMemory(Machine&machine){
   const Memory& memory = machine.getMemory();

    for(int i = 0; i < 16; i++ ){
    for(int j = 0; j < 16; j++ ){

    cout << hex << uppercase << setw(2) << setfill('0')
     << static_cast<int>(memory.read((i * 16) + j)) << " ";
        
    }
        cout << endl;
}
        cout << dec;
}
void SimulatorUI::displayRegister(Machine&machine){
    const RegisterFile& registers = machine.getRegisters();

    for(int i = 0; i < 16; i++ ){

     cout << "R" << i << ":" ;    
    cout << hex << uppercase << setw(2) << setfill('0')
     << static_cast<int>(registers.get(i)) << " ";      
}
    cout << dec;
}
void SimulatorUI::modifyMemory(Machine&machine){
    string address, value;

    cout << "Enter address" <<endl;
    cin >> address;
    cout << "Enter value" <<endl;
    cin >> value;

    uint8_t x = stoi(address, nullptr, 16);
    uint8_t y = stoi(value, nullptr, 16);

    machine.setMemoryCell(x,y);
    cout << "Memory is modified" <<endl;
}
void SimulatorUI::modifyRegister(Machine&machine){
    string value;
    string regNum;

    cout << "Enter register number" <<endl;
    cin >> regNum;
    cout << "Enter value" <<endl;
    cin >> value;

    uint8_t x = stoi(regNum, nullptr, 16);
    uint8_t y = stoi(value, nullptr, 16);

    machine.setRegister(x, y);
    cout << "Register is modified" <<endl;
}
void SimulatorUI::printState(Machine& machine){
    uint8_t pc = machine.getPC();
    cout << "PC: " << dec << static_cast<int>(pc) << " ";  

    string ir = machine.getIR();
    cout << "IR:"<< ir << endl;
    const RegisterFile& registers = machine.getRegisters();

    for(int i = 0; i < 16; i++ ){

     cout << "R" << i << ":" ;    
    cout << hex << uppercase << setw(2) << setfill('0')
     << static_cast<int>(registers.get(i)) << " ";      
    
}
     cout << dec;
}
void SimulatorUI::printScreen(Machine& machine) {
    const string output = machine.getMemory().getScreenOutput();

    cout << "\n========== SCREEN OUTPUT ==========\n";
    cout << output << endl;
}
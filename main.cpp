#include<iostream>
#include "SimulatorUI.h"
using namespace std;


int main(){
    int choice;
    cout << "Welcome to the Vole Machine Simulator!" << endl;
   
    Machine machine;
    SimulatorUI ui;

    do{
        ui.displayMenu();
    cout << "Enter an option from the menu above." << endl;
    cin >> choice;
    switch(choice){

        case 1:
             ui.loadProgram(machine);
            break;
        case 2:    
             ui.run(machine);
            break;
        case 3:    
           ui.step(machine);
            break;
        case 4:    
            ui.reset(machine);
            break;
        case 5:    
            ui.displayMemory(machine);
            break;
        case 6:    
            ui.displayRegister(machine);
            break;
        case 7:    
            ui.modifyMemory(machine);
            break;
        case 8:    
            ui.modifyRegister(machine);
            break;
        case 9:  
            ui.printState(machine);
            break; 
        case 10:  
            ui.printScreen(machine);
            break;
        case 11:        
            cout <<"Exit"<<endl;
            break;
        default:
            cout <<"Invalid option"<<endl;
            break;
    }   


    }
    while(choice != 11 );

    return 0;
}
#include<iostream>
#include "SimulatorUI.h"
using namespace std;


int main(){
    int choice;
    cout << "Welcome to the Vole Machine Simulator!" << endl;
   
    Machine machine;

    do{
        displayMenu();
    cout << "Enter an option from the menu above." << endl;
    cin >> choice;
    switch(choice){

        case 1:
             loadProgram(machine);
            break;
        case 2:    
             run(machine);
            break;
        case 3:    
           step(machine);
            break;
        case 4:    
            reset(machine);
            break;
        case 5:    
            displayMemory(machine);
            break;
        case 6:    
            displayRegister(machine);
            break;
        case 7:    
            modifyMemory(machine);
            break;
        case 8:    
            modifyRegister(machine);
            break;
        case 9:    
            cout <<"Exit"<<endl;
            break;
        default:
            cout <<"Invalid option"<<endl;
            break;
    }   


    }
    while(choice != 9 );

    return 0;
}

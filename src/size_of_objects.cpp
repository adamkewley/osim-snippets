#include <OpenSim/OpenSim.h>
#include <iostream>

// COMPILEME: g++ main.cpp -o main -I include/ -I include/simbody/ -I include/OpenSim/ -L lib/ -losimTools -losimAnalyses -losimActuators -losimSimulation -losimCommon

#define PRINT_CLASS( o ) std::cout << "sizeof(" #o ") = " << sizeof(o) << std::endl;

int main(int argc, char** argv) {
  PRINT_CLASS(OpenSim::Object);
  PRINT_CLASS(OpenSim::Component);
  PRINT_CLASS(OpenSim::ModelComponent);
  PRINT_CLASS(OpenSim::Point);
  return 0;
}

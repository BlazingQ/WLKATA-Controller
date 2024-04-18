#include "tcpdocker.h"
#include "arm_converter/armVerifier.hpp"
#include <iostream>

using namespace std;

int main(){
    cout<<"start"<<endl;
    listen("192.168.1.102", 12345);

    // send("192.168.1.102", 12345, "hello from docker!");
    return 0;
}
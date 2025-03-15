#include "ArmControllerServer.hpp"
#include <iostream>

int main() {
    ArmControllerServer server;
    int port = 12345;
    
    // server.runServer(port, false); 
    // server.runServer(port, true); 

    // string cmds = readFile("input.md");
    // string id = readFile("input2.md");
    // server.visualize(cmds, stoi(id));

    overwriteToFile("", "json/control.json");
    overwriteToFile("", "json/status.json");
    overwriteToFile("", "timeused.md");
    string statusstr = readFile("json/testinput.json");
    server.oneRun(statusstr, 0, true);

    // bool vrfres = arm_verify(statusstr);
    // cout << vrfres << endl;

    // string controlresult = arm_control(statusstr, 5);
    // cout << controlresult << endl;

    return 0;
}

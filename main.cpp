#include "ArmControllerServer.hpp"
#include <iostream>

int main() {
    ArmControllerServer server;
    int port = 12345;  // 你可以指定你的服务器端口
    
    server.runServer(port);  // 启动服务器并运行

    // string statusstr = readFile("json/testinput.json");
    // server.oneRun(statusstr, 0, true);
    // bool vrfres = arm_verify(statusstr);

    // bool vrfres = server.verifyMultiArm(statusstr, 4);
    // cout << vrfres << endl;

    // string controlresult = arm_control(statusstr, 3);
    // cout << controlresult << endl;

    return 0;
}

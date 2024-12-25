#include "ArmControllerServer.hpp"
#include <iostream>

int main() {
    ArmControllerServer server;
    int port = 12345;  // 你可以指定你的服务器端口

    // std::cout << "Starting Arm Controller Server on port " << port << "..." << std::endl;
    
    // server.runServer(port);  // 启动服务器并运行

    string statusstr = readFile("json/status.json");
    // json statusjson = json::parse(statusstr);

    // string jsonstr = server.transCmds(statusjson);

    // cout << jsonstr << endl;

    string controlresult = arm_control(statusstr, 3);

    cout << controlresult << endl;

    return 0;
}

#ifndef ARM_CONTROLLER_SERVER_HPP
#define ARM_CONTROLLER_SERVER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <map>
#include "nlohmann/json.hpp"
#include "tcpdocker.h"
#include "arm_converter/armVerifier.hpp"
#include "arm_controller/armController.hpp"

using json = nlohmann::json;



class ArmControllerServer {
private:
    std::map<int, json> armConfigs;
    std::vector<double> locs;
    bool isincre;
    bool isangle;

    // 私有成员函数
    json parseInitialState(const std::string& data);
    json parseCommand(const std::string& cmd);
    json parseComponent(const json& singleArm, json& result);
    std::string transCmds(const std::string& status);
    std::string generateCmd(const json& behavior);
    std::string jsonToCmds(const std::string& jsonString);
    
    void initializeArmConfigs();

public:
    ArmControllerServer();
    void runServer(int port);
};

std::string doubleToStr(double value);
void overwriteToFile(const std::string& str, const std::string& filename);
std::string readFile(const std::string& filename);
long long int timenow();
void appendToFile(const std::string& str, const std::string& filename);


#endif

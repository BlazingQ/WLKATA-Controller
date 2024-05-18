#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <chrono>
#include "arm_converter/armVerifier.hpp"
#include "arm_controller/armController.hpp"
#include "nlohmann/json.hpp"
using json = nlohmann::json;

json parseInitialState(const std::string& data);
json parseComponent(std::string singleArm, json& result);
json parseCommand(const std::string& cmd);
std::string transCmds(const std::string& cmd);
// std::string transCmds2(const std::string& cmd);
std::string generateCmd(const json& behavior);
std::string jsonToCmds(const std::string& jsonString);
std::string doubleToStr(double value);
void overwriteToFile(const std::string& str, const std::string& filename);
void appendToFile(const std::string& str, const std::string& filename);
std::string readFile(const std::string& filename);
long long int timenow();
void initializeArmConfigs();
// void basicArmInfo(json& result, int armindex);
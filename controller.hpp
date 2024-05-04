#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "arm_converter/armVerifier.hpp"
#include "arm_controller/armController.hpp"
#include "nlohmann/json.hpp"
using json = nlohmann::json;

json parseInitialState(const std::string& data);
std::string transCmd(const std::string& cmd);
json parseCommand(const std::string& cmd);
std::string generateCmd(const json& behavior);
std::string jsonToCmds(const std::string& jsonString);
std::string doubleToStr(double value);
void initializeArmConfigs();
void basicArmInfo(json& result, int armindex);
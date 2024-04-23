#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "arm_converter/armVerifier.hpp"
#include "arm_controller/armController.hpp"
#include "nlohmann/json.hpp"
using json = nlohmann::json;

json parseInitialState(const std::string& data);
string transCmd(const string& cmd);
json parseCommand(const string& cmd);
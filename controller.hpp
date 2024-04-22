#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "arm_converter/armVerifier.hpp"
#include "nlohmann/json.hpp"
using json = nlohmann::json;


string transCmd(const string& cmd);
json parseCommand(const string& cmd);
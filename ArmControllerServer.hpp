#ifndef ARM_CONTROLLER_SERVER_HPP
#define ARM_CONTROLLER_SERVER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <map>
#include <unordered_map>
// #include <variant>
#include "nlohmann/json.hpp"
#include "tcpdocker.h"
#include "arm_converter/armVerifier.hpp"
#include "arm_controller/armController.hpp"

using json = nlohmann::json;
// using VariantType = std::variant<int, double, bool, std::string>;

class ArmControllerServer {
private:
    std::map<int, json> armConfigs;
    std::vector<double> locs;
    bool isincre;
    bool isangle;

    // 私有成员函数
    json preprocessStateJson(json statusstr);
    std::pair<std::pair<int, int>, json> getExecTime(json& j, int cmdId);
    json parseInitialState(const std::string& data);
    json parseCommand(const std::string& cmd);
    json parseComponent(const json& singleArm, json& result);
    std::string transCmds(json arms);
    std::string generateCmd(const json& behavior);
    std::string jsonToCmds(const std::string& jsonString);
    std::string verifyMsg(const int armid, const int cmdid, const int vrfres);
    void initializeArmConfigs();
    void updateLocs(std::string cmd, float locs[]);

public:
    ArmControllerServer();
    void runServer(int port);
};

std::string doubleToStr(double value);
void overwriteToFile(const std::string& str, const std::string& filename);
std::string readFile(const std::string& filename);
void appendToFile(const std::string& str, const std::string& filename);
long long int timenow();
std::vector<std::string> decodeCommaStr(const std::string& commands, int startIndex, int length);
std::string subCommaStr(const std::string& commands, int startIndex, int length);
std::string encodeCommaStr(std::string commandArray[], int arraysize, int startIndex, int length);
json unordered_map_to_json(const std::unordered_map<std::string, std::string>& umap);
std::unordered_map<std::string, std::string> json_to_unordered_map(const json& j);



#endif

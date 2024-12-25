#ifndef ARM_CONTROLLER_SERVER_HPP
#define ARM_CONTROLLER_SERVER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <map>
#include <unordered_map>
#include <climits>
#include <algorithm>
// #include <variant>

#include "nlohmann/json.hpp"
#include "tcpdocker.h"
#include "arm_converter/armVerifier.hpp"
#include "arm_controller/armController.hpp"

using json = nlohmann::json;
using namespace std;
// using VariantType = variant<int, double, bool, string>;

class ArmControllerServer {
private:
    map<int, json> armConfigs;
    vector<double> locs;
    bool isincre;
    bool isangle;

    // 私有成员函数
    json preprocessStateJson(string statusstr);
    pair<pair<int, int>, json> getVrfArmInfo(json armjson, int vrfid);
    pair<int, json> getOtherArmInfo(json armjson, pair<int, int> timepair, int delay);
    json parseInitialState(const string& data);
    json parseCommand(const string& cmd);
    json parseComponent(const json& singleArm, json& result);
    string generateCmd(const json& behavior);
    string jsonToCmds(const string& jsonString);
    string verifyMsg(const int armid, const int vrfid, const int vrfres);
    string controlMsg(const string& vrfjsonstr, const string& controljsonstr);
    void initializeArmConfigs();
    void updateLocs(string cmd, float locs[]);

public:
    ArmControllerServer();
    void runServer(int port);
    string transCmds(json arms);
};

string doubleToStr(double value);
void overwriteToFile(const string& str, const string& filename);
string readFile(const string& filename);
void appendToFile(const string& str, const string& filename);
long long int timenow();
vector<string> decodeCommaStr(const string& commands, int startIndex, int length);
vector<int> commaStrtoInt(const string& commands, int startIndex, int length);
string subCommaStr(const string& commands, int startIndex, int length);
string encodeCommaStr(string commandArray[], int arraysize, int startIndex, int length);
json unordered_map_to_json(const unordered_map<string, string>& umap);
unordered_map<string, string> json_to_unordered_map(const json& j);



#endif

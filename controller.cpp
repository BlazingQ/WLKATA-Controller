#include "tcpdocker.h"
#include "controller.hpp"

using namespace std;

std::vector<double> locs;
bool isincre = false;
bool isangle = false;
map<int, json> armConfigs;

int main(){
    initializeArmConfigs();
    int server_fd = setup_server(12345);
    if (server_fd < 0){
        return 1;
    }

    while(true){
        int client_fd = wait_for_connection(server_fd);
        if (client_fd < 0){
            return 1;
        }
        char buffer[4096] = {0};
        if(receive_message(client_fd, buffer, 4096)){//process
            // strcat(buffer, "send back");
            // send_message(client_fd, buffer);
            string cmd = buffer;
            string jsonstr = transCmd(cmd);
            cout<<jsonstr<<endl;
            
            if(!jsonstr.empty() && !arm_verify(jsonstr)){
                string controljsonstr;
                string cmdsendback;
                controljsonstr = arm_control(jsonstr);
                cout<<"\ncontroljsonstr = "<<controljsonstr<<endl;
                if(!controljsonstr.empty()){
                    cmdsendback = jsonToCmds(controljsonstr);
                }
                cout<<"\ncmdsendback = "<<cmdsendback<<endl;
                send_message(client_fd, cmdsendback.c_str());
                
            }
            else{//safe
                send_message(client_fd, "OK");
            }
            
        }
        close_connection(client_fd);
    }
    close_connection(server_fd);

    return 0;
}


json parseInitialState(const std::string& data) {
    std::istringstream iss(data);
    std::string value;
    

    while (getline(iss, value, ',')) {
        locs.push_back(std::stod(value));
    }

    json initialState = {
        {"Mode", "Cartesian"},
        {"X", {locs[0], locs[1], locs[2]}},
        {"R", {locs[3], locs[4], locs[5]}}
    };

    return initialState;
}

json parseCommand(const std::string& cmd) {
    std::istringstream iss(cmd);
    std::string part;
    json behavior;
    isincre = false;
    isangle = false;
    behavior["X"] = json::array();
    behavior["R"] = json::array();
    behavior["Theta"] = json::array();
    behavior["Mode"] = "Cartesian";
    behavior["Motion type"] = "Linear";
    while (iss >> part) {
        if (part[0] == 'M') {
            if(part == "M20"){
                behavior["Mode"] = "Cartesian";
            }
            else if(part == "M21"){
                behavior["Mode"] = "Angle";
                isangle = true;
            }
        } else if (part[0] == 'G') {
            if (part == "G00") {
                behavior["Motion type"] = "Joint";
            } else if (part == "G01") {
                behavior["Motion type"] = "Linear";
            } else if (part == "G91"){
                isincre = true;
            }
        } else if (part[0] == 'X' || part[0] == 'Y' || part[0] == 'Z' ||
         part[0] == 'A' || part[0] == 'B' || part[0] == 'C') {
            std::size_t pos = part.find_first_not_of("ABCXYZ");
            if (pos != std::string::npos) {
                switch (part[0])
                {
                case 'X':
                    if(!isincre){
                        locs[0] = std::stod(part.substr(pos));
                    }
                    else{
                        locs[0] += std::stod(part.substr(pos));
                    }
                    if(!isangle){
                        behavior["X"].push_back(locs[0]);
                    }else{
                        behavior["Theta"].push_back(locs[0]);
                    }
                    break;
                case 'Y':
                    if(!isincre){
                        locs[1] = std::stod(part.substr(pos));
                    }
                    else{
                        locs[1] += std::stod(part.substr(pos));
                    }
                    if(!isangle){
                        behavior["X"].push_back(locs[1]);
                    }else{
                        behavior["Theta"].push_back(locs[1]);
                    }
                    break;
                case 'Z':
                    if(!isincre){
                        locs[2] = std::stod(part.substr(pos));
                    }
                    else{
                        locs[2] += std::stod(part.substr(pos));
                    }
                    if(!isangle){
                        behavior["X"].push_back(locs[2]);
                    }else{
                        behavior["Theta"].push_back(locs[2]);
                    }
                    break;
                case 'A':
                    if(!isincre){
                        locs[3] = std::stod(part.substr(pos));
                    }
                    else{
                        locs[3] += std::stod(part.substr(pos));
                    }
                    if(!isangle){
                        behavior["R"].push_back(locs[3]);
                    }else{
                        behavior["Theta"].push_back(locs[3]);
                    }
                    break;
                case 'B':
                    if(!isincre){
                        locs[4] = std::stod(part.substr(pos));
                    }
                    else{
                        locs[4] += std::stod(part.substr(pos));
                    }
                    if(!isangle){
                        behavior["R"].push_back(locs[4]);
                    }else{
                        behavior["Theta"].push_back(locs[4]);
                    }
                    break;
                case 'C':
                    if(!isincre){
                        locs[5] = std::stod(part.substr(pos));
                    }
                    else{
                        locs[5] += std::stod(part.substr(pos));
                    }
                    if(!isangle){
                        behavior["R"].push_back(locs[5]);
                    }else{
                        behavior["Theta"].push_back(locs[5]);
                    }
                    break;
                default:
                    break;
                }
                
            }
        } 
        // else if (part[0] == 'A' || part[0] == 'B' || part[0] == 'C') {
        //     std::size_t pos = part.find_first_not_of("ABC");
        //     if (pos != std::string::npos) {
        //         behavior["R"].push_back(std::stod(part.substr(pos)));
        //     }
        // }
    }

    // Speed is fixed as 2000
    behavior["Speed"] = 2000;

    return behavior;
}

// 主函数，处理整个指令字符串
std::string transCmd(const std::string& packet) {
    std::size_t delimiterPos = packet.find(';');
    std::size_t delimiterRPos = packet.rfind(';');
    int armindex = std::stoi(packet.substr(0, delimiterPos));
    std::string initialStateData = packet.substr(delimiterPos + 1, delimiterRPos - delimiterPos - 1);
    std::string commands = packet.substr(delimiterRPos + 1);

    locs.clear();
    
    json result;
    // result["Obstacles"] = json::array({ {{"Radius", 10.0},{"X", {100, 100, 100}}} });
    result["Obstacles"] = json::array();
    result["Components"] = json::array({
        {
            {"BasePosition", {0, 0, 0}},
            {"InitialState", parseInitialState(initialStateData)},
            {"Behavior", json::array()}
        }
    });
    basicArmInfo(result, armindex);
    std::istringstream iss(commands);
    std::string token;
    json lastCommand;
    bool hasTargetState = false;
    while (getline(iss, token, ',')) {
        if (token.find("M20") != std::string::npos || token.find("M21") != std::string::npos) {
            json behavior = parseCommand(token);
            lastCommand = behavior;
            hasTargetState = true;
            result["Components"][0]["Behavior"].push_back(behavior);
        }
    }

    if (hasTargetState) {
        result["Components"][0]["TargetState"] = lastCommand;
    }
    if(result["Components"][0]["Behavior"].empty()){
        return "";
    }
    return result.dump(4);  // 输出格式化的 JSON 字符串
}

std::string generateCmd(const json& behavior) {
    std::string cmd = "M20 G90 "; // 假设都是绝对坐标系统
    cmd += behavior["Motion type"] == "Joint" ? "G00 " : "G01 ";

    // 处理 X, Y, Z 坐标
    std::vector<std::string> axis = {"X", "Y", "Z"};
    for (int i = 0; i < 3; ++i) {
        cmd += axis[i] + doubleToStr(behavior["X"][i]) + " ";
    }

    // 处理 A, B, C 坐标（旋转）
    axis = {"A", "B", "C"};
    for (int i = 0; i < 3; ++i) {
        cmd += axis[i] + doubleToStr(behavior["R"][i]) + " ";
    }

    return cmd;
}

std::string jsonToCmds(const std::string& jsonString) {
    json j = json::parse(jsonString);
    std::string cmds = "";

    // 遍历行为数组并生成指令
    for (const auto& behavior : j["Components"][0]["Behavior"]) {
        if (!cmds.empty()) {
            cmds += ",";
        }
        cmds += generateCmd(behavior);
    }

    return cmds;
}

std::string doubleToStr(double value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << value;
    return oss.str();
}

void initializeArmConfigs() {
    // 初始化示例数据
    armConfigs[1] = {
        {"BasePosition", {0, 0, 0}},
        {"Obstacles", {{{"X", {71.19, 163.67, 190.66}}, {"Radius", 10}}}}
    };
    armConfigs[2] = {
        {"BasePosition", {420, 230, 0}},
        {"Obstacles", {{{"X", {50, 100, 150}}, {"Radius", 20}}}}
    };
    armConfigs[3] = {
        {"BasePosition", {800, 230, 0}},
        {"Obstacles", {{{"X", {30, 60, 90}}, {"Radius", 10}}}}
    };
    armConfigs[4] = {
        {"BasePosition", {800, -230, 0}},
        {"Obstacles", {
        {{"X", {800, -55, 205}}, {"Radius", 10}}}}
    };
    armConfigs[5] = {
        {"BasePosition", {0, 0, 0}},
        {"Obstacles", {{{"X", {0, 0, 0}}, {"Radius", 10}}}}
    };
}


void basicArmInfo(json& result, int armindex){//basePosition and Obstacles
    if (armConfigs.find(armindex) != armConfigs.end()) {
        result["Components"][0]["BasePosition"] = armConfigs[armindex]["BasePosition"];
        result["Obstacles"] = armConfigs[armindex]["Obstacles"];
    } else {
        cout << "Invalid arm index." << endl;
    }
}
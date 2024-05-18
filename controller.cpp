#include "tcpdocker.h"
#include "controller.hpp"

using namespace std;

std::vector<double> locs;
bool isincre = false;
bool isangle = false;
map<int, json> armConfigs;

int main(){
    initializeArmConfigs();
    overwriteToFile("\n", "json/verify.json");
    overwriteToFile("\n", "json/control.json");
    overwriteToFile("\n", "json/status.json");
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
            auto starttime = timenow();
            string status = buffer;
            appendToFile(status, "json/status.json");
            string jsonstr = transCmds(status);
            cout<<jsonstr<<endl;
            appendToFile(jsonstr, "json/verify.json");
            if(!jsonstr.empty() && !arm_verify(jsonstr)){
                string controljsonstr;
                string cmdsendback;
                controljsonstr = arm_control(jsonstr);
                cout<<"\ncontroljsonstr = "<<controljsonstr<<endl;
                appendToFile(controljsonstr, "json/control.json");
                if(!controljsonstr.empty()){
                    cmdsendback = jsonToCmds(controljsonstr);
                }
                // cout<<"\ncmdsendback = "<<cmdsendback<<endl;
                send_message(client_fd, cmdsendback.c_str());
                
            }
            else{//safe
                send_message(client_fd, "OK");
            }
            auto endtime = timenow();
            
            cout<<"\n time = "<<endtime - starttime<<endl;
        }
        close_connection(client_fd);
    }
    close_connection(server_fd);

    return 0;
}

json parseInitialState(const std::string& data) {
    std::istringstream iss(data);
    std::string value;
    locs.clear();

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

json parseComponent(json singleArm, json& result){
    json component;
    int armindex = singleArm["ArmId"];
    string initialStateData = singleArm["Locs"];
    string commands = singleArm["Cmds"];
    // std::size_t delimiterPos = singleArm.find(';');
    // std::size_t delimiterRPos = singleArm.rfind(';');
    // int armindex = std::stoi(singleArm.substr(0, delimiterPos));
    // std::string initialStateData = singleArm.substr(delimiterPos + 1, delimiterRPos - delimiterPos - 1);
    // std::string commands = singleArm.substr(delimiterRPos + 1);
    if (armConfigs.find(armindex) != armConfigs.end()) {
        component["BasePosition"] = armConfigs[armindex]["BasePosition"];
        for (const auto& obstacle : armConfigs[armindex]["Obstacles"]) {
                result["Obstacles"].push_back(obstacle); // 逐个添加
            }
        // result["Obstacles"].push_back(armConfigs[armindex]["Obstacles"]);
    } else {
        cout << "Invalid arm index." << endl;
    }
    component["InitialState"] = parseInitialState(initialStateData);
    component["Behavior"] = json::array();

    std::istringstream iss(commands);
    std::string token;
    json lastCommand;
    bool hasTargetState = false;
    while (getline(iss, token, ',')) {
        if (token.find("M20") != std::string::npos || token.find("M21") != std::string::npos) {
            json behavior = parseCommand(token);
            lastCommand = behavior;
            hasTargetState = true;
            component["Behavior"].push_back(behavior);
        }
    }

    if (hasTargetState) {
        component["TargetState"] = lastCommand;
    }

    if(component["Behavior"].empty()){//放弃之后的验证与控制生成
        return NULL;
    }
    return component;
}

// 主函数，处理整个指令字符串
std::string transCmds(const std::string& status) {
    json arms = json::parse(status);
    json result;
    result["Obstacles"] = json::array();
    result["Components"] = json::array();
    // int index = 0;
    // std::string singleArm;
    // std::string strcopy = packet;//auto deep copy
    // std::size_t AndPos = strcopy.find('&');
    // while(AndPos != std::string::npos){
    //     singleArm = strcopy.substr(0, AndPos);
    for(json arm: arms){
        json component = parseComponent(arm, result);
        if(component == NULL){//no behavior
            return "";
        }
        result["Components"].push_back(component);
        // strcopy = strcopy.substr(AndPos + 1);
        // AndPos = strcopy.find('&');
        // index++;
    }
    // if(strcopy != ""){
    //     json component = parseComponent(strcopy, result);
    //     if(component == NULL){//no behavior
    //         return "";
    //     }
    //     result["Components"].push_back(component);
    // }

    return result.dump(4);  // 输出格式化的 JSON 字符串
}

// std::string transCmds2(const std::string& packet) {
//     json result;
//     result["Obstacles"] = json::array();
//     result["Components"] = json::array();
//     // int index = 0;
//     std::string singleArm;
//     std::string strcopy = packet;//auto deep copy
//     std::size_t AndPos = strcopy.find('&');
//     while(AndPos != std::string::npos){
//         singleArm = strcopy.substr(0, AndPos);
//         singleArm = "4;183.669998,0.000000,230.000000,0.000000,0.000000,0.000000;" + singleArm;
//         //func here
//         json component = parseComponent(singleArm, result);
//         if(component == NULL){//no behavior
//             return "";
//         }
//         result["Components"].push_back(component);
//         strcopy = strcopy.substr(AndPos + 1);
//         AndPos = strcopy.find('&');
//         // index++;
//     }
//     if(strcopy != ""){
//         strcopy = "4;183.669998,0.000000,230.000000,0.000000,0.000000,0.000000;" + strcopy;
//         json component = parseComponent(strcopy, result);
//         if(component == NULL){//no behavior
//             return "";
//         }
//         result["Components"].push_back(component);
//     }

//     return result.dump(4);  // 输出格式化的 JSON 字符串
// }

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
    std::string cmdsback = "";
    std::vector<std::string> cmds;
    cmds.resize(6);
    for(int i = 0; i < 6; ++i){
        cmds[i] = "";
    }

    for(const auto& component: j["Components"]){
        // if(!cmds.empty()){
        //     cmds += "&";
        // }
        std::string cmd = "";
        for (const auto& behavior : component["Behavior"]) {
            if (!cmd.empty()) {
                cmd += ",";
            }
            cmd += generateCmd(behavior);
        }
        // cmds += cmd;
        for (const auto& [key, value] : armConfigs) {
            if (value["BasePosition"] == component["BasePosition"]) {
                cmds[key] = cmd;
            }
        }
    }

    for(int i = 0; i < 6; ++i){
        if(cmds[i] != ""){
            if(cmdsback != ""){
                cmdsback += "&";
            }
            cmdsback += cmds[i];
        }
    }

    return cmdsback;
}

std::string doubleToStr(double value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << value;
    return oss.str();
}

void overwriteToFile(const std::string& str, const std::string& filename) {
    // 打开文件，使用 std::ios::out 模式以覆盖方式写入
    std::ofstream file(filename, std::ios::out);
    if (file.is_open()) {
        // 写入字符串并添加换行符
        file << str << '\n';
        // 关闭文件
        file.close();
    } else {
        // 文件打开失败，输出错误消息
        std::cerr << "Unable to open file: " << filename << std::endl;
    }
}

std::string readFile(const std::string& filename) {
    // 打开文件
    std::ifstream file(filename);
    if (!file.is_open()) {
        // 文件打开失败，输出错误消息
        std::cerr << "Unable to open file: " << filename << std::endl;
        return "";
    }
    
    // 使用 std::istreambuf_iterator 读取数据，直到 EOF
    std::string str((std::istreambuf_iterator<char>(file)),
                    std::istreambuf_iterator<char>());
    
    // 关闭文件
    file.close();
    
    // 打印读取的结果
    std::cout << "Read file: " << filename << std::endl;
    
    return str;
}

long long int timenow(){
    auto now = std::chrono::high_resolution_clock::now();
    auto duration_since_epoch = now.time_since_epoch();
    long long int millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration_since_epoch).count();
    return millis;
}

void appendToFile(const std::string& str, const std::string& filename) {
    // 打开文件，使用 std::ios::app 模式以追加方式写入
    std::ofstream file(filename, std::ios::app);
    if (file.is_open()) {
        // 写入字符串并添加换行符
        file << str << endl;
        // 关闭文件
        file.close();
    } else {
        // 文件打开失败，输出错误消息
        std::cerr << "Unable to open file: " << filename << std::endl;
    }
}




void initializeArmConfigs() {
    // 初始化示例数据
    armConfigs[1] = {
        {"BasePosition", {0, 0, 0}},
        {"Obstacles", 
            {
                {{"X", {71.19, 163.67, 190.66}}, {"Radius", 10}}
        
            }
        }
    };
    armConfigs[2] = {
        {"BasePosition", {420, 230, 0}},
        {"Obstacles", 
            {
                {{"X", {71.19, 163.67, 190.66}}, {"Radius", 10}}
        
            }
        }
    };
    armConfigs[3] = {
        {"BasePosition", {800, 230, 0}},
        {"Obstacles", 
            {
                {{"X", {71.19, 163.67, 190.66}}, {"Radius", 10}}
        
            }
        }
    };
    armConfigs[4] = {
        {"BasePosition", {800, -230, 0}},
        {"Obstacles", 
            {
                {{"X", {800, -55, 205}}, {"Radius", 10}},
                {{"X", {900, -140, 200}}, {"Radius", 10}}
            }
        }
    };
    //25传送带
    armConfigs[5] = {
        {"BasePosition", {1600, -230, 0}},
        {"Obstacles", 
            {
                {{"X", {0, 0, 0}}, {"Radius", 10}}
            
            }
        }
    };
}


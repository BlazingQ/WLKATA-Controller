#include "ArmControllerServer.hpp"

ArmControllerServer::ArmControllerServer() : isincre(false), isangle(false) {
    initializeArmConfigs();
}

void ArmControllerServer::runServer(int port) {
    overwriteToFile("\n", "json/verify.json");
    overwriteToFile("\n", "json/control.json");
    overwriteToFile("\n", "json/status.json");

    int server_fd = setup_server(port);
    if (server_fd < 0) {
        std::cerr << "Failed to set up server." << std::endl;
        return;
    }

    while (true) {
        int client_fd = wait_for_connection(server_fd);
        if (client_fd < 0) {
            std::cerr << "Failed to accept connection." << std::endl;
            continue;
        }

        char buffer[4096] = {0};
        if (receive_message(client_fd, buffer, 4096)) {
            auto starttime = timenow();
            std::string statusstr = buffer;
            if(buffer == ""){
                send_message(client_fd, verifyMsg(0, 0, 0).c_str());
            }else{
                appendToFile(statusstr, "json/status.json");
                json statusjson = json::parse(statusstr);
                int armid = statusjson["armid"];
                int cmdid = statusjson["cmdid"];
                int isinit = statusjson["isinit"];
                json arms = statusjson["Arms"];

                std::string jsonstr = transCmds(arms); // 调用命令处理函数
                if(!jsonstr.empty()){
                    bool res = arm_verify(jsonstr);
                    send_message(client_fd, verifyMsg(armid, cmdid, res).c_str());
                }
                // if (!jsonstr.empty() && !arm_verify(jsonstr)) {
                //     std::string controljsonstr = arm_control(jsonstr);
                //     appendToFile(controljsonstr, "json/control.json");

                //     if (!controljsonstr.empty()) {
                //         std::string cmdsendback = jsonToCmds(controljsonstr);
                //         send_message(client_fd, cmdsendback.c_str());
                //     }
                // } else {
                //     send_message(client_fd, "OK");
                // }

                auto endtime = timenow();
                appendToFile(std::to_string(endtime - starttime), "timeused.md");
            }
        }

        close_connection(client_fd);
    }

    close_connection(server_fd);
}

/*为原始status信息进行处理的总体函数，先输出一个格式能对接transCmds，后续可以考虑再结合*/
json ArmControllerServer::preprocessStateJson(json statusstr){
    json statusjson = json::parse(statusstr);
    int armid = statusjson["armid"];
    int cmdid = statusjson["cmdid"];
    int isinit = statusjson["isinit"];
    json arms = statusjson["Arms"];
    json retarms = json::array();
    if(isinit){//暂定和之前一样验证前三个指令，目前是所有arm都验证，会有重复验证
        for(json arm: arms){
            std::string cmds = arm["Cmds"];
            std::string subcmds = subCommaStr(cmds, 0, 3);
            arm["Cmds"] = subcmds;
            //duration后面暂时用不到,改不改区别不大
            std::string durations = arm["Durations"];
            std::string subdurations = subCommaStr(durations, 0, 3);
            arm["Durations"] = subcmds;
            retarms.push_back(arm);
        }
    }else{
        json vrfarm;
        for(json arm: arms){
            if(arm["ArmId"] == armid){
                vrfarm = arm;
            }
        }
        if(vrfarm.empty()){
            return nullptr;
        }

    }
    return retarms;
}

std::pair<std::pair<int, int>, json> ArmControllerServer::getExecTime(json& j, int cmdId) {
    
    // 获取 Cmds 和 Durations 字段
    std::string cmds = j["Cmds"];
    std::string durations = j["Durations"];
    std::string locsstr = j["Locs"];

    float locallocs[6];
    std::istringstream locsStream(locsstr);
    std::string loc;
    int i = 0;
    while (std::getline(locsStream, loc, ',')) {
        if (!loc.empty()) {
            locallocs[i] = 
        }
    }
    
    // 按逗号分割指令和持续时间
    std::vector<std::string> cmdList;
    std::vector<int> durationList;
    
    std::istringstream cmdStream(cmds);
    std::string cmd;
    while (std::getline(cmdStream, cmd, ',')) {
        if (!cmd.empty()) {
            cmdList.push_back(cmd);
            
        }
    }
    
    std::istringstream durationStream(durations);
    std::string duration;
    while (std::getline(durationStream, duration, ',')) {
        if (!duration.empty()) {
            durationList.push_back(std::stoi(duration));
        }
    }
    
    // 检查 cmdId 的有效性
    int cmdIndex = cmdId - j["CmdId"]; // 计算 Cmds 中的索引
    if (cmdIndex < 0 || cmdIndex >= static_cast<int>(cmdList.size())) {
        throw std::invalid_argument("Invalid CmdId provided.");
    }

    // 计算起止时间
    int startTime = 0; // Cmds 的第一条指令的起始时间为 0
    int durationSum = 0;

    // 累加直到 cmdId 对应的指令
    for (int i = 0; i < cmdIndex; ++i) {
        durationSum += durationList[i];
    }

    int endTime = startTime + durationSum + durationList[cmdIndex];
    return {{startTime + durationSum, endTime}, j};
}

/*根据传来的locs信息确定起始位置*/
json ArmControllerServer::parseInitialState(const std::string& data) {
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

/*几乎是硬编码做的指令解析，不过由于本身指令解析不是实验重点，
也没必要用更复杂的方法*/
json ArmControllerServer::parseCommand(const std::string& cmd) {
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
    }

    // Speed is fixed as 2000
    behavior["Speed"] = 2000;

    return behavior;
}

/*解析从客户端传来的当前状态，根据armId和服务器端的arm基准位置坐标可以
获取全局位置坐标，其他信息全部由客户端提供。 */
json ArmControllerServer::parseComponent(const json& singleArm, json& result){
    json component;
    int armindex = singleArm["ArmId"];
    std::string initialStateData = singleArm["Locs"];
    std::string commands = singleArm["Cmds"];
    if (armConfigs.find(armindex) != armConfigs.end()) {
        component["BasePosition"] = armConfigs[armindex]["BasePosition"];
        for (const auto& obstacle : armConfigs[armindex]["Obstacles"]) {
                result["Obstacles"].push_back(obstacle); // 逐个添加
            }
    } else {
        std::cout << "Invalid arm index." << std::endl;
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
std::string ArmControllerServer::transCmds(json arms) {
    
    json result;
    result["Obstacles"] = json::array();
    result["Components"] = json::array();

    for(json arm: arms){
        json component = parseComponent(arm, result);
        if(component == NULL){//no behavior
            return "";
        }
        result["Components"].push_back(component);
    }

    return result.dump(4);  // 输出格式化的 JSON 字符串
}

std::string ArmControllerServer::generateCmd(const json& behavior) {
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

/*根据验证与控制生成的结果，将其重新封装为返回的cmd串。
目前的格式不太美妙，可以继续设计，返回的串的格式很机械，
就是按armId的顺序排的*/
std::string ArmControllerServer::jsonToCmds(const std::string& jsonString) {
    json j = json::parse(jsonString);
    std::string cmdsback = "";
    std::vector<std::string> cmds;
    cmds.resize(6);
    for(int i = 0; i < 6; ++i){
        cmds[i] = "";
    }

    for(const auto& component: j["Components"]){
        std::string cmd = "";
        for (const auto& behavior : component["Behavior"]) {
            if (!cmd.empty()) {
                cmd += ",";
            }
            cmd += generateCmd(behavior);
        }
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

std::string ArmControllerServer::verifyMsg(const int armid, const int cmdid, const int vrfres){
    std::string vrfmsg = "";
    json jsonmsg;
    jsonmsg["armid"] = armid;
    jsonmsg["cmdid"] = cmdid;
    jsonmsg["vrfres"] = vrfres;
    vrfmsg = jsonmsg.dump(4);
    return vrfmsg;
}

void ArmControllerServer::updateLocs(std::string cmd, float locs[]){
    std::istringstream stream(cmd);
    std::string token;
    char identifier;
    float value;
    bool isincre = false; //一次只考虑当前这一条指令，简单这么写就行
    while (stream >> token) {
        if (token[0] == 'X' || token[0] == 'Y' || token[0] == 'Z' || 
            token[0] == 'A' || token[0] == 'B' || token[0] == 'C') {
            identifier = token[0];
            int idx = 1; // 开始解析的位置，跳过标识符

            // 处理数值可能直接跟在标识符后面的情况
            if (!isdigit(token[idx]) && token[idx] != '-' && token[idx] != '+') {
                idx++; // 如果标识符后面有空格，则跳过
            }

            value = std::stof(token.substr(idx));

            if(!isincre){
                if (identifier == 'X') {
                    locs[0] = value;
                } else if (identifier == 'Y') {
                    locs[1] = value;
                } else if (identifier == 'Z') {
                    locs[2] = value;
                } else if (identifier == 'A') {
                    locs[3] = value;
                } else if (identifier == 'B') {
                    locs[4] = value;
                } else if (identifier == 'C') {
                    locs[5] = value;
                }
            }else{
                if (identifier == 'X') {
                    locs[0] += value;
                } else if (identifier == 'Y') {
                    locs[1] += value;
                } else if (identifier == 'Z') {
                    locs[2] += value;
                } else if (identifier == 'A') {
                    locs[3] += value;
                } else if (identifier == 'B') {
                    locs[4] += value;
                } else if (identifier == 'C') {
                    locs[5] += value;
                }
            }
        }else if(token == "G91"){
            isincre = true;
        }
    }
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
        file << str << std::endl;
        // 关闭文件
        file.close();
    } else {
        // 文件打开失败，输出错误消息
        std::cerr << "Unable to open file: " << filename << std::endl;
    }
}

// 将单个指令串转换为字符串数组，指定起始索引和长度
std::vector<std::string> decodeCommaStr(const std::string& commands, int startIndex, int length) {
    std::vector<std::string> commandArray;
    std::stringstream ss(commands);
    std::string command;
    int index = 0;

    while (std::getline(ss, command, ',')) {
        if (index >= startIndex && index < startIndex + length) {
            commandArray.push_back(command);
        }
        index++;
    }

    return commandArray;
}

std::string subCommaStr(const std::string& commands, int startIndex, int length) {
    std::string subcmdstr;
    std::stringstream ss(commands);
    std::string command;
    int index = 0;

    while (std::getline(ss, command, ',')) {
        if (index >= startIndex && index < startIndex + length) {
            if(!subcmdstr.empty()){
                subcmdstr += ",";
            }
            subcmdstr += command;
        }
        index++;
    }

    return subcmdstr;
}

// 将字符串数组转换为单个指令串，指定起始索引和长度
std::string encodeCommaStr(std::string commandArray[], int arraysize, int startIndex, int length) {
    std::string commands;
    
    for (int i = startIndex; i < startIndex + length && i < arraysize; ++i) {
        commands += commandArray[i];
        if (i < startIndex + length - 1 && i < arraysize - 1) {
            commands += ","; // 在每个指令后添加逗号，除了最后一个
        }
    }

    return commands;
}


/*由于全部是string类型，最好不去使用，而是手动填写*/
json unordered_map_to_json(const std::unordered_map<std::string, std::string>& umap) {
    json j;
    for (const auto& pair : umap) {
        j[pair.first] = pair.second;
    }
    return j;
}

/*现在感觉自己蠢了，这两用在cJSON上可能有点作用*/
std::unordered_map<std::string, std::string> json_to_unordered_map(const json& j) {
    std::unordered_map<std::string, std::string> umap;
    for (auto it = j.begin(); it != j.end(); ++it) {
        std::string key = it.key(); 
        std::string value = it.value().is_string() ? it.value().get<std::string>() : it.value().dump(); // 将非字符串值转换为字符串
        umap[key] = value;
    }
    return umap;
}

void ArmControllerServer::initializeArmConfigs() {
    // 初始化示例数据
    armConfigs[1] = {
        {"BasePosition", {0, 230, 0}},
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
        
            }
        }
    };
    armConfigs[4] = {
        {"BasePosition", {800, -230, 0}},
        {"Obstacles", 
            {
                
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

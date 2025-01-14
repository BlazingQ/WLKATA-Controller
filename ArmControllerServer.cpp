#include "ArmControllerServer.hpp"

ArmControllerServer::ArmControllerServer() : isincre(false), isangle(false) {
    initializeArmConfigs();
    lastControlTime = timenow();
}

void ArmControllerServer::runServer(int port) {
    overwriteToFile("", "json/control.json");
    overwriteToFile("", "json/status.json");
    overwriteToFile("", "timeused.md");

    int server_fd = setup_server(port);
    if (server_fd < 0) {
        cerr << "Failed to set up server." << endl;
        return;
    }

    while (true) {
        int client_fd = wait_for_connection(server_fd);
        if (client_fd < 0) {
            cerr << "Failed to accept connection." << endl;
            continue;
        }

        char buffer[4096] = {0};
        if (receive_message(client_fd, buffer, 4096)) {
            // auto starttime = timenow();
            string statusstr = buffer;
            if(statusstr.empty() || statusstr[0] == '^' ){
                send_message(client_fd, "");
            }else{
                oneRun(statusstr, client_fd, false);
            }
        }

        close_connection(client_fd);
    }

    close_connection(server_fd);
}

void ArmControllerServer::oneRun(string statusstr, int client_fd, bool istest) {
    auto starttime = timenow();
    json statusjson = json::parse(statusstr);
    //armid here is vrfarm, vrfid is vrfcmd
    int armid = statusjson["ArmId"]; 
    int vrfid = statusjson["VrfId"];
    int isinit = statusjson["IsInit"];
    // string constructtime = statusjson["ConstructTime"];

    //think status constructed before lastControlTime is not secure
    // if(!istest && (stoll(constructtime) <= stoll(lastControlTime))){
    if(false){
        if(!istest)
            send_message(client_fd, verifyMsg(armid, vrfid, -1).c_str());
        else
            cout<< "vrfmsgstr = "<< verifyMsg(armid, vrfid, -1) <<endl;
    } else if(isinit){ //considering time complexity, ignore init verify
        if(!istest)
            send_message(client_fd, verifyMsg(armid, vrfid, 1).c_str());
        else
            cout << "vrfmsgstr = "<< verifyMsg(armid, vrfid, 1) <<endl;

    }else {
        appendToFile("\noringinal status:", "json/status.json");
        appendToFile(statusstr, "json/status.json");
        json arms = preprocessStateJson(statusstr);

        //arms/jsonstr isempty is not considered to occur
        if(!arms.empty()){
            appendToFile("\nprocessed status:", "json/status.json");
            appendToFile(arms.dump(4), "json/status.json");
            string jsonstr = transCmds(arms); // 调用命令处理函数
            // appendToFile("\ninput:", "json/status.json");
            // appendToFile(jsonstr, "json/status.json");
            if(!jsonstr.empty()){
                bool res = verifyMultiArm(jsonstr, armid);
                // bool res = true;
                // std::this_thread::sleep_for(std::chrono::milliseconds(300)); //simulate verify
                string vrfmsgstr = verifyMsg(armid, vrfid, res);
                if(!res){
                    string controljsonstr = arm_control(jsonstr, armid);
                    appendToFile(controljsonstr, "json/control.json");
                    if (!controljsonstr.empty()) {
                        string controlmsgstr = controlMsg(vrfmsgstr, controljsonstr);
                        lastControlTime = timenow();
                        if(!istest)
                            send_message(client_fd, controlmsgstr.c_str());
                        else
                            cout << "controlmsgstr = "<< controlmsgstr<<endl;
                    }else{
                        if(!istest)
                            send_message(client_fd, vrfmsgstr.c_str());
                        else
                            cout << "vrfmsgstr = "<< vrfmsgstr<<endl;
                    }
                } else{
                    if(!istest)
                        send_message(client_fd, vrfmsgstr.c_str());
                    else
                        cout << "vrfmsgstr = "<< vrfmsgstr<<endl;
                }
            }else{
                if(!istest)
                    send_message(client_fd, verifyMsg(armid, vrfid, 0).c_str());
                else
                    cout << "vrfmsgstr = "<< verifyMsg(armid, vrfid, 0) <<endl;
            }
        }else{
            if(!istest)
                send_message(client_fd, verifyMsg(armid, vrfid, 0).c_str());
            else
                cout << "vrfmsgstr = "<< verifyMsg(armid, vrfid, 0) <<endl;
        }
    }
    auto endtime = timenow();
    appendToFile(to_string(stoll(endtime) - stoll(starttime)), "timeused.md");
    return ;
}



/*封装arm_verify，将其验证两个机械臂安全的功能扩展到多机械臂*/
bool ArmControllerServer::verifyMultiArm(const string& jsonStr, int targetArmId) {
    json j = json::parse(jsonStr);
    vector<json> components = j["Components"];
    
    // 找到目标机械臂的index
    int targetIdx = -1;
    for(size_t i = 0; i < components.size(); i++) {
        if(components[i]["ArmId"] == targetArmId) {
            targetIdx = i;
            break;
        }
    }
    
    if(targetIdx == -1) {
        cerr << "Target arm " << targetArmId << " not found" << endl;
        return false;
    }

    // 对目标机械臂与其他ID大于它的机械臂进行验证
    for(size_t i = 0; i < components.size(); i++) {
        if(components[i]["ArmId"] <= targetArmId ){
            if(components[i]["ArmId"] < targetArmId)
                cout<<"no need to compare main arm "<<targetArmId<<" with arm "<<components[i]["ArmId"]<<endl;
            continue;
        }
        cout <<"compare main arm "<<targetArmId<<" with arm "<<components[i]["ArmId"]<<endl;
        // 构造仅包含两个机械臂的json
        json pairJson;
        pairJson["Obstacles"] = j["Obstacles"];
        pairJson["Components"] = json::array();
        pairJson["Components"].push_back(components[targetIdx]);
        pairJson["Components"].push_back(components[i]);
        
        // 使用现有的arm_verify进行验证
        bool res = arm_verify(pairJson.dump());
        if(!res) {
            return false; // 如果任何一对验证失败就返回false
        }
    }
    
    return true; // 所有验证都通过
}

/*为原始status信息进行处理的总体函数，先输出一个格式能对接transCmds，后续可以考虑再结合。
这一步并没有改变输入的json格式，其arm仍然含有armid。
目前的想法是把两个过程分开的好，方便修改，本身还是用现在的格式方便*/
json ArmControllerServer::preprocessStateJson(string statusstr){
    json statusjson = json::parse(statusstr);
    int armid = statusjson["ArmId"];
    int vrfid = statusjson["VrfId"];
    int isinit = statusjson["IsInit"];
    json arms = statusjson["Arms"];
    json retarms = json::array();
    if(isinit){//暂定和之前一样验证前三个指令，目前是所有arm都验证，会有重复验证
        for(json arm: arms){
            string cmds = arm["Cmds"];
            string subcmds = subCommaStr(cmds, 0, 3);
            arm["Cmds"] = subcmds;
            //duration后面暂时用不到,改不改区别不大
            string durations = arm["Durations"];
            string subdurations = subCommaStr(durations, 0, 3);
            arm["Durations"] = subcmds;
            retarms.push_back(arm);
        }
    }else{
        json vrfarm;
        long long int starttimearray[6] = {LLONG_MAX,LLONG_MAX,LLONG_MAX,LLONG_MAX,LLONG_MAX,LLONG_MAX};
        long long int starttimedelay[6] = {LLONG_MAX,LLONG_MAX,LLONG_MAX,LLONG_MAX,LLONG_MAX,LLONG_MAX};
        for(json arm: arms){
            string timestr = arm["StartTime"];
            starttimearray[static_cast<int>(arm["ArmId"])] = stoll(timestr);
            if(arm["ArmId"] == armid){
                vrfarm = arm;
            }
        }
        if(vrfarm.empty()){ //没有找到vrfarm，返回null
            return nullptr;
        }else{
            //计算指令起始的delay方便后续计算，后续只提供相对时间
            for(int i = 1; i < 6; ++i){
                if(starttimearray[i] != LLONG_MAX){//本次验证涉及该机械臂
                    starttimedelay[i] = starttimearray[i] - starttimearray[armid];
                }
            }
            // 获得vrfarm需要获得的一切信息，并生成最终的vrfarmjson
            pair<pair<int, int>, json> vrfarminfo = getVrfArmInfo(vrfarm, vrfid);
            pair<int, int> timepair = vrfarminfo.first;
            json newvrfarm = vrfarminfo.second;

            retarms.push_back(newvrfarm);
            int finalstarttime[6] = {0};
            int minstarttime = INT_MAX;
            finalstarttime[armid] = timepair.first;
            minstarttime = min(minstarttime, timepair.first);

            //第二次遍历期望获得其他的armjson，并且获取足够的信息构造最终的delay
            for(json arm: arms){
                int tempindex = arm["ArmId"];
                if(tempindex != armid){
                    pair<int, json> otherarminfo = getOtherArmInfo(arm, timepair, starttimedelay[tempindex]);
                    if(otherarminfo.first != INT_MAX){
                        finalstarttime[tempindex] = otherarminfo.first;
                        minstarttime = min(minstarttime, otherarminfo.first);
                        retarms.push_back(otherarminfo.second);
                    }
                }
            }
            // 最终delay就是看下最小的starttime,调为0
            for(json& retarm: retarms){
                int tempindex = retarm["ArmId"];
                retarm["delay"] = finalstarttime[tempindex] - minstarttime;
            }

        }

    }
    return retarms;
}

/*获取需要vrfarm的具体的指令构造和指令起止相对时间*/
pair<pair<int, int>, json> ArmControllerServer::getVrfArmInfo(json armjson, int vrfid) {
    
    // 获取 Cmds 和 Durations 字段
    string cmds = armjson["Cmds"];
    string durations = armjson["Durations"];
    string locsstr = armjson["Locs"];
    int startid = armjson["CmdId"];

    /*初始化locs，为后续指令构造做准备*/
    float locallocs[6];
    istringstream locsStream(locsstr);
    string loc;
    int i = 0;
    while (getline(locsStream, loc, ',')) {
        if (!loc.empty()) {
            locallocs[i] = stof(loc);
            i++;
        }
    }

    // 按逗号分割指令和持续时间
    vector<string> cmdList = decodeCommaStr(cmds, 0, INT_MAX);
    vector<int> durationList = commaStrtoInt(durations, 0, INT_MAX);
    assert(cmdList.size() == durationList.size());
    
    string vrfcmd;
    int starttime = 0;
    int duration;
    for(size_t i = 0; i < cmdList.size(); ++i) {
        string cmd = cmdList[i];
        if (!cmd.empty()) {
            if(i + startid < vrfid){
                updateLocs(cmd, locallocs);
                starttime += durationList[i];
            }else if((i + startid) == vrfid){
                vrfcmd = cmd;
                duration = durationList[i];
            }
        }
    }

    json newarmjson;
    newarmjson["ArmId"] = armjson["ArmId"];
    newarmjson["Cmds"] = vrfcmd;
    string locallocsstr;
    for(int i = 0; i < 6; ++i){
        if(!locallocsstr.empty()){
            locallocsstr += ",";
        }
        locallocsstr += to_string(locallocs[i]);
    }
    newarmjson["Locs"] = locallocsstr;
    //不需要starttime，在执行时看的是delay，但是加上方便看效果
    newarmjson["TimePair"] = {starttime, starttime + duration};
    newarmjson["StartId"] = vrfid;

    return {{starttime, starttime + duration}, newarmjson};
}

/*需要注意这里返回的starttime已经是被delay修正过的了，和vrfarm保持一个基准了*/
pair<int, json> ArmControllerServer::getOtherArmInfo(json armjson, pair<int, int> timepair, int delay){
    // 获取 Cmds 和 Durations 字段
    string cmds = armjson["Cmds"];
    string durations = armjson["Durations"];
    string locsstr = armjson["Locs"];
    int startid = armjson["CmdId"];

    /*初始化locs，为后续指令构造做准备*/
    float locallocs[6];
    istringstream locsStream(locsstr);
    string loc;
    int i = 0;
    while (getline(locsStream, loc, ',')) {
        if (!loc.empty()) {
            locallocs[i] = stof(loc);
            i++;
        }
    }

    // 按逗号分割指令和持续时间
    vector<string> cmdList = decodeCommaStr(cmds, 0, INT_MAX);
    vector<int> durationList = commaStrtoInt(durations, 0, INT_MAX);
    assert(cmdList.size() == durationList.size());
    
    string newcmds;
    int starttime = INT_MAX;
    int endtime = INT_MAX; 
    int vrfstart = timepair.first;
    int vrfend = timepair.second;
    int curtime = delay;
    int newstartid = INT_MAX;
    //另外有一种匹配思路是，先求每一条指令的区间，然后写一个算法专门看两个区间是否匹配，这样匹配一定不出错。
    //但是后续看cmds和locs还是一样麻烦。
    if(curtime < vrfend){//否则就是空
        for(size_t i = 0; i < cmdList.size(); ++i) {
            string cmd = cmdList[i];
            int duration = durationList[i];
            if (!cmd.empty()) {
                if(starttime == INT_MAX){//还没找到起点
                    if(curtime + duration <= vrfstart){//还不是起点
                        updateLocs(cmd, locallocs);
                    }else{
                        starttime = curtime;
                        if(!newcmds.empty()){
                            newcmds += ",";
                        }
                        if(newcmds.empty()){
                            newstartid = startid + i;
                        }
                        newcmds += cmd;
                        if(curtime + duration >= vrfend){//目前能想到的边界情况
                            endtime = curtime + duration;
                        }
                    }
                }else if(endtime == INT_MAX){
                    if(!newcmds.empty()){
                        newcmds += ",";
                    }
                    newcmds += cmd;
                    if(curtime + duration >= vrfend){
                        endtime = curtime + duration;
                    }
                }
                curtime += duration;
            }
        }
    }else{//后续不需要参与讨论了，但是不定义此情况为出错。其实真实情况也不会发生
        return {INT_MAX, nullptr};
    }
    //不再需要check endtime有没有，不关注，求endtime只是希望能及时停下来
    json newarmjson;
    newarmjson["ArmId"] = armjson["ArmId"];
    newarmjson["Cmds"] = newcmds;
    string locallocsstr;
    for(int i = 0; i < 6; ++i){
        if(!locallocsstr.empty()){
            locallocsstr += ",";
        }
        locallocsstr += to_string(locallocs[i]);
    }
    newarmjson["Locs"] = locallocsstr;
    //不需要starttime，在执行时看的是delay，但是加上方便看效果
    newarmjson["TimePair"] = {starttime, endtime};
    newarmjson["StartId"] = newstartid;

    return {starttime, newarmjson};
}

/*根据传来的locs信息确定起始位置*/
json ArmControllerServer::parseInitialState(const string& data) {
    istringstream iss(data);
    string value;
    locs.clear();

    while (getline(iss, value, ',')) {
        locs.push_back(stod(value));
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
json ArmControllerServer::parseCommand(const string& cmd) {
    istringstream iss(cmd);
    string part;
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
            size_t pos = part.find_first_not_of("ABCXYZ");
            if (pos != string::npos) {
                switch (part[0])
                {
                case 'X':
                    if(!isincre){
                        locs[0] = stod(part.substr(pos));
                    }
                    else{
                        locs[0] += stod(part.substr(pos));
                    }
                    if(!isangle){
                        behavior["X"].push_back(locs[0]);
                    }else{
                        behavior["Theta"].push_back(locs[0]);
                    }
                    break;
                case 'Y':
                    if(!isincre){
                        locs[1] = stod(part.substr(pos));
                    }
                    else{
                        locs[1] += stod(part.substr(pos));
                    }
                    if(!isangle){
                        behavior["X"].push_back(locs[1]);
                    }else{
                        behavior["Theta"].push_back(locs[1]);
                    }
                    break;
                case 'Z':
                    if(!isincre){
                        locs[2] = stod(part.substr(pos));
                    }
                    else{
                        locs[2] += stod(part.substr(pos));
                    }
                    if(!isangle){
                        behavior["X"].push_back(locs[2]);
                    }else{
                        behavior["Theta"].push_back(locs[2]);
                    }
                    break;
                case 'A':
                    if(!isincre){
                        locs[3] = stod(part.substr(pos));
                    }
                    else{
                        locs[3] += stod(part.substr(pos));
                    }
                    if(!isangle){
                        behavior["R"].push_back(locs[3]);
                    }else{
                        behavior["Theta"].push_back(locs[3]);
                    }
                    break;
                case 'B':
                    if(!isincre){
                        locs[4] = stod(part.substr(pos));
                    }
                    else{
                        locs[4] += stod(part.substr(pos));
                    }
                    if(!isangle){
                        behavior["R"].push_back(locs[4]);
                    }else{
                        behavior["Theta"].push_back(locs[4]);
                    }
                    break;
                case 'C':
                    if(!isincre){
                        locs[5] = stod(part.substr(pos));
                    }
                    else{
                        locs[5] += stod(part.substr(pos));
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
    component["ArmId"] = armindex;
    string initialStateData = singleArm["Locs"];
    string commands = singleArm["Cmds"];
    if (armConfigs.find(armindex) != armConfigs.end()) {
        component["BasePosition"] = armConfigs[armindex]["BasePosition"];
        for (const auto& obstacle : armConfigs[armindex]["Obstacles"]) {
                result["Obstacles"].push_back(obstacle); // 逐个添加
            }
    } else {
        cout << "Invalid arm index." << endl;
    }
    component["InitialState"] = parseInitialState(initialStateData);
    component["Behavior"] = json::array();

    istringstream iss(commands);
    string token;
    json lastCommand;
    bool hasTargetState = false;
    while (getline(iss, token, ',')) {
        if (token.find("M20") != string::npos || token.find("M21") != string::npos) {
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
string ArmControllerServer::transCmds(json arms) {
    
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

string ArmControllerServer::generateCmd(const json& behavior) {
    string cmd = "M20 G90 "; // 假设都是绝对坐标系统
    cmd += behavior["Motion type"] == "Joint" ? "G00 " : "G01 ";

    // 处理 X, Y, Z 坐标
    vector<string> axis = {"X", "Y", "Z"};
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

string ArmControllerServer::verifyMsg(const int armid, const int vrfid, const int vrfres){
    string vrfmsg = "";
    json jsonmsg;
    jsonmsg["ArmId"] = armid;
    jsonmsg["VrfId"] = vrfid;
    if(vrfres){
        jsonmsg["VrfRes"] = 1;
    }
    else{
        jsonmsg["VrfRes"] = 0;
    }
    jsonmsg["Time"] = timenow(); //time used to compare verify sequence, maybe useless now
    jsonmsg["Cmds"] = "";
    vrfmsg = jsonmsg.dump(4);
    return vrfmsg;
}


string ArmControllerServer::controlMsg(const string& vrfjsonstr, const string& controljsonstr){
    string controlmsg = "";
    json jsonmsg = json::parse(vrfjsonstr);
    json controljson = json::parse(controljsonstr);

    string cmds;
    for(const auto& component: controljson["Components"]){
        if(component["ArmId"] == jsonmsg["ArmId"]){
            for(const auto& behavior: component["Behavior"]){
                if(!cmds.empty()){
                    cmds += ",";
                }
                cmds += generateCmd(behavior);
                //test for 2 control cmds
                if(!cmds.empty()){  
                    cmds += ",";
                }
                cmds += generateCmd(behavior);
            }
        }
    }

    //很不严谨的操作，并没有检查对应armid的component是否存在，只是通过cmds是否为空简单判断
    if(!cmds.empty()){
        jsonmsg["Cmds"] = cmds;
    }
    controlmsg = jsonmsg.dump(4);
    return controlmsg;
}

/*根据验证与控制生成的结果，将其重新封装为返回的cmd串。
目前的格式不太美妙，可以继续设计，返回的串的格式很机械，
就是按armId的顺序排的*/
string ArmControllerServer::jsonToCmds(const string& jsonString) {
    json j = json::parse(jsonString);
    string cmdsback = "";
    vector<string> cmds;
    cmds.resize(6);
    for(int i = 0; i < 6; ++i){
        cmds[i] = "";
    }

    for(const auto& component: j["Components"]){
        string cmd = "";
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


void ArmControllerServer::updateLocs(string cmd, float locs[]){
    istringstream stream(cmd);
    string token;
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

            value = stof(token.substr(idx));

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


string doubleToStr(double value) {
    ostringstream oss;
    oss << fixed << setprecision(2) << value;
    return oss.str();
}

void overwriteToFile(const string& str, const string& filename) {
    // 打开文件，使用 ios::out 模式以覆盖方式写入
    ofstream file(filename, ios::out);
    if (file.is_open()) {
        // 写入字符串并添加换行符
        file << str;
        // 关闭文件
        file.close();
    } else {
        // 文件打开失败，输出错误消息
        cerr << "Unable to open file: " << filename << endl;
    }
}

string readFile(const string& filename) {
    // 打开文件
    ifstream file(filename);
    if (!file.is_open()) {
        // 文件打开失败，输出错误消息
        cerr << "Unable to open file: " << filename << endl;
        return "";
    }
    
    // 使用 istreambuf_iterator 读取数据，直到 EOF
    string str((istreambuf_iterator<char>(file)),
                    istreambuf_iterator<char>());
    
    // 关闭文件
    file.close();
    
    // 打印读取的结果
    cout << "Read file: " << filename << endl;
    
    return str;
}

std::string timenow(){
    auto now = chrono::high_resolution_clock::now();
    auto duration_since_epoch = now.time_since_epoch();
    long long int millis = chrono::duration_cast<chrono::milliseconds>(duration_since_epoch).count();
    return to_string(millis);
}

void appendToFile(const string& str, const string& filename) {
    // 打开文件，使用 ios::app 模式以追加方式写入
    ofstream file(filename, ios::app);
    if (file.is_open()) {
        // 写入字符串并添加换行符
        file << str << endl;
        // 关闭文件
        file.close();
    } else {
        // 文件打开失败，输出错误消息
        cerr << "Unable to open file: " << filename << endl;
    }
}

// 将单个指令串转换为字符串数组，指定起始索引和长度
vector<string> decodeCommaStr(const string& commands, int startIndex, int length) {
    vector<string> commandArray;
    stringstream ss(commands);
    string command;
    int index = 0;

    while (getline(ss, command, ',')) {
        if (index >= startIndex && index < startIndex + length) {
            commandArray.push_back(command);
        }
        index++;
    }

    return commandArray;
}

vector<int> commaStrtoInt(const string& commands, int startIndex, int length) {
    vector<int> commandArray;
    stringstream ss(commands);
    string command;
    int index = 0;

    while (getline(ss, command, ',')) {
        if (index >= startIndex && index < startIndex + length) {
            commandArray.push_back(stoi(command));
        }
        index++;
    }

    return commandArray;
}

string subCommaStr(const string& commands, int startIndex, int length) {
    string subcmdstr;
    stringstream ss(commands);
    string command;
    int index = 0;

    while (getline(ss, command, ',')) {
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
string encodeCommaStr(string commandArray[], int arraysize, int startIndex, int length) {
    string commands;
    
    for (int i = startIndex; i < startIndex + length && i < arraysize; ++i) {
        commands += commandArray[i];
        if (i < startIndex + length - 1 && i < arraysize - 1) {
            commands += ","; // 在每个指令后添加逗号，除了最后一个
        }
    }

    return commands;
}


/*由于全部是string类型，最好不去使用，而是手动填写*/
json unordered_map_to_json(const unordered_map<string, string>& umap) {
    json j;
    for (const auto& pair : umap) {
        j[pair.first] = pair.second;
    }
    return j;
}

/*现在感觉自己蠢了，这两用在cJSON上可能有点作用*/
unordered_map<string, string> json_to_unordered_map(const json& j) {
    unordered_map<string, string> umap;
    for (auto it = j.begin(); it != j.end(); ++it) {
        string key = it.key(); 
        string value = it.value().is_string() ? it.value().get<string>() : it.value().dump(); // 将非字符串值转换为字符串
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
               
        
            }
        }
    };
    armConfigs[2] = {
        {"BasePosition", {420, 230, 0}},
        {"Obstacles", 
            {
                
        
            }
        }
    };
    armConfigs[3] = {
        {"BasePosition", {800, 210, 0}},
        {"Obstacles", 
            {
        
            }
        }
    };
    armConfigs[4] = {
        {"BasePosition", {800, -210, 0}},
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
               
            
            }
        }
    };
}


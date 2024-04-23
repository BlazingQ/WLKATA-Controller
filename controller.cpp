#include "tcpdocker.h"
#include "controller.hpp"

using namespace std;

int main(){
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
            // cout<<endl<<arm_verify(jsonstr)<<endl;
            if(!arm_verify(jsonstr)){
                string controljsonstr;
                cout<<endl<<arm_control(controljsonstr)<<endl;
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
    std::vector<float> locs;

    while (getline(iss, value, ',')) {
        locs.push_back(std::stof(value));
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
    behavior["X"] = json::array();
    behavior["R"] = json::array();

    while (iss >> part) {
        if (part[0] == 'M') {
            behavior["Mode"] = (part == "M20") ? "Cartesian" : "Angel";
        } else if (part[0] == 'G') {
            if (part == "G00") {
                behavior["Motion type"] = "Joint";
            } else if (part == "G01") {
                behavior["Motion type"] = "Linear";
            }
        } else if (part[0] == 'X' || part[0] == 'Y' || part[0] == 'Z') {
            std::size_t pos = part.find_first_not_of("XYZ");
            if (pos != std::string::npos) {
                behavior["X"].push_back(std::stod(part.substr(pos)));
            }
        } else if (part[0] == 'A' || part[0] == 'B' || part[0] == 'C') {
            std::size_t pos = part.find_first_not_of("ABC");
            if (pos != std::string::npos) {
                behavior["R"].push_back(std::stod(part.substr(pos)));
            }
        }
    }

    // Speed is fixed as 2000
    behavior["Speed"] = 2000;

    return behavior;
}

// 主函数，处理整个指令字符串
std::string transCmd(const std::string& packet) {
    std::size_t delimiterPos = packet.find(';');
    std::string initialStateData = packet.substr(0, delimiterPos);
    std::string commands = packet.substr(delimiterPos + 1);

    
    json result;
    result["Obstacles"] = json::array({ {{"Radius", 10.0},{"X", {-10, 198, 155}}} });
    result["Components"] = json::array({
        {
            {"BasePosition", {0, 0, 0}},
            {"InitialState", parseInitialState(initialStateData)},
            {"Behavior", json::array()}
        }
    });

    std::istringstream iss(commands);
    std::string token;

    while (getline(iss, token, ',')) {
        if (token.find("M20") != std::string::npos || token.find("M21") != std::string::npos) {
            json behavior = parseCommand(token);
            result["Components"][0]["Behavior"].push_back(behavior);
        }
    }

    return result.dump(4);  // 输出格式化的 JSON 字符串
}

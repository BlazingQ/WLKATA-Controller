#include "tcpdocker.h"
#include "arm_converter/armVerifier.hpp"

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
            strcat(buffer, "send back");
            send_message(client_fd, buffer);
            
        }
        close_connection(client_fd);
    }
    close_connection(server_fd);

    return 0;
}
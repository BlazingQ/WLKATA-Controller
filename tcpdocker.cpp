#include "tcpdocker.h"



int setup_server(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        std::cerr << "Failed to create socket." << std::endl;
        return -1;
    }

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed." << std::endl;
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed." << std::endl;
        close(server_fd);
        return -1;
    }

    return server_fd;
}

int wait_for_connection(int server_fd) {
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    int new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
    if (new_socket < 0) {
        std::cerr << "Accept failed." << std::endl;
        return -1;
    }
    return new_socket;
}

bool send_message(int sock, const char* message) {
    if (send(sock, message, strlen(message), 0) < 0) {
        std::cerr << "Failed to send message." << std::endl;
        return false;
    }
    std::cout << "Message sent successfully." << std::endl;
    return true;
}

bool receive_message(int sock, char* buffer, size_t buffer_size) {
    ssize_t bytes_received = recv(sock, buffer, buffer_size, 0);
    if (bytes_received < 0) {
        std::cerr << "Error in receiving data." << std::endl;
        return false;
    } else if (bytes_received == 0) {
        std::cerr << "Connection closed by the peer." << std::endl;
        return false;
    }
    buffer[bytes_received] = '\0'; // Ensure null-termination
    std::cout << "Received: " << buffer << std::endl;
    return true;
}

 void close_connection(int sock) {
    close(sock);
    std::cout << "Connection closed." << std::endl;
}

// void listen(const char* dstIP, int dstPort) {
//     int server_fd, new_socket;
//     struct sockaddr_in address;
//     int opt = 1;
//     int addrlen = sizeof(address);
      
//     // 创建socket文件描述符
//     if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
//         perror("socket failed");
//         exit(EXIT_FAILURE);
//     }
      
//     // 附加socket到指定IP / 端口
//     if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
//         perror("setsockopt");
//         exit(EXIT_FAILURE);
//     }
//     address.sin_family = AF_INET;
//     address.sin_addr.s_addr = INADDR_ANY;
//     address.sin_port = htons(dstPort);
      
//     // 绑定socket到IP / 端口
//     if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
//         perror("bind failed");
//         exit(EXIT_FAILURE);
//     }
//     printf("\nlistening...\n");
//     if (listen(server_fd, 3) < 0) {
//         perror("listen");
//         exit(EXIT_FAILURE);
//     }
//     if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
//         perror("accept");
//         exit(EXIT_FAILURE);
//     }

//     // 接受和处理客户端消息
//     char buffer[1024] = {0};
//     read(new_socket, buffer, 1024);
//     std::cout << "Message from client: " << buffer << std::endl;
    
//     // 关闭socket
//     close(server_fd);
// }

// void send(const char* dstIP, int dstPort, char* message) {
//     struct sockaddr_in serv_addr;
//     int sock = 0;
    
//     if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
//         printf("\n Socket creation error \n");
//         return;
//     }
  
//     serv_addr.sin_family = AF_INET;
//     serv_addr.sin_port = htons(dstPort);
      
//     // 将IPv4和IPv6地址从文本转换为二进制形式
//     if(inet_pton(AF_INET, dstIP, &serv_addr.sin_addr)<=0) {
//         printf("\nInvalid address/ Address not supported \n");
//         return;
//     }
  
//     if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
//         printf("\nConnection Failed \n");
//         return;
//     }
//     send(sock, message, strlen(message), 0);
//     std::cout << "Message sent\n";
    
//     // 关闭socket
//     close(sock);
// }

// void listen_and_respond(const char* serverIP, int serverPort) {
//     int server_fd, new_socket;
//     struct sockaddr_in address;
//     socklen_t addrlen = sizeof(address);
    
//     // 创建socket文件描述符
//     if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
//         perror("socket failed");
//         exit(EXIT_FAILURE);
//     }
    
//     // 允许地址重用
//     int opt = 1;
//     if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
//         perror("setsockopt");
//         exit(EXIT_FAILURE);
//     }
    
//     // 设置服务器地址信息
//     address.sin_family = AF_INET;
//     address.sin_addr.s_addr = INADDR_ANY;
//     address.sin_port = htons(serverPort);
    
//     // 绑定socket
//     if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
//         perror("bind failed");
//         exit(EXIT_FAILURE);
//     }
    
//     // 开始监听
//     if (listen(server_fd, 3) < 0) {
//         perror("listen");
//         exit(EXIT_FAILURE);
//     }
    
//     std::cout << "Server is listening on " << serverIP << ":" << serverPort << "\n";
    
//     while (true) { // 无限循环监听
//         addrlen = sizeof(address);
//         if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
//             perror("accept");
//             exit(EXIT_FAILURE);
//         }
        
//         // 接收消息
//         char buffer[1024] = {0};
//         int valread = read(new_socket, buffer, 1024);
//         if (valread > 0) {
//             std::string message(buffer);
            
//             // 调用验证接口
//             std::string response = call_bach(message);
            
//             // 发送响应
//             send(new_socket, serverPort, response.c_str(), );
            
//             std::cout << "Message sent: " << response << "\n";
//         }
        
//         // 关闭连接
//         close(new_socket);
//     }
    
//     // 关闭服务器socket
//     close(server_fd);
// }

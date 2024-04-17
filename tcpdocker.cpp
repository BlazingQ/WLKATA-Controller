#include "tcpdocker.h"

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

void listen(const char* dstIP, int dstPort) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
      
    // 创建socket文件描述符
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
      
    // 附加socket到指定IP / 端口
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(dstPort);
      
    // 绑定socket到IP / 端口
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("\nlistening...\n");
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // 接受和处理客户端消息
    char buffer[1024] = {0};
    read(new_socket, buffer, 1024);
    std::cout << "Message from client: " << buffer << std::endl;
    
    // 关闭socket
    close(server_fd);
}

void send(const char* dstIP, int dstPort, char* message) {
    struct sockaddr_in serv_addr;
    int sock = 0;
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return;
    }
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(dstPort);
      
    // 将IPv4和IPv6地址从文本转换为二进制形式
    if(inet_pton(AF_INET, dstIP, &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        return;
    }
  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return;
    }
    send(sock, message, strlen(message), 0);
    std::cout << "Message sent\n";
    
    // 关闭socket
    close(sock);
}

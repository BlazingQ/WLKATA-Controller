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
    appendToFile2("\nmessage:", "json/status.json");
    appendToFile2(std::string(message), "json/status.json");
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
    // std::cout << "Received: " << buffer << std::endl;
    return true;
}

 void close_connection(int sock) {
    close(sock);
    // std::cout << "Connection closed." << std::endl;
}

//专门为了这个cpp文件写的函数
void appendToFile2(const string& str, const string& filename) {
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
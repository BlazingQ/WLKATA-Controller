#ifndef TCPP_I_H
#define TCPP_I_H

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

// void listen(const char* dstIP, int dstPort);
// void send(const char* dstIP, int dstPort, char* message);
int setup_server(int port);
int wait_for_connection(int server_fd);
bool send_message(int sock, char* message);
bool receive_message(int sock, char* buffer, size_t buffer_size);
void close_connection(int sock);

#endif
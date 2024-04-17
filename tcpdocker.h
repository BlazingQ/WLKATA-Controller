#ifndef TCPP_I_H
#define TCPP_I_H

void listen(const char* dstIP, int dstPort);
void send(const char* dstIP, int dstPort, char* message);

#endif
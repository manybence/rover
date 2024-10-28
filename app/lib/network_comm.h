
// network_comm.h
#pragma once
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <vector>
#include <cstdint>
#include <string>

const std::string SERVER_IP = "192.168.68.154";
const int SERVER_PORT = 6556;
const int ARRAY_SIZE = 7168;

int setupSocket();
void sendData(int sockfd, const std::vector<int16_t> &data);
void closeSocket(int sockfd);

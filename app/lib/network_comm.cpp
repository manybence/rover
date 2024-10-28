// network_comm.cpp
#include "network_comm.h"

int setupSocket()
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Cannot create socket!");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

void sendData(int sockfd, const std::vector<int16_t> &data)
{
    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP.c_str(), &server_addr.sin_addr);

    sendto(sockfd, data.data(), data.size() * sizeof(int16_t), 0,
           (struct sockaddr *)&server_addr, sizeof(server_addr));
}

void closeSocket(int sockfd)
{
    close(sockfd);
}

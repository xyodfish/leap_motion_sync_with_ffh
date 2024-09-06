#ifndef __UDP_CLIENT_H__
#define __UDP_CLIENT_H__

#include <stdlib.h>
#include <sys/types.h>
#include <string>

#ifdef IS_WINDOWS_OS
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

class UdpClient {
   public:
    explicit UdpClient(const std::string& _ip, const int& _port);
    ~UdpClient();
    int _connect();
    int _disconnect();

    int sendCmd(const char* data, size_t dataLen);
    int recieveData(char* data, size_t dataLen);

    int getFd() { return sockfd_; }

   private:
    struct sockaddr_in server_addr;
    socklen_t server_addr_len_;

    int sockfd_;
    std::string ip_;
    int port_;

    bool inConnection_ = false;
};

#endif
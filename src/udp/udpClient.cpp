#include "udpClient.h"

UdpClient::UdpClient(const std::string& _ip, const int& _port) : ip_(_ip), port_(_port) {}

UdpClient::~UdpClient() {
    if (inConnection_)
        _disconnect();
}

int UdpClient::_disconnect() {
#ifdef IS_WINDOWS_OS
    closesocket(sockfd_);
#else
    close(sockfd_);
#endif
    inConnection_ = false;
    return 0;
}

int UdpClient::_connect() {
    // Creating socket file descriptor
    if ((sockfd_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
#ifdef IS_WINDOWS_OS
        closesocket(sockfd_);
#else
        close(sockfd_);
#endif
        printf("Error creating socket.\n");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));

    // Filling server information
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(port_);
    server_addr.sin_addr.s_addr = inet_addr(ip_.c_str());

    server_addr_len_ = sizeof(server_addr);

    inConnection_ = true;

    return 0;
}

int UdpClient::sendCmd(const char* data, size_t dataLen) {
#ifdef IS_WINDOWS_OS
    if (sendto(sockfd_, data, dataLen, 0, (const struct sockaddr*)(&server_addr), sizeof(struct sockaddr_in)) < 0)
#else
    if (sendto(sockfd_, data, dataLen, MSG_CONFIRM, (const struct sockaddr*)(&server_addr),
               sizeof(struct sockaddr_in)) < 0)
#endif
    {
        printf("Unable to send message\n");
        return -1;
    }

    return 0;
}

int UdpClient::recieveData(char* data, size_t dataLen) {
    // return recvfrom(sockfd_, data, dataLen, MSG_WAITALL, (struct sockaddr*)&server_addr, &server_addr_len_);
    return recvfrom(sockfd_, data, dataLen, MSG_WAITALL, nullptr, nullptr);
}

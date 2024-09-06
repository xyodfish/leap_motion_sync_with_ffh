#include "ffhCtrl.h"

FfhCtrl::FfhCtrl(const int& _index, const std::string& _ip, const int& _port)
    : udp_(std::make_shared<UdpClient>(_ip, _port)), index_(_index) {}

FfhCtrl::FfhCtrl(const FfhUdpData& _data) : udp_(std::make_shared<UdpClient>(_data.ip, _data.port)), index_(_data.id) {}

FfhCtrl::~FfhCtrl() {
    if (inConnection_) {
        _disconnect();
    }
}

AR_RETURN_VALUE FfhCtrl::_disconnect() {

    if (udp_->_disconnect() < 0) {
        std::cout << "udp disconnect error" << std::endl;
        return AR_RETURN_VALUE::ACTION_FAIL;
    }

    inConnection_ = false;
    return AR_RETURN_VALUE::SUCCESS;
}

int FfhCtrl::getId() const {
    return index_;
}

AR_RETURN_VALUE FfhCtrl::_connect() {
    if (udp_->_connect() < 0) {
        std::cout << "udp connect error" << std::endl;
        return AR_RETURN_VALUE::ACTION_FAIL;
    }

    inConnection_ = true;

    return AR_RETURN_VALUE::SUCCESS;
}

void FfhCtrl::dataProcess() {

    for (auto i = 0; i < 5; ++i) {
        for (auto j = 0; j < 3; ++j) {
            if (handCmd_.finger[i].angle[j] < LIMIT_HCMD_LOWER[j]) {
                handCmd_.finger[i].angle[j] = LIMIT_HCMD_LOWER[j];
            }

            if (handCmd_.finger[i].angle[j] > LIMIT_HCMD_UPPER[j]) {
                handCmd_.finger[i].angle[j] = LIMIT_HCMD_UPPER[j];
            }
        }
    }
}

int FfhCtrl::send_hand_cmd() {
    return udp_->sendCmd((const char*)&handCmd_, sizeof(udp_hand_cmd));
}

int FfhCtrl::send_hand_cmd(udp_hand_cmd cmd) {
    set_hand_cmd(cmd);
    return send_hand_cmd();
}

int FfhCtrl::recieve_hand_data() {
    return udp_->recieveData((char*)&handData_, sizeof(udp_hand_data));
}

udp_hand_data FfhCtrl::get_hand_data() {
    if (udp_->recieveData((char*)&handData_, sizeof(udp_hand_data)) < 0) {
        printf("receive data errror\n");
        return {};
    }
    return handData_;
}
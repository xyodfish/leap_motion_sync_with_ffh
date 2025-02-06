#include "ffhCtrl.h"
#include "spdlog/spdlog.h"

FfhCtrl::FfhCtrl(const int& _index, const std::string& _ip, const int& _port)
    : udp_(std::make_shared<UdpClient>(_ip, _port)), index_(_index) {}

FfhCtrl::FfhCtrl(const FfhUdpData& _data) : udp_(std::make_shared<UdpClient>(_data.ip, _data.port)), index_(_data.id) {}

FfhCtrl::~FfhCtrl() {
    if (inConnection_) {
        reset();
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

void FfhCtrl::setDefaultCmdLimit() {
    lowerLimits_.resize(fingerNum_);
    upperLimits_.resize(fingerNum_);

    for (auto i = 0; i < fingerNum_; ++i) {
        lowerLimits_[i] = {0.0f, 0.0f, -10.f};
        upperLimits_[i] = {90.0f, 90.0f, 10.0f};
    }
}

AR_RETURN_VALUE FfhCtrl::_connect() {
    if (udp_->_connect() < 0) {
        std::cout << "udp connect error" << std::endl;
        return AR_RETURN_VALUE::ACTION_FAIL;
    }

    inConnection_ = true;
    setDefaultCmdLimit();
    reset();
    return AR_RETURN_VALUE::SUCCESS;
}

void FfhCtrl::reset() {
    memset(&handCmd_, 0, sizeof(udp_hand_cmd));
    send_hand_cmd();
}

AR_RETURN_VALUE FfhCtrl::testFlow() {
    if (!inConnection_) {
        spdlog::error("The hand has not been connected");
        return AR_RETURN_VALUE::ACTION_FAIL;
    }

    std::vector<std::string> strs(3);
    bool resetFlag = false;
    while (true) {

        std::cout << "请按顺序输入想要控制的手指id、关节id和角度, 如果想退出 请输入 <\033[32m"
                     "quit"
                  << "\033[0m>" << ", 如果想复位，请输入<\033[32m" << "reset" << "\033[0m>" << std::endl;

        for (auto& str : strs) {
            std::cin >> str;
            if (str == "quit") {
                std::cout << "当前测试已经退出!!!" << std::endl;
                return AR_RETURN_VALUE::SUCCESS;
            }

            if (str == "reset") {
                reset();
                resetFlag = true;
                std::cout << "当前测试已经复位" << std::endl;
                break;
            }
        }

        if (resetFlag) {
            resetFlag = false;
            continue;
        }

        std::cout << "测试手指id[" << "\033[32m" << strs[0] << "\033[0m], " << "关节id[\033[32m" << strs[1]
                  << "\033[0m]"
                  << ", 角度[\033[32m" << strs[2] << "\033[0m]" << std::endl;

        // auto fingerId = std::stoi(strs[0]);
        // auto jointId  = std::stoi(strs[1]);
        // auto angle    = std::stof(strs[2]);

        // if (fingerId > 4 || jointId > 2 || angle > std::fabs(lowerLimits_[fingerId][2])) {
        //     std::cout << "输入有误，请重新输入" << std::endl;
        //     continue;
        // }

        handCmd_.finger[std::stoi(strs[0])].angle[std::stoi(strs[1])] = std::stof(strs[2]);
        send_hand_cmd();
    }

    return AR_RETURN_VALUE::SUCCESS;
}

void FfhCtrl::dataProcess() {

    const size_t fingerNum = 5;
    const size_t jointNum  = sizeof(udp_finger_cmd::angle) / sizeof(float);

    for (auto i = 0; i < fingerNum; ++i) {
        for (auto j = 0; j < jointNum; ++j) {
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

void FfhCtrl::setCmdLimit(const std::vector<std::vector<float>>& lowerLimit,
                          const std::vector<std::vector<float>>& upperLimit) {
    lowerLimits_ = lowerLimit;
    upperLimits_ = upperLimit;
}
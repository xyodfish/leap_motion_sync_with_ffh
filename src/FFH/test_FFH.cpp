#include <exception>
#include "arEnumUtils.h"
#include "ffhCtrl.h"
#include "helper_functions.h"
#include "spdlog/spdlog.h"
#include "yaml-cpp/yaml.h"

using namespace ar::Types;
using namespace std::chrono_literals;

using handPtr = std::shared_ptr<FfhCtrl>;

std::vector<int> ffhValid;
handPtr ffh;
std::string g_taskName;
std::string g_config;
std::vector<std::vector<float>> g_selfCheckAngles1, g_selfCheckAngles2;

/**
 * 解析配置文件并初始化 FfhCtrl 对象
 * 
 * @param config 配置文件的路径
 * 
 * @throws std::runtime_error 如果配置文件加载失败或配置信息无效
 */

void parseConfig(const std::string& config) {

    try {
        auto node = YAML::LoadFile(config);

        ffhValid           = node["hand_Valid"].as<std::vector<int>>();
        auto ffhInfo       = node["hand_info"];
        auto handNode      = ffhInfo[EnumUtils::intToString<size_t, FFHEnumClass>(0)];
        g_taskName         = node["task_name"].as<std::string>();
        g_selfCheckAngles1 = node["self_check_angles1"].as<std::vector<std::vector<float>>>();
        g_selfCheckAngles2 = node["self_check_angles2"].as<std::vector<std::vector<float>>>();

        if (ffhValid[0]) {
            ffh =
                std::make_shared<FfhCtrl>(FfhUdpData{0, handNode["ip"].as<std::string>(), handNode["port"].as<int>()});
        }
    } catch (...) {
        throw std::runtime_error("The config file is invalid, please check the config file.");
    }
}

static void singleJointTest() {
    std::string operCmd;

    while (true) {
        std::cout << "退出测试请输入 <\033[32m"
                  << "quit" << "\033[0m>"
                  << ", 进入测试请输入 <\033[32m" << "test" << "\033[0m>," << " 手指复位请输入 <\033[32m" << "reset"
                  << "\033[0m>" << std::endl;

        std::cin >> operCmd;

        if (operCmd == "test") {
            ffh->testFlow();
        } else if (operCmd == "quit") {
            ffh->reset();
            break;
        } else {
            std::cout << "操作字输入无效，请重新输入！！！！" << std::endl;
        }
    }
}

static void sendFingerCommands(int id, const std::vector<std::vector<float>>& angles, udp_hand_cmd& cmd) {

    for (auto j = 0; j < 3; ++j) {
        cmd.finger[id].angle[j] = angles[id][j];
        ffh->send_hand_cmd(cmd);
        std::this_thread::sleep_for(0.75s);

        if (j != 2) {
            cmd.finger[id].angle[j] = 0;
        }

        ffh->send_hand_cmd(cmd);
        std::this_thread::sleep_for(0.75s);
    }
}

static void selfCheck() {
    std::string str;
    udp_hand_cmd handCmd;

    while (true) {

        std::cout << "灵巧手即将进入自检模式，输入<\033[32m"
                  << "test" << "\033[0m>"
                  << ", 进入自检, 输入<\033[32m" << "quit" << "\033[0m>," << " 退出自检" << std::endl;

        std::cin >> str;

        if (str == "test") {
            ffh->reset();
            std::this_thread::sleep_for(0.5s);

            memset(&handCmd, 0, sizeof(udp_hand_cmd));
            for (auto i = 0; i < 5; ++i) {
                sendFingerCommands(i, g_selfCheckAngles1, handCmd);
            }
            std::this_thread::sleep_for(0.5s);

            ffh->reset();
            std::this_thread::sleep_for(1s);

            memset(&handCmd, 0, sizeof(udp_hand_cmd));
            for (auto i = 0; i < 5; ++i) {
                sendFingerCommands(4 - i, g_selfCheckAngles2, handCmd);
            }
            std::this_thread::sleep_for(0.5s);

            ffh->reset();
            std::this_thread::sleep_for(1s);

        } else if (str != "quit") {
            std::cout << "输入了无效指令，请检查后重新输入！！！" << std::endl;
        } else {
            return;
        }
    }
}

int main() {
    std::string g_config = "./leap_motion_demo_config.yaml";
    parseConfig(g_config);

    if (ffh->_connect() != AR_RETURN_VALUE::SUCCESS) {
        spdlog::error("connect failed, please check the hand connection");
    }

    if (g_taskName == "test_by_hand") {
        singleJointTest();
    } else if (g_taskName == "self_check") {
        selfCheck();
    } else {
        std::cout << "配置文件中的任务名称无效，请检查配置文件" << std::endl;
    }

    return 1;
}
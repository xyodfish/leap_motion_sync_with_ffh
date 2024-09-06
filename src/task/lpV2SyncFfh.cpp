#include "lpV2SyncFfh.h"
#include "arEnumUtils.h"
#include "helper_functions.h"
#include "leapMotionV2Math.h"
#include "spdlog/spdlog.h"
#include "yaml-cpp/yaml.h"
namespace ar::Hardware::LeapMotion {
    LpV2SyncFFH::LpV2SyncFFH(const std::string& config)
        : ffhs_(handTotalNum), ffhValid(std::vector<int>(handTotalNum, 0)), config_(config) {
        parseConfig(config_);
    }

    LpV2SyncFFH::~LpV2SyncFFH() {
        if (inConnection_) {
            disconnect();
        }
    }

    AR_RETURN_VALUE LpV2SyncFFH::disconnect() {
        if (lmWrap.inConnection()) {
            if (lmWrap.disconnect() != AR_RETURN_VALUE::SUCCESS) {
                return AR_RETURN_VALUE::ACTION_FAIL;
            }
        }

        auto retval = disconnectFFH();
        if (retval != AR_RETURN_VALUE::SUCCESS) {
            spdlog::error("ffh disconnect fail.");
            return retval;
        }

        inConnection_ = false;
        return AR_RETURN_VALUE::SUCCESS;
    }

    AR_RETURN_VALUE LpV2SyncFFH::parseConfig(const std::string& config) {

        auto node = YAML::LoadFile(config);

        ffhValid       = node["hand_Valid"].as<std::vector<int>>();
        auto ffhInfo   = node["hand_info"];
        auto handNode  = ffhInfo[EnumUtils::intToString<size_t, FFHEnumClass>(0)];
        taskName_      = node["task_name"].as<std::string>();
        selfCheckData1 = node["self_check_angles1"].as<std::vector<std::vector<float>>>();
        selfCheckData2 = node["self_check_angles2"].as<std::vector<std::vector<float>>>();

        for (auto i = 0; i < handTotalNum; ++i) {
            if (!ffhValid[i])
                continue;

            auto handNode = ffhInfo[EnumUtils::intToString<size_t, FFHEnumClass>(i)];
            ffhs_[i] =
                std::make_shared<FfhCtrl>(FfhUdpData{i, handNode["ip"].as<std::string>(), handNode["port"].as<int>()});

            lowerLimits_ = handNode["lower_limits"].as<std::vector<std::vector<float>>>();
            upperLimits_ = handNode["upper_limits"].as<std::vector<std::vector<float>>>();

            ffhs_[i]->setCmdLimit(lowerLimits_, upperLimits_);
        }

        return AR_RETURN_VALUE::SUCCESS;
    }

    void LpV2SyncFFH::initialize() {
        lmWrap.initialize();
    }

    bool LpV2SyncFFH::inConnection() const {
        return inConnection_;
    }

    AR_RETURN_VALUE LpV2SyncFFH::connectFFH() {

        for (auto i = 0; i < handTotalNum; ++i) {
            if (!ffhValid[i])
                continue;

            if (ffhs_[i]->_connect() != AR_RETURN_VALUE::SUCCESS) {
                spdlog::error("Cannot connect to {} device.",
                              EnumUtils::intToString<int, FFHEnumClass>(ffhs_[i]->getId()));
                return AR_RETURN_VALUE::ACTION_FAIL;
            }
        }

        return AR_RETURN_VALUE::SUCCESS;
    }

    AR_RETURN_VALUE LpV2SyncFFH::disconnectFFH() {
        for (auto i = 0; i < handTotalNum; ++i) {
            if (!ffhValid[i])
                continue;

            if (ffhs_[i]->_disconnect() != AR_RETURN_VALUE::SUCCESS) {
                spdlog::error("Cannot disconnect {} device.",
                              EnumUtils::intToString<int, FFHEnumClass>(ffhs_[i]->getId()));
                return AR_RETURN_VALUE::ACTION_FAIL;
            }
        }

        return AR_RETURN_VALUE::SUCCESS;
    }

    AR_RETURN_VALUE LpV2SyncFFH::connect() {

        if (taskName_ == "leap_motion_demo") {
            if (lmWrap.connect() != AR_RETURN_VALUE::SUCCESS) {
                spdlog::error("Cannot connect to Leap Motion.");
                return AR_RETURN_VALUE::ACTION_FAIL;
            }
        }

        auto retval = connectFFH();
        if (retval != AR_RETURN_VALUE::SUCCESS) {
            spdlog::error("Cannot connect to FFH device.");
            return retval;
        }

        inConnection_ = true;

        return AR_RETURN_VALUE::SUCCESS;
    }

    AR_RETURN_VALUE LpV2SyncFFH::tryConnect() {

        while (true) {

            if (connect() == AR_RETURN_VALUE::SUCCESS) {
                spdlog::info("connection success");
                return AR_RETURN_VALUE::SUCCESS;
            }

            std::this_thread::sleep_for(std::chrono::duration<double>(tryConnectTime_));
        }

        spdlog::info("you have stop the process by enter the key");
        return AR_RETURN_VALUE::ACTION_FAIL;
    }

    void LpV2SyncFFH::singleJointTest() {
        std::string operCmd;

        while (true) {
            std::cout << "退出测试请输入 <\033[32m"
                      << "quit" << "\033[0m>"
                      << ", 进入测试请输入 <\033[32m" << "test" << "\033[0m>," << " 手指复位请输入 <\033[32m" << "reset"
                      << "\033[0m>" << std::endl;

            std::cin >> operCmd;

            if (operCmd == "test") {
                ffhs_.front()->testFlow();
            } else if (operCmd == "quit") {
                ffhs_.front()->reset();
                break;
            } else {
                std::cout << "操作字输入无效，请重新输入！！！！" << std::endl;
            }
        }
    }

    void LpV2SyncFFH::sendFingerCommands(int id, const std::vector<std::vector<float>>& angles,
                                         const std::shared_ptr<FfhCtrl>& ffh, udp_hand_cmd& cmd) {

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

    void LpV2SyncFFH::selfCheck() {
        std::string str;
        udp_hand_cmd tmp_cmd;

        auto ffh = ffhs_.front();

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
                    sendFingerCommands(i, selfCheckData1, ffh, tmp_cmd);
                }
                std::this_thread::sleep_for(0.5s);

                ffh->reset();
                std::this_thread::sleep_for(1s);

                memset(&handCmd, 0, sizeof(udp_hand_cmd));
                for (auto i = 0; i < 5; ++i) {
                    sendFingerCommands(4 - i, selfCheckData2, ffh, tmp_cmd);
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

    void LpV2SyncFFH::start() {

        if (taskName_ == "test_by_hand") {
            singleJointTest();
        } else if (taskName_ == "self_check") {
            selfCheck();
        } else if (taskName_ == "leap_motion_demo") {
            Runnable::start();
            initialize();

            mThreads.push_back(std::thread(&LpV2SyncFFH::monitorDaemon, this));
            mThreads.push_back(std::thread(&LpV2SyncFFH::executeLoop, this));

            while (isRunning()) {
                std::this_thread::sleep_for(10ms);
            }

            for (auto& th : mThreads) {
                if (th.joinable())
                    th.join();
            }

            mThreads.clear();
        }
    }

    void LpV2SyncFFH::stop() {
        Runnable::stop();
        lmWrap.disconnect();
        spdlog::info("Stop the daemon...");
    }

    void LpV2SyncFFH::postProcessHandCmd() {
        for (auto i = 0; i < fingerNum_; ++i) {
            for (auto j = 0; j < angleNum_; ++j) {
                if (handCmd.finger[i].angle[j] < lowerLimits_[i][j]) {
                    handCmd.finger[i].angle[j] = lowerLimits_[i][j];
                }

                if (handCmd.finger[i].angle[j] > upperLimits_[i][j]) {
                    handCmd.finger[i].angle[j] = upperLimits_[i][j];
                }
            }
        }

        /// 特殊限位
        /// 大拇指为食指让路
        if (handCmd.finger[1].angle[0] > 20) {
            if (handCmd.finger[0].angle[0] > 5) {
                handCmd.finger[0].angle[0] = 5;
            }

            if (handCmd.finger[0].angle[1] > 5) {
                handCmd.finger[0].angle[1] = 5;
            }
        }
    }

    AR_RETURN_VALUE LpV2SyncFFH::publishFfhCmd(size_t id) {
        if (ffhValid[id]) {
            postProcessHandCmd();
            std::cout << handCmd << std::endl;
            ffhs_[id]->send_hand_cmd(handCmd);
        }

        return AR_RETURN_VALUE::SUCCESS;
    }

    LEAP_TRACKING_EVENT& LpV2SyncFFH::getLastFrame() {
        return lmWrap.buffer().back();
    }

    std::vector<LEAP_HAND>& LpV2SyncFFH::getHands() {
        return lmWrap.hands();
    }

    void LpV2SyncFFH::sendInitHandData() {
        std::memset(&handCmd, 0, sizeof(udp_hand_cmd));
        publishFfhCmd(static_cast<size_t>(FFHEnumClass::left_hand));
    }

    AR_RETURN_VALUE LpV2SyncFFH::runIteration() {
        // check connection to server and device
        if (!lmWrap.inConnection()) {
            spdlog::error("Could not iterate. Leapd server is not connected.");
            sendInitHandData();
            return AR_RETURN_VALUE::ACTION_FAIL;
        }

        // get the newest hand data from data buffer
        auto hands = getHands();

        // no hands detected
        // publish hand-open position values
        if (hands.empty()) {
            sendInitHandData();
            return AR_RETURN_VALUE::SUCCESS;
        }

        for (auto& hand : hands) {
            // 只检测左手并转发
            if (hand.type == eLeapHandType_Left) {
                if (calPubFingerAngle(hand)) {
                    publishFfhCmd(static_cast<size_t>(FFHEnumClass::left_hand));
                } else {
                    sendInitHandData();
                }
                return AR_RETURN_VALUE::SUCCESS;
            }
        }

        spdlog::info("There is no left hand detected in the frame.");
        return AR_RETURN_VALUE::SUCCESS;
    }

    /**
     * @brief if key 'q' is pressed then stop the daemon
     *
     */
    void LpV2SyncFFH::monitorDaemon() {
        while (isRunning()) {
            if (get_char() == 'q') {
                stop();
                return;
            }

            std::this_thread::sleep_for(200ms);  // wait move
        }
    }

    void LpV2SyncFFH::executeLoop() {
        while (isRunning()) {
            runIteration();
            std::this_thread::sleep_for(std::chrono::duration<double>(controlInternal_));
        }
    }

    bool LpV2SyncFFH::calPubFingerAngle(const LEAP_HAND& hand) {
        if (hand.confidence < 0.3) {
            spdlog::info("The hand data is noet believable");
            return false;
        }

        auto palm_normal    = Vector3(hand.palm.normal);
        auto hand_direction = hand.palm.direction;
        auto handDir        = Vector3(hand_direction);
        handDir.normalize();

        auto tipDir = Vector3{0, 0, 0} - handDir.getCross(Vector3(palm_normal));
        tipDir.normalize();

        // spdlog::info("The hand dir x is {}, y is {}, z is {}", tipDir.x(), tipDir.y(), tipDir.z());

        // the finger cmd which is needed to be send by udp
        auto& fingerCmd = handCmd.finger;

        // Get fingers
        auto fingers = hand.digits;

        for (auto i = 1; i < fingerNum_; ++i) {
            auto& data = fingers[i];

            fingerCmd[i].angle[static_cast<int>(FJIndex::Distal)] =
                r2d * LeapTool::getBonesAngle(data.bones[middleBone], data.bones[distalBone]);

            // if the fingers are turned away from palm, set angle to zero
            if (palm_normal.dot(Bone{data.bones[distalBone]}.direction() * r2d) < 0) {
                handCmd.finger[i].angle[static_cast<int>(FJIndex::Distal)] = 0;
            }

            fingerCmd[i].angle[static_cast<int>(FJIndex::Middle)] =
                r2d * LeapTool::getBonesAngle(data.bones[proxBone], data.bones[middleBone]);

            // if the fingers are turned away from palm, set angle to zero
            if (palm_normal.dot(Bone{data.bones[static_cast<int>(middleBone)]}.direction() * r2d) < 0) {
                handCmd.finger[i].angle[static_cast<int>(FJIndex::Middle)] = 0;
            }

            fingerCmd[i].angle[static_cast<int>(FJIndex::Proxiaml)] =
                // r2d * LeapTool::getBonesAngle(data.bones[handBone], data.bones[proxBone]);
                // r2d * tipDir.angleTo(Bone{data.bones[proxBone]}.direction());
                // r2d * Vector3{1, 1, 0}.angleTo(Bone{data.bones[proxBone].direction()});
                r2d * Vector3{0, 1, 1}.angleTo(Bone{data.bones[proxBone]}.direction());

            // if the fingers are turned away from palm, set angle to zero
            if (palm_normal.dot(Bone{data.bones[proxBone]}.direction() * r2d) < 0) {
                handCmd.finger[i].angle[static_cast<int>(FJIndex::Proxiaml)] = 0;
            }
        }

        // thumb is a bit special: the angle of the finger vs the hand gives us thumb oppose,
        //      and the angle between joints is needed to get the proximal angle
        //      - there is also no "kHandBone" in the thumb

        // let's send the same value to all joints, as it is the most evident angle (from testing)
        // thumb proximal angle: finger_angles_[0][2]

        auto& data = fingers[0];
        fingerCmd[0].angle[static_cast<int>(FJIndex::Distal)] =
            r2d * LeapTool::getBonesAngle(data.bones[middleBone], data.bones[distalBone]);

        // if the fingers are turned away from palm, set angle to zero
        // WHY ? (copied from code above)
        // WHY do we need to multiply (int)Leap::RAD_TO_DEG inside dot product ?
        if (palm_normal.dot(Bone{data.bones[distalBone]}.direction() * r2d) < 0) {
            fingerCmd[0].angle[static_cast<int>(FJIndex::Distal)] = 0;
        }

        fingerCmd[0].angle[static_cast<int>(FJIndex::Middle)] = fingerCmd[0].angle[static_cast<int>(FJIndex::Distal)];

        fingerCmd[0].angle[static_cast<int>(FJIndex::Proxiaml)] =
            r2d * LeapTool::getBonesAngle(data.bones[proxBone], data.bones[middleBone]);

        if (palm_normal.dot(Bone{data.bones[distalBone]}.direction() * r2d) < 0) {
            fingerCmd[0].angle[static_cast<int>(FJIndex::Proxiaml)] = 0;
        }

        return true;
    }

}  // namespace ar::Hardware::LeapMotion
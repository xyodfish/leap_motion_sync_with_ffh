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

        tryConnectTime_ = node["connection_internal"].as<double>();
        ffhValid        = node["hand_Valid"].as<std::vector<int>>();
        auto ffhInfo    = node["hand_info"];

        for (auto i = 0; i < handTotalNum; ++i) {
            if (!ffhValid[i])
                continue;

            auto handNode = ffhInfo[EnumUtils::intToString<size_t, FFHEnumClass>(i)];
            ffhs_[i] =
                std::make_shared<FfhCtrl>(FfhUdpData{i, handNode["ip"].as<std::string>(), handNode["port"].as<int>()});

            lowerLimits_ = handNode["lower_limits"].as<std::vector<std::vector<float>>>();
            upperLimits_ = handNode["upper_limits"].as<std::vector<std::vector<float>>>();
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
        if (lmWrap.connect() != AR_RETURN_VALUE::SUCCESS) {
            spdlog::error("Cannot connect to Leap Motion.");
            return AR_RETURN_VALUE::ACTION_FAIL;
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

    void LpV2SyncFFH::start() {
        Runnable::start();
        initialize();

        mThreads.push_back(std::thread(&LpV2SyncFFH::monitorDaemon, this));
        mThreads.push_back(std::thread(&LpV2SyncFFH::executeLoop, this));

        while (isRunning()) {
            std::this_thread::sleep_for(10ms);
        }

        for (auto& th : mThreads) {
            if (th.joinable()) {
                th.join();
            }
        }
        mThreads.clear();
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

        /// 食指指侧向角度取反
        handCmd.finger[1].angle[2] = -handCmd.finger[1].angle[2];
    }

    AR_RETURN_VALUE LpV2SyncFFH::pubFfhCmd(size_t id) {
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
        pubFfhCmd(static_cast<size_t>(FFHEnumClass::left_hand));
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
                    pubFfhCmd(static_cast<size_t>(FFHEnumClass::left_hand));
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
                r2d * LeapTool::getBonesAngle(data.bones[handBone], data.bones[proxBone]);

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

        // thumb abduction (thumb oppose): finger_angles_[0][3]
        // fingerCmd[0].angle[static_cast<int>(FJIndex::Proxiaml)] = 0;

        return true;
    }

}  // namespace ar::Hardware::LeapMotion
#include "leapMotionV2WrapperImpl.h"
#include <spdlog/spdlog.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <map>
#include <thread>
#include "arEnumUtils.h"

using namespace std::chrono_literals;
using namespace ar::Types;

static void lp_assert_impl(bool success, const char* instruction_line) {
    if (!success) {
        spdlog::error("ERROR: {}", instruction_line);
        abort();
    }
}

#define str_i(s) #s
#define str(s) str_i(s)

#define lp2_assert_i(test, msg) lp_assert_impl(test, __FILE__ ":" str(__LINE__) " -- " msg)
#define lp2_assert(test) lp2_assert_i((test), #test)

#define p_1 std::placeholders::_1
#define p_2 std::placeholders::_2
#define p_3 std::placeholders::_3

namespace ar::Hardware::LeapMotion {

    /* Used in Polling Example: */
    void deepCopyTrackingEvent(LEAP_TRACKING_EVENT* dst, const LEAP_TRACKING_EVENT* src) {
        memcpy(&dst->info, &src->info, sizeof(LEAP_FRAME_HEADER));
        dst->tracking_frame_id = src->tracking_frame_id;
        dst->nHands            = src->nHands;
        dst->framerate         = src->framerate;
        memcpy(dst->pHands, src->pHands, src->nHands * sizeof(LEAP_HAND));
    }

    LeapMotionV2Wrapper::LeapMotionV2WrapperImpl::~LeapMotionV2WrapperImpl() {
        // free malloc memory in the LeapMotionV2WrapperImpl
        if (lastFrame) {
            free(lastFrame);
            lastFrame = nullptr;
        }

        if (lastDevice) {
            free(lastDevice);
            lastDevice = nullptr;
        }
    }

    template <typename... Args>
    std::map<std::string, std::function<void(Args...)>> LeapMotionV2Wrapper::LeapMotionV2WrapperImpl::callBackFuncs_;

    template <typename... Args>
    void LeapMotionV2Wrapper::LeapMotionV2WrapperImpl::handleCallback(const std::string& name, Args... args) {
        if (callBackFuncs_<Args...>.find(name) != callBackFuncs_<Args...>.end()) {
            callBackFuncs_<Args...>[name](args...);
        }
    }

    void LeapMotionV2Wrapper::LeapMotionV2WrapperImpl::setFrame(const LEAP_TRACKING_EVENT* frame) {
        if (!lastFrame) {
            lastFrame         = static_cast<LEAP_TRACKING_EVENT*>(malloc(sizeof(LEAP_TRACKING_EVENT)));
            lastFrame->pHands = static_cast<LEAP_HAND*>(malloc(2 * sizeof(LEAP_HAND)));
        }

        if (frame != NULL) {
            deepCopyTrackingEvent(lastFrame, frame);
        }
    }

    void LeapMotionV2Wrapper::LeapMotionV2WrapperImpl::setDevice(const LEAP_DEVICE_INFO* deviceProps) {
        std::unique_lock<std::mutex> locker(mtx_);

        if (lastDevice) {
            free(lastDevice->serial);
        } else {
            lastDevice = static_cast<LEAP_DEVICE_INFO*>(malloc(sizeof(*deviceProps)));
        }

        *lastDevice        = *deviceProps;
        lastDevice->serial = static_cast<char*>(malloc(deviceProps->serial_length));
        memcpy(lastDevice->serial, deviceProps->serial, deviceProps->serial_length);
    }

    void LeapMotionV2Wrapper::LeapMotionV2WrapperImpl::handleDeviceEvent(const LEAP_DEVICE_EVENT* device_event) {
        LEAP_DEVICE deviceHandle;
        //Open device using LEAP_DEVICE_REF from event struct.
        auto retval = LeapOpenDevice(device_event->device, &deviceHandle);
        if (retval != eLeapRS_Success) {
            spdlog::info("Could not open device {}", EnumUtils::toString<eLeapRS>(retval));
            return;
        }

        //Create a struct to hold the device properties, we have to provide a buffer for the serial string
        LEAP_DEVICE_INFO deviceProperties = {sizeof(deviceProperties)};
        // Start with a length of 1 (pretending we don't know a priori what the length is).
        // Currently device serial numbers are all the same length, but that could change in the future
        deviceProperties.serial_length = 1;
        deviceProperties.serial        = static_cast<char*>(malloc(deviceProperties.serial_length));
        //This will fail since the serial buffer is only 1 character long
        // But deviceProperties is updated to contain the required buffer length
        retval = LeapGetDeviceInfo(deviceHandle, &deviceProperties);
        if (retval == eLeapRS_InsufficientBuffer) {
            //try again with correct buffer size
            deviceProperties.serial =
                static_cast<char*>(realloc(deviceProperties.serial, deviceProperties.serial_length));
            retval = LeapGetDeviceInfo(deviceHandle, &deviceProperties);

            if (retval != eLeapRS_Success) {
                spdlog::info("Failed to get device info {}", EnumUtils::toString<eLeapRS>(retval));
                free(deviceProperties.serial);
                return;
            }
        }

        setDevice(&deviceProperties);
        handleCallback("onDeviceFound", &deviceProperties);

        free(deviceProperties.serial);
        LeapCloseDevice(deviceHandle);
    }

    void LeapMotionV2Wrapper::LeapMotionV2WrapperImpl::handleTrackingEvent(const LEAP_TRACKING_EVENT* tracking_event) {
        setFrame(tracking_event);  //support polling tracking data from different thread
        handleCallback("onFrame", tracking_event);
    }

    void LeapMotionV2Wrapper::LeapMotionV2WrapperImpl::serviceMessageLoop() {
        eLeapRS retval;
        LEAP_CONNECTION_MESSAGE msg;

        while (inConncetion_) {
            unsigned int timeout = 1000;
            retval               = LeapPollConnection(connectionHandle_, timeout, &msg);

            if (retval != eLeapRS_Success) {
                spdlog::info("LeapC PollConnection cal was {}", EnumUtils::toString<eLeapRS>(retval));
                continue;
            }

            // spdlog::info("Time to process message {}");

            switch (msg.type) {
                case eLeapEventType_Connection:
                    handleCallback("onConnect");
                    break;
                case eLeapEventType_ConnectionLost:
                    handleCallback("onConnectLost");
                    break;
                case eLeapEventType_Device:
                    handleDeviceEvent(msg.device_event);
                    break;
                case eLeapEventType_Tracking:
                    handleTrackingEvent(msg.tracking_event);
                    break;
                default:
                    //discard unknown message types
                    spdlog::info("Unhandled message type {}", msg.type);
                    break;
            }  //switch on msg.type
        }

        spdlog::info("The service is shutting down");
    }

    void LeapMotionV2Wrapper::LeapMotionV2WrapperImpl::onConnectLost() {
        spdlog::error("connection from Leap Motion V2 is lost");
    }

    void LeapMotionV2Wrapper::LeapMotionV2WrapperImpl::onConnect() {
        spdlog::info("Connecting to LeapMotionV2 ......");

        // if (LeapMotionV2WrapperImpl::connect() != AR_RETURN_VALUE::SUCCESS) {
        //     spdlog::warn("Unable to connect to LeapMotionV2.");
        // }

        spdlog::info("The connection to the LeapMotionV2 has been estabilished.");
    }

    void LeapMotionV2Wrapper::LeapMotionV2WrapperImpl::onDeviceFound(const LEAP_DEVICE_INFO* props) {
        spdlog::info("Found device {}", props->serial);
    }

    void LeapMotionV2Wrapper::LeapMotionV2WrapperImpl::onDeviceFailure(const eLeapDeviceStatus failure_code,
                                                                       const LEAP_DEVICE failed_device) {}

    void LeapMotionV2Wrapper::LeapMotionV2WrapperImpl::onPolicy(const uint32_t current_policies) {}

    void LeapMotionV2Wrapper::LeapMotionV2WrapperImpl::onFrame(const LEAP_TRACKING_EVENT* frame) {

        frameBuffer_.add(*frame);

        auto curData = frameBuffer_.back();

        // if (frame->info.frame_id % 60 == 0) {
        //     spdlog::info("Frame {} with {} hands", static_cast<long long int>(frame->info.frame_id), frame->nHands);
        // }

        if (hands_.size() != frame->nHands) {
            hands_.resize(frame->nHands);
        }

        if (frame->nHands == 0) {
            hands_.clear();
            spdlog::info("No hands have been detected");
            return;
        }

        for (auto i = 0; i < curData.nHands; ++i) {
            hands_[i] = frame->pHands[i];

            if (curData.info.frame_id % 180 == 0) {
                spdlog::info("print the finger confidence {}", hands_[i].confidence);
            }

            // spdlog::info("Hand id {} is a {} hand with position {}, {}, {},", hands_[i].id,
            //              (hands_[i].type == eLeapHandType_Left ? "left" : "right"), hands_[i].palm.position.x,
            //              hands_[i].palm.position.y, hands_[i].palm.position.z);

            // if (curData.info.frame_id % 180 == 0) {
            //     for (auto j = 0; j < 5; ++j) {
            //         for (auto k = 0; k < 4; ++k) {

            //             // 打印食指
            //             auto bones = hands_[i].digits[1].bones[k];
            //             spdlog::info("next joint info is {}, {} ,{}", bones.next_joint.x, bones.next_joint.y,
            //                          bones.next_joint.z);

            //             spdlog::info("prev joint info is {}, {} ,{}", bones.prev_joint.x, bones.prev_joint.y,
            //                          bones.prev_joint.z);
            //         }
            //     }
            // }

            // if (lastData.info.frame_id % 30 == 0) {

            //     auto grabAngle = hands_[i].grab_angle;

            //     if (grabAngle > 2.2) {
            //         spdlog::info("The shape is stone");
            //     } else if (grabAngle < 0.65) {
            //         spdlog::info("The shape is cloth");
            //     } else {
            //         spdlog::info("The shape is scissor");
            //     }
            // }
        }
    }

    void LeapMotionV2Wrapper::LeapMotionV2WrapperImpl::onLogMsg(const eLeapLogSeverity severity,
                                                                const int64_t timestamp, const char* message) {}

    void LeapMotionV2Wrapper::LeapMotionV2WrapperImpl::onCfgChange(const uint32_t requestID, const bool success) {}

    void LeapMotionV2Wrapper::LeapMotionV2WrapperImpl::onCfgRes(const uint32_t requestID, LEAP_VARIANT value) {}

    void LeapMotionV2Wrapper::LeapMotionV2WrapperImpl::onImage(const LEAP_IMAGE_EVENT* image_event) {}

    void LeapMotionV2Wrapper::LeapMotionV2WrapperImpl::onPointMappingChange(
        const LEAP_POINT_MAPPING_CHANGE_EVENT* point_mapping_change_event) {}

    void LeapMotionV2Wrapper::LeapMotionV2WrapperImpl::onHeadPose(
        const LEAP_HEAD_POSE_EVENT* point_mapping_change_event) {}

    void LeapMotionV2Wrapper::LeapMotionV2WrapperImpl::onImu(const LEAP_IMU_EVENT* point_mapping_change_event) {}

    void LeapMotionV2Wrapper::LeapMotionV2WrapperImpl::onTrackingMode(
        const LEAP_TRACKING_MODE_EVENT* point_mapping_change_event) {}

    template <typename... Args>
    void LeapMotionV2Wrapper::LeapMotionV2WrapperImpl::regCallback(const std::string& CBType,
                                                                   std::function<void(Args...)> callBackFunc) {
        callBackFuncs_<Args...>[CBType] = (callBackFunc);
    }

    void LeapMotionV2Wrapper::LeapMotionV2WrapperImpl::initialize() {

        std::function<void()> _onConnect     = std::bind(&LeapMotionV2WrapperImpl::onConnect, this);
        std::function<void()> _onConnectLost = std::bind(&LeapMotionV2WrapperImpl::onConnectLost, this);
        std::function<void(const LEAP_DEVICE_INFO*)> _onDeviceFound =
            std::bind(&LeapMotionV2WrapperImpl::onDeviceFound, this, p_1);

        std::function<void(const eLeapDeviceStatus, const LEAP_DEVICE)> _onDeviceFailure =
            std::bind(&LeapMotionV2WrapperImpl::onDeviceFailure, this, p_1, p_2);

        std::function<void(const uint32_t)> _onPolicy = std::bind(&LeapMotionV2WrapperImpl::onPolicy, this, p_1);
        std::function<void(const LEAP_TRACKING_EVENT*)> _onFrame =
            std::bind(&LeapMotionV2WrapperImpl::onFrame, this, p_1);
        std::function<void(const eLeapLogSeverity, const int64_t, const char*)> _onLogMsg =
            std::bind(&LeapMotionV2WrapperImpl::onLogMsg, this, p_1, p_2, p_3);

        std::function<void(const uint32_t, const bool)> _onCfgChange =
            std::bind(&LeapMotionV2WrapperImpl::onCfgChange, this, p_1, p_2);

        std::function<void(const uint32_t, LEAP_VARIANT)> _onCfgRes =
            std::bind(&LeapMotionV2WrapperImpl::onCfgRes, this, p_1, p_2);
        std::function<void(const LEAP_IMAGE_EVENT*)> _onImage = std::bind(&LeapMotionV2WrapperImpl::onImage, this, p_1);
        std::function<void(const LEAP_POINT_MAPPING_CHANGE_EVENT*)> _onPointMappingChange =
            std::bind(&LeapMotionV2WrapperImpl::onPointMappingChange, this, p_1);

        std::function<void(const LEAP_HEAD_POSE_EVENT*)> _onHeadPose =
            std::bind(&LeapMotionV2WrapperImpl::onHeadPose, this, p_1);
        std::function<void(const LEAP_IMU_EVENT*)> _onImu = std::bind(&LeapMotionV2WrapperImpl::onImu, this, p_1);
        std::function<void(const LEAP_TRACKING_MODE_EVENT*)> _onTrackingMode =
            std::bind(&LeapMotionV2WrapperImpl::onTrackingMode, this, p_1);

        regCallback("onConnect", _onConnect);
        regCallback("onConnectLost", _onConnectLost);
        regCallback("onDeviceFound", _onDeviceFound);
        regCallback("onPolicy", _onPolicy);
        regCallback("onFrame", _onFrame);
        regCallback("onLogMsg", _onLogMsg);
        regCallback("onCfgChange", _onCfgChange);
        regCallback("onCfgRes", _onCfgRes);
        regCallback("onCfgRes", _onCfgRes);
        regCallback("onImage", _onImage);
        regCallback("onHeadPose", _onHeadPose);
        regCallback("onImu", _onImu);
        regCallback("onTrackingMode", _onTrackingMode);

        frameBuffer_.setSize(frameCount_);
    }

    AR_RETURN_VALUE LeapMotionV2Wrapper::LeapMotionV2WrapperImpl::disconnect() {

        if (inConncetion_) {
            inConncetion_ = false;
            for (auto& thread : pollingThread_) {
                if (thread.joinable())
                    thread.join();
            }

            LeapCloseConnection(connectionHandle_);
            LeapDestroyConnection(connectionHandle_);
            spdlog::info("Closing LeapMotionV2 connection.");
        }

        return AR_RETURN_VALUE::SUCCESS;
    }

    AR_RETURN_VALUE LeapMotionV2Wrapper::LeapMotionV2WrapperImpl::connect() {

        lp2_assert(LeapCreateConnection(NULL, &connectionHandle_) == eLeapRS_Success);
        lp2_assert(LeapOpenConnection(connectionHandle_) == eLeapRS_Success);

        const uint32_t timeout = 1000U;

        uint32_t computed_array_size = 0U;

        for (uint32_t retry_limit = 7U; retry_limit > 0; --retry_limit) {
            LEAP_CONNECTION_MESSAGE msg;

            LeapPollConnection(connectionHandle_, timeout, &msg);

            eLeapRS retVal = LeapGetDeviceList(connectionHandle_, NULL, &computed_array_size);
            if (retVal == eLeapRS_NotConnected) {
                continue;
            }
            // assert(retVal == eLeapRS_Success);
            spdlog::info("Number of devices available: {}", computed_array_size);

            if (computed_array_size > 0U) {
                break;
            }
        }

        if (computed_array_size > 0U) {
            LEAP_DEVICE_REF* leap_device_list = (LEAP_DEVICE_REF*)malloc(sizeof(LEAP_DEVICE_REF) * computed_array_size);
            lp2_assert(LeapGetDeviceList(connectionHandle_, leap_device_list, &computed_array_size) == eLeapRS_Success);

            /* Make use of leap_device_list here */
            free(leap_device_list);
            inConncetion_ = true;
            pollingThread_.push_back(std::thread(&LeapMotionV2WrapperImpl::serviceMessageLoop, this));
            return AR_RETURN_VALUE::SUCCESS;
        }

        return AR_RETURN_VALUE::ACTION_FAIL;
    }

}  // namespace ar::Hardware::LeapMotion
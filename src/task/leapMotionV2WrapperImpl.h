#ifndef __LEAP_MOTION_V2_WRAPPER_IMPL_H__
#define __LEAP_MOTION_V2_WRAPPER_IMPL_H__

#include "leapMotionV2Wrapper.h"

namespace ar::Hardware::LeapMotion {
    class LeapMotionV2Wrapper::LeapMotionV2WrapperImpl {

       public:
        explicit LeapMotionV2WrapperImpl() = default;
        ~LeapMotionV2WrapperImpl();

        AR_RETURN_VALUE connect();
        AR_RETURN_VALUE disconnect();

        bool inConncetion() const noexcept { return inConncetion_; }

        void initialize();

        template <typename... Args>
        static std::map<std::string, std::function<void(Args...)>> callBackFuncs_;

        CircularBuffer<LEAP_TRACKING_EVENT>& buffer() { return frameBuffer_; };

        std::vector<LEAP_HAND>& hands() { return hands_; };

       private:
        LEAP_CONNECTION connectionHandle_;
        bool inConncetion_{false};
        bool _isRunning{false};

        /// @brief Callback function type
        void onConnect();
        void onConnectLost();
        void onDeviceFound(const LEAP_DEVICE_INFO* props);
        void onDeviceFailure(const eLeapDeviceStatus faileure_code, const LEAP_DEVICE failed_device);
        void onPolicy(const uint32_t current_policies);
        void onFrame(const LEAP_TRACKING_EVENT* tracking_event);
        void onLogMsg(const eLeapLogSeverity severity, const int64_t timestamp, const char* message);
        void onCfgChange(const uint32_t requestID, const bool success);
        void onCfgRes(const uint32_t requestID, LEAP_VARIANT value);
        void onImage(const LEAP_IMAGE_EVENT* image_event);
        void onPointMappingChange(const LEAP_POINT_MAPPING_CHANGE_EVENT* point_mapping_change_event);
        void onHeadPose(const LEAP_HEAD_POSE_EVENT* head_pose_event);
        void onImu(const LEAP_IMU_EVENT* imu_event);
        void onTrackingMode(const LEAP_TRACKING_MODE_EVENT* mode_event);

        /// @brief get service msg and process fsm function
        void serviceMessageLoop();

        template <typename... Args>
        void regCallback(const std::string& CBType, std::function<void(Args...)> callBackFunc);

        template <typename... Args>
        static void handleCallback(const std::string& name, Args... args);

        std::vector<std::thread> pollingThread_;

        void handleDeviceEvent(const LEAP_DEVICE_EVENT* device_event);
        void handleTrackingEvent(const LEAP_TRACKING_EVENT* tracking_event);

        void setDevice(const LEAP_DEVICE_INFO* deviceProps);
        void setFrame(const LEAP_TRACKING_EVENT* frame);

        std::mutex mtx_;

        LEAP_DEVICE_INFO* lastDevice   = nullptr;
        LEAP_TRACKING_EVENT* lastFrame = nullptr;

        std::vector<LEAP_HAND> hands_;

        CircularBuffer<LEAP_TRACKING_EVENT> frameBuffer_;

        static constexpr size_t frameCount_{60};
    };

}  // namespace ar::Hardware::LeapMotion

#endif
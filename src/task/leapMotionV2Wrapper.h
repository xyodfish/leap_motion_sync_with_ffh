
/**
 * @brief 以下是对leapMotion中的数据及枚举的说明
 * metacarpal： 掌指关节
 * proximal： 近端指间关节
 * intermediate： 中间骨
 * distal:远端指间关节
 */

#ifndef __LEAP_MOTION_V2_WRAPPER_H__
#define __LEAP_MOTION_V2_WRAPPER_H__

#include <map>
#include <memory>
#include "LeapC.h"
#include "arCircularBuffer.h"
#include "arDeviceBase.h"

using namespace ar::Types;

namespace ar::Hardware::LeapMotion {
    class LeapMotionV2Wrapper : public ar::Hardware::DeviceBase {

       private:
        class LeapMotionV2WrapperImpl;
        std::unique_ptr<LeapMotionV2WrapperImpl> pImpl_;

       public:
        LeapMotionV2Wrapper();
        ~LeapMotionV2Wrapper();

        virtual AR_RETURN_VALUE connect() override;
        virtual AR_RETURN_VALUE disconnect() override;
        virtual AR_RETURN_VALUE initialize(const std::string& config) override { return AR_RETURN_VALUE::SUCCESS; };
        virtual AR_RETURN_VALUE parseConfig(const std::string& config) override { return AR_RETURN_VALUE::SUCCESS; };
        virtual void start() override {};
        virtual void stop() override {};

        void initialize();
        bool inConnection();

        CircularBuffer<LEAP_TRACKING_EVENT>& buffer();

        std::vector<LEAP_HAND>& hands();
    };
}  // namespace ar::Hardware::LeapMotion

#endif
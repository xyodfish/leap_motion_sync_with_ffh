#include "leapMotionV2Wrapper.h"
#include "leapMotionV2WrapperImpl.h"

namespace ar::Hardware::LeapMotion {

    LeapMotionV2Wrapper::LeapMotionV2Wrapper() : pImpl_(std::make_unique<LeapMotionV2WrapperImpl>()) {}
    LeapMotionV2Wrapper::~LeapMotionV2Wrapper() {
        disconnect();
    }

    void LeapMotionV2Wrapper::initialize() {
        return pImpl_->initialize();
    }

    bool LeapMotionV2Wrapper::inConnection() {
        return pImpl_->inConncetion();
    }

    AR_RETURN_VALUE LeapMotionV2Wrapper::connect() {
        return pImpl_->connect();
    }

    AR_RETURN_VALUE LeapMotionV2Wrapper::disconnect() {
        return pImpl_->disconnect();
    }

    CircularBuffer<LEAP_TRACKING_EVENT>& LeapMotionV2Wrapper::buffer() {
        return pImpl_->buffer();
    }

    std::vector<LEAP_HAND>& LeapMotionV2Wrapper::hands() {
        return pImpl_->hands();
    }

}  // namespace ar::Hardware::LeapMotion

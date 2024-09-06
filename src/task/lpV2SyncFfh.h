#ifndef __LP2_SYNC_FFH_H__
#define __LP2_SYNC_FFH_H__

#include "ffhCtrl.h"
#include "leapMotionV2Wrapper.h"

using namespace ar::Types;

namespace ar::Hardware::LeapMotion {

    enum class BoneType {
        BT_METACARPAL   = 0, /**< Bone connected to the wrist inside the palm */
        BT_PROXIMAL     = 1, /**< Bone connecting to the palm */
        BT_INTERMEDIATE = 2, /**< Bone between the tip and the base*/
        BT_DISTAL       = 3, /**< Bone at the tip of the finger */
    };

    class LpV2SyncFFH : public ar::Hardware::DeviceBase {
       public:
        LpV2SyncFFH();
        ~LpV2SyncFFH();

        virtual AR_RETURN_VALUE connect() override;
        virtual AR_RETURN_VALUE disconnect() override;
        virtual AR_RETURN_VALUE initialize(const std::string& config) override { return AR_RETURN_VALUE::SUCCESS; }
        virtual AR_RETURN_VALUE parseConfig(const std::string& config) override;
        virtual void start() override;
        virtual void stop() override;

        void initialize();
        bool inConnection() const;

       private:
        LeapMotionV2Wrapper lmWrap;
        std::vector<std::shared_ptr<FfhCtrl>> ffhs_;
        udp_hand_cmd handCmd;
        std::vector<int> ffhValid;

        void postProcessHandCmd();

        LEAP_TRACKING_EVENT& getLastFrame();

        void executeLoop();
        void monitorDaemon();

        AR_RETURN_VALUE runIteration();
        AR_RETURN_VALUE pubFfhCmd(size_t id);

        std::vector<LEAP_HAND>& getHands();
        bool calPubFingerAngle(const LEAP_HAND& hand);

        static constexpr size_t handTotalNum{2};
        static constexpr size_t fingerNum_{5}, angleNum_{3};

        bool inConnection_{false};

        AR_RETURN_VALUE connectFFH();
        AR_RETURN_VALUE disconnectFFH();

        void sendInitHandData();

        std::vector<std::thread> mThreads;

        // 连接到掌骨的骨头，位于手指的基部，连接到掌骨。
        static constexpr int proxBone = static_cast<int>(BoneType::BT_PROXIMAL);
        // 这是连接到手腕内侧的掌骨，位于手腕和手指之间。
        static constexpr int handBone = static_cast<int>(BoneType::BT_METACARPAL);
        // 这是位于指尖和基部之间的骨头，位于手指的中间位置。
        static constexpr int middleBone = static_cast<int>(BoneType::BT_INTERMEDIATE);
        // 这是位于手指尖端的骨头，位于手指的最远端。
        static constexpr int distalBone = static_cast<int>(BoneType::BT_DISTAL);

        std::vector<float> lowerLimit_, upperLimit_;
    };

}  // namespace ar::Hardware::LeapMotion

#endif
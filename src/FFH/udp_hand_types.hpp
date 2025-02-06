#ifndef UDP_HAND_CMD_H_
#define UDP_HAND_CMD_H_
#include <iostream>

// This is the shared part of data structure for both python and cpp code
// Don't include it directly; include 'udp_hand_types.hpp' instead
// distal, base forward-backword, base left-right
const float LIMIT_HCMD_LOWER[4] = {0, 0, -15, -30};
const float LIMIT_HCMD_UPPER[4] = {90, 90, 15, 30};

struct udp_finger_cmd {
    float angle[4];
    // float distal;
    // float base_forw_backw;
    // float base_left_right;  // 侧摆（除了拇指外的指头）
    // float thumb angle;
};

struct udp_hand_cmd {
    struct udp_finger_cmd finger[5];
};

struct udp_finger_data {
    float distal_angle;
    float base_forw_backw_angle;
    float base_left_right_angle;  // 侧摆（除了拇指外的指头）
    float torque[3];              // tip, base1, base left right
};

struct udp_hand_data {
    struct udp_finger_data finger[5];
    float thumb_palm_angle;
};

std::ostream& operator<<(std::ostream& os, const struct udp_finger_cmd& fcmd);
std::ostream& operator<<(std::ostream& os, const struct udp_hand_cmd& handcmd);

std::ostream& operator<<(std::ostream& os, const struct udp_finger_data& fcmd);
std::ostream& operator<<(std::ostream& os, const struct udp_hand_data& handcmd);

enum class FJIndex {
    Distal = 0,  // 远短指骨
    FbDir,       // 前后弯曲
    LrDir,       // 左右弯曲
    ThumbPalm,   // 大拇指虎口合掌
};  // FingerJointIndex

enum class FFHEnumClass { left_hand = 0, right_hand = 1 };

class FfhUdpData {
   public:
    int id;
    std::string ip;
    int port;
};

#endif  // UDP_HAND_CMD_H_
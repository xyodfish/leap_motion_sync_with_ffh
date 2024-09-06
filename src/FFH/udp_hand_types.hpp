#ifndef UDP_HAND_CMD_H_
#define UDP_HAND_CMD_H_
#include <iostream>

// This is the shared part of data structure for both python and cpp code
// Don't include it directly; include 'udp_hand_types.hpp' instead
// distal, base forward-backword, base left-right
const float LIMIT_HCMD_LOWER[3] = {0, 0, -10};
const float LIMIT_HCMD_UPPER[3] = {90, 90, 10};

struct udp_finger_cmd {
    float angle[3];
    // float distal;
    // float base_forw_backw;
    // float base_left_right;
};

struct udp_hand_cmd {
    struct udp_finger_cmd finger[5];
};

struct udp_finger_data {
    float distal_angle;
    float base_forw_backw_angle;
    float base_left_right_angle;
    float torque[3];  // tip, base1, base left right
};

struct udp_hand_data {
    struct udp_finger_data finger[5];
};

std::ostream& operator<<(std::ostream& os, const struct udp_finger_cmd& fcmd);
std::ostream& operator<<(std::ostream& os, const struct udp_hand_cmd& handcmd);

std::ostream& operator<<(std::ostream& os, const struct udp_finger_data& fcmd);
std::ostream& operator<<(std::ostream& os, const struct udp_hand_data& handcmd);

enum class FJIndex {
    Distal = 0,  // 远短指骨
    Middle,      // 中间指骨
    Proxiaml     // 近端指骨
};  // FingerJointIndex

enum class FFHEnumClass { left_hand = 0, right_hand = 1 };

class FfhUdpData {
   public:
    int id;
    std::string ip;
    int port;
};

#endif  // UDP_HAND_CMD_H_
#include "udp_hand_types.hpp"
#include <iomanip>

std::ostream& operator<<(std::ostream& os, const struct udp_finger_cmd& fcmd) {
    os << std::fixed << std::setprecision(2);
    os << "[" << std::setw(7) << fcmd.angle[0] << ",";
    os << std::setw(7) << fcmd.angle[1] << ",";
    os << std::setw(7) << fcmd.angle[2] << "]\n";
    os << std::setw(7) << fcmd.angle[3] << "]\n";
    return os;
}

std::ostream& operator<<(std::ostream& os, const struct udp_hand_cmd& handcmd) {
    os << std::setw(15) << "---hand cmd---" << ":";
    for (int i = 0; i < 5; i++) {
        os << "   F[" << i << "] ";
    }
    os << "\n";

    const size_t angleNum = sizeof(handcmd.finger[0]) / sizeof(float);

    const char* row_name[] = {"distal", "base_forw_backw", "base_left_right", "thumb_palm_pos"};
    for (int row = 0; row < 3; row++) {
        os << std::setw(15) << row_name[row] << ":";
        for (int i = 0; i < 5; i++) {
            const float* row_value = const_cast<float*>(&handcmd.finger[i].angle[0]) + row;
            os << std::setw(7) << *row_value << " ";
        }
        os << "\n";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const struct udp_finger_data& fdata) {
    os << "{\n"
       << "\tdistal_angle = " << fdata.distal_angle << "\n"
       << "\tbase_forw_backw_angle = " << fdata.base_forw_backw_angle << "\n"
       << "\tbase_left_right_angle = " << fdata.base_left_right_angle << "\n"
       << "}\n";
    return os;
}

std::ostream& operator<<(std::ostream& os, const struct udp_hand_data& handdata) {
    os << std::setw(15) << "---hand DATA---" << ":";
    for (int i = 0; i < 5; i++) {
        os << "   F[" << i << "] ";
    }
    os << "\n";

    const char* row_name[] = {"distal", "base_forw_backw", "base_left_right"};
    for (int row = 0; row < 3; row++) {
        os << std::setw(15) << row_name[row] << ":";
        for (int i = 0; i < 5; i++) {
            const float* row_value = const_cast<float*>(&handdata.finger[i].distal_angle) + row;
            os << std::setw(7) << *row_value << " ";
        }
        os << "\n";
    }

    os << std::setw(15) << row_name[3];
    os << std::setw(7) << handdata.thumb_palm_angle;
    os << "\n";
    return os;
}
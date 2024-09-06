#include "udp_hand_types.hpp"
#include <boost/format.hpp>

using boost::format;

std::ostream& operator<<(std::ostream& os, const struct udp_finger_cmd& fcmd) {
    os << format("[%7.2f,") % fcmd.angle[0];
    os << format(" %7.2f,") % fcmd.angle[1];
    os << format(" %7.2f]\n") % fcmd.angle[2];
    return os;
}

std::ostream& operator<<(std::ostream& os, const struct udp_hand_cmd& handcmd) {
    os << format("%-15s :") % "---hand cmd---";
    for (int i{0}; i < 5; i++) {
        os << format("   F[%1d] ") % i;
    }
    os << "\n";

    const char* row_name[] = {"distal", "base_forw_backw", "base_left_right"};
    for (int row{0}; row < 3; row++) {
        os << format("%-15s :") % row_name[row];
        for (int i{0}; i < 5; i++) {
            const float* row_value = const_cast<float*>(&handcmd.finger[i].angle[0]) + row;
            os << format("%7.2f ") % *row_value;
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
    os << format("%-15s :") % "---hand DATA---";
    for (int i{0}; i < 5; i++) {
        os << format("   F[%1d] ") % i;
    }
    os << "\n";

    const char* row_name[] = {"distal", "base_forw_backw", "base_left_right"};
    for (int row{0}; row < 3; row++) {
        os << format("%-15s :") % row_name[row];
        for (int i{0}; i < 5; i++) {
            const float* row_value = const_cast<float*>(&handdata.finger[i].distal_angle) + row;
            os << format("%7.2f ") % *row_value;
        }
        os << "\n";
    }
    return os;
}
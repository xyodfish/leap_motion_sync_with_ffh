/**
 * @file arEnumUtils.h
 * @brief Provide string conversion functions for ar_enum
 * use the header only library magic_enum.hpp, the range of enum value is [-128, 128] 
 * you can change the marco MAGIC_ENUM_RANGE_MIN and MAGIC_ENUM_RANGE_MAX to change the range
 * if you change the marco to large the compile time will be increased
 *
 * @author yu.xia (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-08-28
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef __AR_ENUM_UTILS_H_
#define __AR_ENUM_UTILS_H_

#include <stdexcept>
#include "magic_enum.hpp"

namespace ar::Types {
    class EnumUtils {
       public:
        template <typename T, class U>
        static std::string intToString(const T& value) {
            return std::string(magic_enum::enum_name<U>(static_cast<U>(value)));
        }

        template <typename U>
        static std::string toString(const U& value) {
            return std::string(magic_enum::enum_name<U>(value));
        }

        template <class U>
        static std::string toEnum(const U& enumVar) {
            return std::string(magic_enum::enum_name<U>(enumVar));
        }

        template <class U>
        static U toEnum(const std::string& name) {
            auto res = magic_enum::enum_cast<U>(name);
            if (res.has_value()) {
                return res.value();
            } else {
                throw std::runtime_error("the input string is not a valid enum string");
            }
        }
    };
}  // namespace ar::Types

#endif
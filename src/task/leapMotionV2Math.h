#ifndef __LEAP_MOTION_V2_MATH_H__
#define __LEAP_MOTION_V2_MATH_H__

#include <cmath>
#include "LeapC.h"

namespace ar::Hardware::LeapMotion {

    static const float LEAP_EPSILON = 1.192092896e-07f;
    static const float LEAP_PI      = 3.1415926536f;
    static const float r2d          = 57.295779513f;

    class Vector3 {
       public:
        Vector3() {};
        Vector3(const float& x, const float& y, const float& z) : v_({x, y, z}) {}
        Vector3(const LEAP_VECTOR& vector) : v_(vector) {}
        Vector3(const LEAP_VECTOR& v1, const LEAP_VECTOR& v2) : v_{v1.x - v2.x, v1.y - v2.y, v1.z - v2.z} {}

        void normalize() {
            if (magnitude() == 0) {
                v_.x = v_.y = v_.z = 0.0f;
                return;
            }

            v_.x /= magnitude();
            v_.y /= magnitude();
            v_.z /= magnitude();
        }

        inline LEAP_VECTOR& raw() { return v_; }

        float magnitudeSquared() { return (v_.x * v_.x) + (v_.y * v_.y) + (v_.z * v_.z); }

        /// @brief  模长
        /// @return
        float magnitude() { return std::sqrt((v_.x * v_.x) + (v_.y * v_.y) + (v_.z * v_.z)); }

        /**
         * @brief cal angle between to vectors
         * 
         * @param v1 
         * @return float 
         */
        inline float angleTo(Vector3 v3) {

            float denom = this->magnitudeSquared() * v3.magnitudeSquared();

            if (denom <= LEAP_EPSILON) {
                return 0.0f;
            }
            float val = this->dot(v3) / std::sqrt(denom);

            if (val >= 1.0f) {
                return 0.0f;
            } else if (val <= -1.0f) {
                return LEAP_PI;
            }

            return std::acos(val);
        }

        float dot(Vector3 v) { return (v_.x * v.x()) + (v_.y * v.y()) + (v_.z * v.z()); }

        /**
         * @brief Multiply vector by a scalar.
         * 
         * @param scalar 
         * @return Vector3 
         */
        Vector3 operator*(float scalar) const { return Vector3(v_.x * scalar, v_.y * scalar, v_.z * scalar); }

        Vector3 operator-(Vector3 other) const { return Vector3(v_.x - other.x(), v_.y - other.y(), v_.z - other.z()); }

        /**
         * @brief 计算当前向量与另一个向量的叉积。
         * 
         * @param v3 
         * @return Vector3 
         */
        Vector3 getCross(Vector3 v3) {
            return Vector3(v_.y * v3.z() - v_.z * v3.y(), v_.z * v3.x() - v_.x * v3.z(), v_.x * v3.y() - v_.y * v3.x());
        }

        float x() const { return v_.x; }
        float y() const { return v_.y; }
        float z() const { return v_.z; }

       private:
        LEAP_VECTOR v_;
    };

    class Bone {
       public:
        // TODO implement constructor and math function
        Bone(const LEAP_BONE& bone) : bone_(bone) {
            dir_ = Vector3{bone_.next_joint.x - bone_.prev_joint.x, bone_.next_joint.y - bone_.prev_joint.y,
                           bone_.next_joint.z - bone_.prev_joint.z};
        }

        inline LEAP_BONE& raw() { return bone_; }
        inline Vector3 direction() { return dir_; }

       private:
        LEAP_BONE bone_;
        Vector3 dir_;
    };

    class LeapTool {
       public:
        static float getBonesAngle(const LEAP_BONE& b1, const LEAP_BONE& b2) {
            Bone lb1(b1);
            Bone lb2(b2);
            return lb1.direction().angleTo(lb2.direction());
        }
    };
}  // namespace ar::Hardware::LeapMotion

#endif
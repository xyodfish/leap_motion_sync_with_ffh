#ifndef __LEAP_MOTION_V2_MATH_H__
#define __LEAP_MOTION_V2_MATH_H__

#include <cmath>
#include "LeapC.h"

namespace ar::Hardware::LeapMotion {

    static constexpr float LEAP_EPSILON = 1.192092896e-07f;
    static constexpr float LEAP_PI      = 3.1415926536f;
    static constexpr float r2d          = 57.295779513f;

    class Vector3 {
       public:
        Vector3() {};
        Vector3(const float& x, const float& y, const float& z) : x_(x), y_(y), z_(z) {}
        Vector3(const LEAP_VECTOR& vector) : x_(vector.x), y_(vector.y), z_(vector.z) {}
        Vector3(const LEAP_VECTOR& v1, const LEAP_VECTOR& v2) : x_(v1.x - v2.x), y_(v1.y - v2.y), z_(v1.z - v2.z) {}

        void normalize() {
            float denom = this->magnitudeSquared();
            if (denom <= LEAP_EPSILON) {
                x_ = y_ = z_ = 0.0f;
                return;
            }

            denom = 1.0 / std::sqrt(denom);
            x_    = x_ * denom;
            y_    = y_ * denom;
            z_    = z_ * denom;
        }

        Vector3 normalized() {
            float denom = this->magnitudeSquared();
            if (denom <= LEAP_EPSILON) {
                return zero();
            }
            denom = 1.0 / std::sqrt(denom);
            return Vector3(static_cast<float>(x_ * denom), static_cast<float>(y_ * denom),
                           static_cast<float>(z_ * denom));
        }

        static const Vector3& zero() {
            static Vector3 s_zero(0, 0, 0);
            return s_zero;
        }

        float magnitudeSquared() { return (x_ * x_) + (y_ * y_) + (z_ * z_); }

        /// @brief  模长
        /// @return
        float magnitude() { return std::sqrt((x_ * x_) + (y_ * y_) + (z_ * z_)); }

        /**
         * @brief cal angle between to vectors
         * 
         * @param v1 
         * @return float 
         */
        inline float angleTo(Vector3 other) {

            float denom = this->magnitudeSquared() * other.magnitudeSquared();

            if (denom <= LEAP_EPSILON) {
                return 0.0f;
            }
            float val = this->dot(other) / std::sqrt(denom);

            if (val >= 1.0f) {
                return 0.0f;
            } else if (val <= -1.0f) {
                return LEAP_PI;
            }

            return std::acos(val);
        }

        float dot(Vector3 v) { return (x_ * v.x()) + (y_ * v.y()) + (z_ * v.z()); }

        /**
         * @brief Multiply vector by a scalar.
         * 
         * @param scalar 
         * @return Vector3 
         */
        Vector3 operator*(float scalar) const { return Vector3(x_ * scalar, y_ * scalar, z_ * scalar); }

        Vector3 operator-(Vector3 other) const { return Vector3(x_ - other.x(), y_ - other.y(), z_ - other.z()); }

        /**
         * @brief 计算当前向量与另一个向量的叉积。
         * 
         * @param v3 
         * @return Vector3 
         */
        Vector3 getCross(Vector3 other) {
            return Vector3(y_ * other.z() - z_ * other.y(), z_ * other.x() - x_ * other.z(),
                           x_ * other.y() - y_ * other.x());
        }

        /// @brief 计算向量other在当前向量上的投影向量。
        /// @param v
        /// @return
        Vector3 calProj(Vector3 v) {
            double scalar = this->dot(v) / this->dot(*this);
            return Vector3(x_ * scalar, y_ * scalar, z_ * scalar);
        }

        /// @brief 计算给定向量在以当前向量为法向量的平面上的投影向量。
        /// @param v
        /// @return
        Vector3 calProjToPlane(Vector3 v) {
            Vector3 proj = this->calProj(v);
            return Vector3(x_ - proj.x(), y_ - proj.y(), z_ - proj.z());
        }

        /// @brief 给定平面法向量 计算该向量与平面的夹角
        /// @param planeNormal
        /// @return
        float angleToPlane(Vector3 planeNormal) {
            float cosTheta = this->dot(planeNormal) / (this->magnitude() * planeNormal.magnitude());
            return std::acos(cosTheta);
        }

        float x() const { return x_; }
        float y() const { return y_; }
        float z() const { return z_; }

       private:
        float x_{0.0f}, y_{0.0f}, z_{0.0f};
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

        static float getVectorAngle(Vector3 v1, Vector3 v2) { return v1.angleTo(v2); }
    };
}  // namespace ar::Hardware::LeapMotion

#endif
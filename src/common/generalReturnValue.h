/**
 * @file generalReturnValue.h
 * @author xingchen.li@agile-robots.com
 * @brief collection of pre-defined return values
 * @version 0.1
 * @date 2023-11-02
 *
 * @copyright Copyright Agile Robots(c) 2023
 *
 */

#ifndef AR_ROBOT_BASE_GENERALRETURNVALUE_H
#define AR_ROBOT_BASE_GENERALRETURNVALUE_H

enum class AR_RETURN_VALUE {
    SUCCESS         = 0,
    MOTION_FAIL     = -1,
    ACTION_FAIL     = -2,
    ALGORITHM_ERROR = -3,
    INVALID_VALUE   = -4,
    OVER_TIME       = -5,
    COMM_ERROR      = -6,
    NOT_FINISHED    = -7

};
#endif  // AR_ROBOT_BASE_GENERALRETURNVALUE_H

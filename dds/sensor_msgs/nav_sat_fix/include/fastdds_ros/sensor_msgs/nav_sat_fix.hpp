/*
 * Copyright (C) 2026 SpacemiT (Hangzhou) Technology Co. Ltd.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <sensor_msgs/msg/nav_sat_fix.hpp>
#include <sensor_msgs/msg/detail/nav_sat_fix__rosidl_typesupport_fastrtps_cpp.hpp>

#include <fastdds_ros/ros_message.hpp>

namespace fastdds_ros
{

template<>
struct RosMessageType<sensor_msgs::msg::NavSatFix>
{
    static const rosidl_message_type_support_t* get()
    {
        return ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
            rosidl_typesupport_fastrtps_cpp, sensor_msgs, msg, NavSatFix)();
    }
};

using NavSatFixPublisher = RosPublisher<sensor_msgs::msg::NavSatFix>;
using NavSatFixSubscriber = RosSubscriber<sensor_msgs::msg::NavSatFix>;

}  // namespace fastdds_ros

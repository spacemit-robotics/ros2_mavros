/*
 * Copyright (C) 2026 SpacemiT (Hangzhou) Technology Co. Ltd.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/detail/pose_stamped__rosidl_typesupport_fastrtps_cpp.hpp>

#include <fastdds_ros/ros_message.hpp>

namespace fastdds_ros
{

template<>
struct RosMessageType<geometry_msgs::msg::PoseStamped>
{
    static const rosidl_message_type_support_t* get()
    {
        return ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
            rosidl_typesupport_fastrtps_cpp, geometry_msgs, msg, PoseStamped)();
    }
};

using PoseStampedPublisher = RosPublisher<geometry_msgs::msg::PoseStamped>;
using PoseStampedSubscriber = RosSubscriber<geometry_msgs::msg::PoseStamped>;

}  // namespace fastdds_ros

/*
 * Copyright (C) 2026 SpacemiT (Hangzhou) Technology Co. Ltd.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <mavros_msgs/msg/rc_in.hpp>
#include <mavros_msgs/msg/detail/rc_in__rosidl_typesupport_fastrtps_cpp.hpp>

#include <fastdds_ros/ros_message.hpp>

namespace fastdds_ros
{

template<>
struct RosMessageType<mavros_msgs::msg::RCIn>
{
    static const rosidl_message_type_support_t* get()
    {
        return ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
            rosidl_typesupport_fastrtps_cpp, mavros_msgs, msg, RCIn)();
    }
};

using RCInPublisher = RosPublisher<mavros_msgs::msg::RCIn>;
using RCInSubscriber = RosSubscriber<mavros_msgs::msg::RCIn>;

}  // namespace fastdds_ros

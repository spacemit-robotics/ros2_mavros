/*
 * Copyright (C) 2026 SpacemiT (Hangzhou) Technology Co. Ltd.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <functional>
#include <memory>
#include <string>

#include <sensor_msgs/msg/imu.hpp>

namespace fastdds_ros
{

class Context;

class ImuPublisher
{
public:
    ImuPublisher();
    ~ImuPublisher();

    // Use the exact DDS topic name (e.g. "rt/imu" for ROS topic "/imu").
    bool init(const std::string& dds_topic, int domain_id);
    bool init(const std::string& dds_topic, const std::shared_ptr<Context>& context);
    bool publish(const sensor_msgs::msg::Imu& message);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

class ImuSubscriber
{
public:
    using Callback = std::function<void(const sensor_msgs::msg::Imu&)>;

    ImuSubscriber();
    ~ImuSubscriber();

    // The callback runs on a DDS receive thread. Do not destroy the subscriber
    // from its callback, throw from it, or retain a reference to the message.
    bool init(const std::string& dds_topic, int domain_id, Callback callback);
    bool init(const std::string& dds_topic, const std::shared_ptr<Context>& context, Callback callback);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace fastdds_ros

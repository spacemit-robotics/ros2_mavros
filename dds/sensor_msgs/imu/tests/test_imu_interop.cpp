/*
 * Copyright (C) 2026 SpacemiT (Hangzhou) Technology Co. Ltd.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include <rcl/context.h>
#include <rclcpp/rclcpp.hpp>

#include "fastdds_ros/context.hpp"
#include "fastdds_ros/imu.hpp"

namespace
{

using Imu = sensor_msgs::msg::Imu;

int test_exchange()
{
    auto node = std::make_shared<rclcpp::Node>("fastdds_ros_imu_interop");
    std::size_t domain_id = 0;
    if (rcl_context_get_domain_id(
            node->get_node_base_interface()->get_context()->get_rcl_context().get(),
            &domain_id) != RCL_RET_OK) {
        return 1;
    }
    const std::string prefix = "fastdds_ros_imu_test_" + std::to_string(getpid());
    const std::string dds_output = prefix + "/dds_output";
    const std::string dds_output2 = prefix + "/dds_output2";
    const std::string ros_output = prefix + "/ros_output";

    Imu sample;
    sample.header.stamp.sec = 123;
    sample.header.stamp.nanosec = 456789012;
    sample.header.frame_id = "imu_interop_frame";
    sample.orientation.x = 0.1;
    sample.orientation.y = 0.2;
    sample.orientation.z = 0.3;
    sample.orientation.w = 0.9;
    sample.angular_velocity.x = 1.1;
    sample.angular_velocity.y = 2.2;
    sample.angular_velocity.z = 3.3;
    sample.linear_acceleration.x = -1.2;
    sample.linear_acceleration.y = 0.7;
    sample.linear_acceleration.z = 9.6;
    for (std::size_t i = 0; i < 9; ++i) {
        sample.orientation_covariance[i] = i * 0.01;
        sample.angular_velocity_covariance[i] = i * 0.02;
        sample.linear_acceleration_covariance[i] = i * 0.03;
    }
    sample.orientation_covariance[0] = -1.0;
    Imu sample2 = sample;
    sample2.header.frame_id = "second_imu";
    sample2.header.stamp.sec = 124;
    sample2.linear_acceleration.z = -9.5;

    std::atomic<bool> ros_received{false};
    std::atomic<bool> ros_received2{false};
    std::atomic<bool> dds_received{false};
    std::atomic<bool> local_dds_received{false};
    std::atomic<bool> mismatch{false};
    const auto qos = rclcpp::SensorDataQoS().keep_last(10);
    auto ros_subscriber = node->create_subscription<Imu>(
        "/" + dds_output, qos, [&](const Imu& received) {
            if (received == sample) {
                ros_received = true;
            } else {
                mismatch = true;
            }
        });
    auto ros_subscriber2 = node->create_subscription<Imu>(
        "/" + dds_output2, qos, [&](const Imu& received) {
            if (received == sample2) {
                ros_received2 = true;
            } else {
                mismatch = true;
            }
        });
    auto ros_publisher = node->create_publisher<Imu>("/" + ros_output, qos);
    auto context = fastdds_ros::Context::create(static_cast<int>(domain_id));
    fastdds_ros::ImuPublisher dds_publisher;
    fastdds_ros::ImuPublisher dds_publisher2;
    fastdds_ros::ImuSubscriber dds_subscriber;
    fastdds_ros::ImuSubscriber local_dds_subscriber;
    if (!context || fastdds_ros::Context::create(-1) || dds_publisher.publish(sample) ||
        dds_publisher.init("", context) ||
        dds_subscriber.init("rt/" + ros_output, static_cast<int>(domain_id), {})) {
        std::cerr << "An invalid endpoint operation returned success\n";
        return 1;
    }
    if (!dds_publisher.init("rt/" + dds_output, context) ||
        !dds_publisher2.init("rt/" + dds_output2, context) ||
        !dds_subscriber.init("rt/" + ros_output, context,
            [&](const Imu& received) {
                if (received == sample) {
                    dds_received = true;
                } else {
                    mismatch = true;
                }
            }) ||
        !local_dds_subscriber.init("rt/" + dds_output2, context,
            [&](const Imu& received) {
                if (received == sample2) {
                    local_dds_received = true;
                } else {
                    mismatch = true;
                }
            })) {
        std::cerr << "Failed to initialize Fast DDS endpoints\n";
        return 1;
    }
    // Endpoint handles own the context. Dropping the caller's reference must
    // not invalidate the topics, callbacks, or registered wire type.
    context.reset();

    rclcpp::executors::SingleThreadedExecutor executor;
    executor.add_node(node);
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(8);
    while (std::chrono::steady_clock::now() < deadline && !mismatch) {
        dds_publisher.publish(sample);
        dds_publisher2.publish(sample2);
        ros_publisher->publish(sample);
        executor.spin_some();
        if (ros_received && ros_received2 && dds_received && local_dds_received && !mismatch) {
            const auto pubs1 = node->get_publishers_info_by_topic("/" + dds_output);
            const auto pubs2 = node->get_publishers_info_by_topic("/" + dds_output2);
            if (pubs1.size() == 1 && pubs2.size() == 1) {
                // Fast DDS maps an RTPS GUID to the ROS endpoint GID. Its first
                // 12 bytes identify the participant, the next 4 the endpoint.
                constexpr std::size_t prefix_size = 12;
                const auto& gid1 = pubs1[0].endpoint_gid();
                const auto& gid2 = pubs2[0].endpoint_gid();
                if (gid1 == gid2 || !std::equal(gid1.begin(), gid1.begin() + prefix_size, gid2.begin())) {
                    std::cerr << "The topics did not use distinct writers on one DDS participant\n";
                    return 1;
                }
                std::cout << "Two IMU topics share a participant; payloads match without cross-talk "
                          << "in both ROS 2/Fast DDS directions\n";
                return 0;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    std::cerr << "IMU interop failed: ROS received=" << ros_received.load()
              << "/" << ros_received2.load()
              << " DDS received=" << dds_received.load()
              << " local DDS received=" << local_dds_received.load()
              << " field mismatch=" << mismatch.load() << '\n';
    return 1;
}

}  // namespace

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    const int result = test_exchange();
    rclcpp::shutdown();
    return result;
}

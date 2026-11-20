/*
 * Copyright (C) 2026 SpacemiT (Hangzhou) Technology Co. Ltd.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <unistd.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include <rclcpp/rclcpp.hpp>

#include <fastdds_ros/geometry_msgs/pose_stamped.hpp>
#include <fastdds_ros/geometry_msgs/twist.hpp>
#include <fastdds_ros/geometry_msgs/twist_stamped.hpp>
#include <fastdds_ros/mavros_msgs/rc_in.hpp>
#include <fastdds_ros/sensor_msgs/nav_sat_fix.hpp>

namespace
{

template<class Message>
bool exchange(const rclcpp::Node::SharedPtr& node,
    const std::shared_ptr<fastdds_ros::Context>& context, const std::string& type,
    const Message& sample)
{
    const auto prefix = "/fastdds_mavros_test_" + std::to_string(getpid()) + "/" + type;
    const auto qos = rclcpp::SensorDataQoS().keep_last(10);
    std::atomic<bool> ros_received{false};
    std::atomic<bool> dds_received{false};
    std::atomic<bool> mismatch{false};
    auto compare = [&](const Message& received, std::atomic<bool>& success) {
        if (received == sample) {
            success = true;
        } else {
            mismatch = true;
        }
    };
    auto ros_reader = node->create_subscription<Message>(prefix + "/dds_out", qos,
        [&](const Message& message) { compare(message, ros_received); });
    auto ros_writer = node->create_publisher<Message>(prefix + "/ros_out", qos);
    fastdds_ros::RosPublisher<Message> dds_writer;
    fastdds_ros::RosSubscriber<Message> dds_reader;
    if (!dds_writer.init("rt" + prefix + "/dds_out", context) ||
        !dds_reader.init("rt" + prefix + "/ros_out", context,
            [&](const Message& message) { compare(message, dds_received); })) {
        std::cerr << type << ": endpoint initialization failed\n";
        return false;
    }

    rclcpp::executors::SingleThreadedExecutor executor;
    executor.add_node(node);
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (std::chrono::steady_clock::now() < deadline && !mismatch) {
        dds_writer.publish(sample);
        ros_writer->publish(sample);
        executor.spin_some();
        if (ros_received && dds_received && !mismatch) {
            std::cout << type << ": DDS -> ROS 2 and ROS 2 -> DDS: all fields match\n";
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }
    std::cerr << type << ": ROS received=" << ros_received.load()
              << " DDS received=" << dds_received.load()
              << " field mismatch=" << mismatch.load() << '\n';
    return false;
}

bool run()
{
    const auto node = std::make_shared<rclcpp::Node>("mavros_dds_interop_test");
    std::size_t domain_id = 0;
    if (rcl_context_get_domain_id(
            node->get_node_base_interface()->get_context()->get_rcl_context().get(),
            &domain_id) != RCL_RET_OK) {
        return false;
    }
    const auto context = fastdds_ros::Context::create(static_cast<int>(domain_id));
    if (!context) {
        return false;
    }

    std_msgs::msg::Header header;
    header.stamp.sec = 127;
    header.stamp.nanosec = 279000001;
    // Exercise reallocating DDS history for unbounded ROS strings and arrays.
    header.frame_id = "frame_" + std::string(384, 'a');
    geometry_msgs::msg::PoseStamped pose;
    pose.header = header;
    pose.pose.position.x = 1.5;
    pose.pose.position.y = -2.25;
    pose.pose.position.z = 3.5;
    pose.pose.orientation.x = -0.5;
    pose.pose.orientation.y = 0.5;
    pose.pose.orientation.z = -0.5;
    pose.pose.orientation.w = 0.5;
    geometry_msgs::msg::TwistStamped twist_stamped;
    twist_stamped.header = header;
    auto& twist = twist_stamped.twist;
    twist.linear.x = -1.125;
    twist.linear.y = 2.25;
    twist.linear.z = -3.5;
    twist.angular.x = 0.25;
    twist.angular.y = -0.5;
    twist.angular.z = 0.125;
    sensor_msgs::msg::NavSatFix gps;
    gps.header = header;
    gps.status.status = sensor_msgs::msg::NavSatStatus::STATUS_FIX;
    gps.status.service = sensor_msgs::msg::NavSatStatus::SERVICE_GPS;
    gps.latitude = -30.25;
    gps.longitude = 119.875;
    gps.altitude = 19.5;
    gps.position_covariance = {1.0, 0.1, -0.2, 0.1, 2.0, 0.3, -0.2, 0.3, 3.0};
    gps.position_covariance_type = sensor_msgs::msg::NavSatFix::COVARIANCE_TYPE_KNOWN;
    mavros_msgs::msg::RCIn rc;
    rc.header = header;
    rc.rssi = 93;
    for (unsigned i = 0; i < 24; ++i) {
        rc.channels.push_back(1000 + i * 39);
    }

    bool pass = exchange(node, context, "pose_stamped", pose);
    pass &= exchange(node, context, "twist_stamped", twist_stamped);
    pass &= exchange(node, context, "twist", twist);
    pass &= exchange(node, context, "nav_sat_fix", gps);
    pass &= exchange(node, context, "rc_in", rc);
    return pass;
}

}  // namespace

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    const int result = run() ? 0 : 1;
    rclcpp::shutdown();
    return result;
}

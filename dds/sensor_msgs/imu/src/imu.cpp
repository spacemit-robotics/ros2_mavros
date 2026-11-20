/*
 * Copyright (C) 2026 SpacemiT (Hangzhou) Technology Co. Ltd.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "fastdds_ros/imu.hpp"

#include <utility>

#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>

#include "ImuPubSubTypes.h"
#include "fastdds_ros/context.hpp"
#include "fastdds_ros/detail/dds_entities.hpp"

namespace fastdds_ros
{
namespace
{

using RosImu = sensor_msgs::msg::Imu;
using DdsImu = my_sensor_msgs::msg::Imu;
namespace dds = eprosima::fastdds::dds;

dds::TypeSupport imu_wire_type()
{
    return dds::TypeSupport(new my_sensor_msgs::msg::ImuPubSubType());
}

DdsImu to_dds(const RosImu& message)
{
    DdsImu sample;
    sample.header().stamp().sec(message.header.stamp.sec);
    sample.header().stamp().nanosec(message.header.stamp.nanosec);
    sample.header().frame_id(message.header.frame_id);
    sample.orientation().x(message.orientation.x);
    sample.orientation().y(message.orientation.y);
    sample.orientation().z(message.orientation.z);
    sample.orientation().w(message.orientation.w);
    sample.angular_velocity().x(message.angular_velocity.x);
    sample.angular_velocity().y(message.angular_velocity.y);
    sample.angular_velocity().z(message.angular_velocity.z);
    sample.linear_acceleration().x(message.linear_acceleration.x);
    sample.linear_acceleration().y(message.linear_acceleration.y);
    sample.linear_acceleration().z(message.linear_acceleration.z);
    sample.orientation_covariance() = message.orientation_covariance;
    sample.angular_velocity_covariance() = message.angular_velocity_covariance;
    sample.linear_acceleration_covariance() = message.linear_acceleration_covariance;
    return sample;
}

RosImu to_ros(const DdsImu& sample)
{
    RosImu message;
    message.header.stamp.sec = sample.header().stamp().sec();
    message.header.stamp.nanosec = sample.header().stamp().nanosec();
    message.header.frame_id = sample.header().frame_id();
    message.orientation.x = sample.orientation().x();
    message.orientation.y = sample.orientation().y();
    message.orientation.z = sample.orientation().z();
    message.orientation.w = sample.orientation().w();
    message.angular_velocity.x = sample.angular_velocity().x();
    message.angular_velocity.y = sample.angular_velocity().y();
    message.angular_velocity.z = sample.angular_velocity().z();
    message.linear_acceleration.x = sample.linear_acceleration().x();
    message.linear_acceleration.y = sample.linear_acceleration().y();
    message.linear_acceleration.z = sample.linear_acceleration().z();
    message.orientation_covariance = sample.orientation_covariance();
    message.angular_velocity_covariance = sample.angular_velocity_covariance();
    message.linear_acceleration_covariance = sample.linear_acceleration_covariance();
    return message;
}

class ImuListener final : public dds::DataReaderListener
{
public:
    explicit ImuListener(ImuSubscriber::Callback callback) : callback_(std::move(callback))
    {
    }

    void on_data_available(dds::DataReader* reader) override
    {
        DdsImu sample;
        dds::SampleInfo info;
        while (reader->take_next_sample(&sample, &info) ==
               eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK) {
            if (info.valid_data) {
                callback_(to_ros(sample));
            }
        }
    }

private:
    ImuSubscriber::Callback callback_;
};

}  // namespace

struct ImuPublisher::Impl
{
    std::unique_ptr<Context::Writer> writer;
};

ImuPublisher::ImuPublisher() = default;
ImuPublisher::~ImuPublisher() = default;

bool ImuPublisher::init(const std::string& dds_topic, int domain_id)
{
    if (impl_ || dds_topic.empty() || domain_id < 0) {
        return false;
    }
    return init(dds_topic, Context::create(domain_id));
}

bool ImuPublisher::init(const std::string& dds_topic, const std::shared_ptr<Context>& context)
{
    if (impl_ || dds_topic.empty() || !context) {
        return false;
    }
    auto candidate = std::make_unique<Impl>();
    candidate->writer = context->create_writer(
        dds_topic, imu_wire_type(), detail::sensor_data_qos<dds::DataWriterQos>());
    if (!candidate->writer) {
        return false;
    }
    impl_ = std::move(candidate);
    return true;
}

bool ImuPublisher::publish(const RosImu& message)
{
    if (!impl_) {
        return false;
    }
    auto sample = to_dds(message);
    return impl_->writer->write(&sample);
}

struct ImuSubscriber::Impl
{
    std::unique_ptr<Context::Reader> reader;
};

ImuSubscriber::ImuSubscriber() = default;
ImuSubscriber::~ImuSubscriber() = default;

bool ImuSubscriber::init(const std::string& dds_topic, int domain_id, Callback callback)
{
    if (impl_ || dds_topic.empty() || domain_id < 0 || !callback) {
        return false;
    }
    return init(dds_topic, Context::create(domain_id), std::move(callback));
}

bool ImuSubscriber::init(const std::string& dds_topic, const std::shared_ptr<Context>& context,
                         Callback callback)
{
    if (impl_ || dds_topic.empty() || !context || !callback) {
        return false;
    }
    auto candidate = std::make_unique<Impl>();
    candidate->reader = context->create_reader(
        dds_topic, imu_wire_type(), detail::sensor_data_qos<dds::DataReaderQos>(),
        std::make_unique<ImuListener>(std::move(callback)));
    if (!candidate->reader) {
        return false;
    }
    impl_ = std::move(candidate);
    return true;
}

}  // namespace fastdds_ros

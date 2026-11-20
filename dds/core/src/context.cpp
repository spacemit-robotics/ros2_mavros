/*
 * Copyright (C) 2026 SpacemiT (Hangzhou) Technology Co. Ltd.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "fastdds_ros/context.hpp"

#include <utility>

#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>

#include "fastdds_ros/detail/dds_entities.hpp"

namespace fastdds_ros
{

namespace dds = eprosima::fastdds::dds;

std::shared_ptr<Context> Context::create(int domain_id)
{
    auto context = std::shared_ptr<Context>(new Context());
    context->participant_ = detail::create_participant(domain_id);
    if (!context->participant_) {
        return nullptr;
    }
    context->publisher_ = context->participant_->create_publisher(dds::PUBLISHER_QOS_DEFAULT);
    context->subscriber_ = context->participant_->create_subscriber(dds::SUBSCRIBER_QOS_DEFAULT);
    return context->publisher_ && context->subscriber_ ? context : nullptr;
}

Context::~Context()
{
    detail::destroy_participant(participant_);
}

Context::Writer::Writer(std::shared_ptr<Context> context) : context_(std::move(context))
{
}

Context::Writer::~Writer()
{
    if (writer_) {
        context_->publisher_->delete_datawriter(writer_);
    }
}

bool Context::Writer::write(void* sample)
{
    return sample && writer_->write(sample);
}

Context::Reader::Reader(std::shared_ptr<Context> context,
                        std::unique_ptr<dds::DataReaderListener> listener)
    : context_(std::move(context)), listener_(std::move(listener))
{
}

Context::Reader::~Reader()
{
    if (reader_) {
        context_->subscriber_->delete_datareader(reader_);
    }
}

std::unique_ptr<Context::Writer> Context::create_writer(const std::string& topic,
    const dds::TypeSupport& type, const dds::DataWriterQos& qos)
{
    std::lock_guard<std::mutex> lock(setup_mutex_);
    auto* resolved = detail::ensure_topic(*participant_, topic, type);
    if (!resolved) {
        return nullptr;
    }
    auto endpoint = std::unique_ptr<Writer>(new Writer(shared_from_this()));
    endpoint->writer_ = publisher_->create_datawriter(resolved, qos);
    return endpoint->writer_ ? std::move(endpoint) : nullptr;
}

std::unique_ptr<Context::Reader> Context::create_reader(const std::string& topic,
    const dds::TypeSupport& type, const dds::DataReaderQos& qos,
    std::unique_ptr<dds::DataReaderListener> listener)
{
    if (!listener) {
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(setup_mutex_);
    auto* resolved = detail::ensure_topic(*participant_, topic, type);
    if (!resolved) {
        return nullptr;
    }
    auto endpoint = std::unique_ptr<Reader>(new Reader(shared_from_this(), std::move(listener)));
    endpoint->reader_ = subscriber_->create_datareader(resolved, qos, endpoint->listener_.get());
    return endpoint->reader_ ? std::move(endpoint) : nullptr;
}

}  // namespace fastdds_ros

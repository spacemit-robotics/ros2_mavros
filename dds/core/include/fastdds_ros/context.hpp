/*
 * Copyright (C) 2026 SpacemiT (Hangzhou) Technology Co. Ltd.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>
#include <mutex>
#include <string>

#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

namespace eprosima { namespace fastdds { namespace dds {
class DataWriter;
class DomainParticipant;
class Publisher;
class Subscriber;
}}}

namespace fastdds_ros
{

// Share this between endpoints using the same DDS domain in one process.
// Topics and registered types live until the context is destroyed.
class Context final : public std::enable_shared_from_this<Context>
{
public:
    class Writer final
    {
    public:
        ~Writer();
        Writer(const Writer&) = delete;
        Writer& operator=(const Writer&) = delete;

        // sample must be an instance of the wire type supplied at creation.
        bool write(void* sample);

    private:
        friend class Context;
        explicit Writer(std::shared_ptr<Context> context);
        std::shared_ptr<Context> context_;
        eprosima::fastdds::dds::DataWriter* writer_ = nullptr;
    };

    class Reader final
    {
    public:
        ~Reader();
        Reader(const Reader&) = delete;
        Reader& operator=(const Reader&) = delete;

    private:
        friend class Context;
        Reader(std::shared_ptr<Context> context,
               std::unique_ptr<eprosima::fastdds::dds::DataReaderListener> listener);
        std::shared_ptr<Context> context_;
        std::unique_ptr<eprosima::fastdds::dds::DataReaderListener> listener_;
        eprosima::fastdds::dds::DataReader* reader_ = nullptr;
    };

    static std::shared_ptr<Context> create(int domain_id);
    ~Context();

    // Use exact DDS names. Different wire types cannot share a topic name.
    std::unique_ptr<Writer> create_writer(const std::string& topic,
        const eprosima::fastdds::dds::TypeSupport& type,
        const eprosima::fastdds::dds::DataWriterQos& qos);
    std::unique_ptr<Reader> create_reader(const std::string& topic,
        const eprosima::fastdds::dds::TypeSupport& type,
        const eprosima::fastdds::dds::DataReaderQos& qos,
        std::unique_ptr<eprosima::fastdds::dds::DataReaderListener> listener);

private:
    Context() = default;
    std::mutex setup_mutex_;
    eprosima::fastdds::dds::DomainParticipant* participant_ = nullptr;
    eprosima::fastdds::dds::Publisher* publisher_ = nullptr;
    eprosima::fastdds::dds::Subscriber* subscriber_ = nullptr;
};

}  // namespace fastdds_ros

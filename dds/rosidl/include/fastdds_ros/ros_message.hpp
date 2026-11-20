/*
 * Copyright (C) 2026 SpacemiT (Hangzhou) Technology Co. Ltd.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include <fastcdr/Cdr.h>
#include <fastcdr/FastBuffer.h>
#include <fastcdr/exceptions/Exception.h>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <rosidl_typesupport_fastrtps_cpp/message_type_support.h>

#include "fastdds_ros/context.hpp"
#include "fastdds_ros/detail/dds_entities.hpp"

namespace fastdds_ros
{

// Specialize with the generated FastRTPS type support for one ROS message.
template<class Message>
struct RosMessageType;

namespace detail
{

template<class Message>
class RosTopicType final : public dds::TopicDataType
{
    using Payload = eprosima::fastrtps::rtps::SerializedPayload_t;
    using Cdr = eprosima::fastcdr::Cdr;

public:
    RosTopicType()
        : callbacks_(static_cast<const message_type_support_callbacks_t*>(
              RosMessageType<Message>::get()->data))
    {
        const auto name = std::string(callbacks_->message_namespace_) +
            "::dds_::" + callbacks_->message_name_ + "_";
        setName(name.c_str());
        char bounds = 0;
        m_typeSize = static_cast<std::uint32_t>(callbacks_->max_serialized_size(bounds) + 4);
        m_isGetKeyDefined = false;
    }

    bool serialize(void* data, Payload* payload) override
    {
        try {
            eprosima::fastcdr::FastBuffer buffer(reinterpret_cast<char*>(payload->data), payload->max_size);
            Cdr cdr(buffer, Cdr::DEFAULT_ENDIAN, Cdr::DDS_CDR);
            cdr.serialize_encapsulation();
            if (!callbacks_->cdr_serialize(data, cdr)) {
                return false;
            }
            payload->encapsulation = cdr.endianness() == Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
            payload->length = static_cast<std::uint32_t>(cdr.getSerializedDataLength());
            return true;
        } catch (const eprosima::fastcdr::exception::Exception&) {
            return false;
        }
    }

    bool deserialize(Payload* payload, void* data) override
    {
        try {
            eprosima::fastcdr::FastBuffer buffer(reinterpret_cast<char*>(payload->data), payload->length);
            Cdr cdr(buffer, Cdr::DEFAULT_ENDIAN, Cdr::DDS_CDR);
            cdr.read_encapsulation();
            payload->encapsulation = cdr.endianness() == Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
            return callbacks_->cdr_deserialize(cdr, data);
        } catch (const eprosima::fastcdr::exception::Exception&) {
            return false;
        }
    }

    std::function<std::uint32_t()> getSerializedSizeProvider(void* data) override
    {
        return [this, data] { return 4u + callbacks_->get_serialized_size(data); };
    }

    void* createData() override { return new Message(); }
    void deleteData(void* data) override { delete static_cast<Message*>(data); }

    bool getKey(void*, eprosima::fastrtps::rtps::InstanceHandle_t*, bool) override
    {
        return false;
    }

private:
    const message_type_support_callbacks_t* callbacks_;
};

template<class Qos>
Qos ros_message_qos()
{
    auto qos = sensor_data_qos<Qos>();
    qos.endpoint().history_memory_policy =
        eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    return qos;
}

}  // namespace detail

template<class Message>
class RosPublisher final
{
public:
    bool init(const std::string& topic, const std::shared_ptr<Context>& context)
    {
        if (writer_ || !context || topic.empty()) {
            return false;
        }
        writer_ = context->create_writer(topic, detail::dds::TypeSupport(new detail::RosTopicType<Message>()),
            detail::ros_message_qos<detail::dds::DataWriterQos>());
        return static_cast<bool>(writer_);
    }

    bool publish(const Message& message)
    {
        // Fast DDS 2.x takes void*, but serialization only reads the message.
        return writer_ && writer_->write(const_cast<Message*>(&message));
    }

private:
    std::unique_ptr<Context::Writer> writer_;
};

template<class Message>
class RosSubscriber final
{
public:
    using Callback = std::function<void(const Message&)>;

    // The callback runs on the DDS receive thread. It must not throw, destroy
    // this subscriber, or retain a reference to the received message.
    bool init(const std::string& topic, const std::shared_ptr<Context>& context, Callback callback)
    {
        if (reader_ || !context || topic.empty() || !callback) {
            return false;
        }
        reader_ = context->create_reader(topic, detail::dds::TypeSupport(new detail::RosTopicType<Message>()),
            detail::ros_message_qos<detail::dds::DataReaderQos>(),
            std::make_unique<Listener>(std::move(callback)));
        return static_cast<bool>(reader_);
    }

private:
    class Listener final : public detail::dds::DataReaderListener
    {
    public:
        explicit Listener(Callback callback) : callback_(std::move(callback)) {}

        void on_data_available(detail::dds::DataReader* reader) override
        {
            Message sample;
            detail::dds::SampleInfo info;
            while (reader->take_next_sample(&sample, &info) ==
                   eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK) {
                if (info.valid_data) {
                    callback_(sample);
                }
            }
        }

    private:
        Callback callback_;
    };

    std::unique_ptr<Context::Reader> reader_;
};

}  // namespace fastdds_ros

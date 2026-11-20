/*
 * Copyright (C) 2026 SpacemiT (Hangzhou) Technology Co. Ltd.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>

namespace fastdds_ros
{
namespace detail
{

namespace dds = eprosima::fastdds::dds;
namespace rtps = eprosima::fastdds::rtps;

inline dds::DomainParticipant* create_participant(int domain_id)
{
    if (domain_id < 0) {
        return nullptr;
    }
    dds::DomainParticipantQos qos;
    qos.name("/");
    auto& discovery = qos.wire_protocol().builtin.discovery_config;
    discovery.discoveryProtocol = eprosima::fastrtps::rtps::DiscoveryProtocol_t::SIMPLE;
    discovery.use_SIMPLE_EndpointDiscoveryProtocol = true;
    discovery.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    discovery.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    discovery.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
    qos.transport().use_builtin_transports = false;
    qos.transport().user_transports.push_back(std::make_shared<rtps::SharedMemTransportDescriptor>());
    qos.transport().user_transports.push_back(std::make_shared<rtps::UDPv4TransportDescriptor>());
    return dds::DomainParticipantFactory::get_instance()->create_participant(domain_id, qos);
}

inline void destroy_participant(dds::DomainParticipant* participant)
{
    if (participant != nullptr) {
        participant->delete_contained_entities();
        dds::DomainParticipantFactory::get_instance()->delete_participant(participant);
    }
}

// The caller serializes endpoint setup on a shared participant.
inline dds::Topic* ensure_topic(dds::DomainParticipant& participant,
                               const std::string& name, const dds::TypeSupport& type)
{
    if (name.empty() || !type) {
        return nullptr;
    }
    const auto& type_name = type.get_type_name();
    if (auto* description = participant.lookup_topicdescription(name)) {
        return description->get_type_name() == type_name
            ? dynamic_cast<dds::Topic*>(description) : nullptr;
    }
    if (!participant.find_type(type_name) &&
        type.register_type(&participant) != eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK) {
        return nullptr;
    }
    return participant.create_topic(name, type_name, dds::TOPIC_QOS_DEFAULT);
}

template<class Qos>
Qos sensor_data_qos()
{
    Qos qos;
    qos.reliability().kind = dds::BEST_EFFORT_RELIABILITY_QOS;
    qos.durability().kind = dds::VOLATILE_DURABILITY_QOS;
    qos.history().kind = dds::KEEP_LAST_HISTORY_QOS;
    qos.history().depth = 10;
    qos.resource_limits().max_samples = 10;
    return qos;
}

}  // namespace detail
}  // namespace fastdds_ros

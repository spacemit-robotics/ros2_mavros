/*
 * Copyright (C) 2026 SpacemiT (Hangzhou) Technology Co. Ltd.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <unordered_map>

#include <fastdds_ros/context.hpp>

#include "mavros/dds_domain.hpp"

namespace mavros::dds
{

// Plugin endpoints keep their context alive, but the cache does not. This lets
// a stopped UAS dispose of its DDS entities before the next startup.
inline std::shared_ptr<fastdds_ros::Context> shared_context(const rclcpp::Node::SharedPtr & node)
{
  const auto raw_id = domain_id_from_node(node);
  if (raw_id > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
    throw std::runtime_error("DDS domain ID is out of range for MAVROS");
  }
  const auto id = static_cast<int>(raw_id);
  static std::mutex mutex;
  static std::unordered_map<int, std::weak_ptr<fastdds_ros::Context>> contexts;
  std::lock_guard<std::mutex> lock(mutex);
  auto & entry = contexts[id];
  auto context = entry.lock();
  if (!context) {
    context = fastdds_ros::Context::create(id);
    if (!context) {
      throw std::runtime_error("Cannot create the MAVROS DDS context");
    }
    entry = context;
  }
  return context;
}

}  // namespace mavros::dds

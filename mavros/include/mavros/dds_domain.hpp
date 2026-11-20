/*
 * Copyright 2026 SpacemiT.
 *
 * This file is part of the mavros package and subject to the license terms
 * in the top-level LICENSE file of the mavros repository.
 */

#pragma once

#include <cstddef>
#include <stdexcept>

#include "rcl/context.h"
#include "rclcpp/node.hpp"

namespace mavros
{
namespace dds
{

inline std::size_t domain_id_from_node(const rclcpp::Node::SharedPtr & node)
{
  const auto context = node->get_node_base_interface()->get_context();
  const auto rcl_context = context->get_rcl_context();

  std::size_t domain_id = 0;
  const auto ret = rcl_context_get_domain_id(rcl_context.get(), &domain_id);
  if (ret != RCL_RET_OK) {
    throw std::runtime_error("failed to get DDS domain ID from the ROS 2 context");
  }

  return domain_id;
}

}  // namespace dds
}  // namespace mavros
